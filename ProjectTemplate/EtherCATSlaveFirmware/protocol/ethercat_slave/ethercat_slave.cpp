#include "protocol/ethercat_slave/ethercat_slave.h"

#include <cstring>

#include "lwip/prot/ethernet.h"
#include "lwip/prot/ieee.h"
#include "netif/ethernet.h"

namespace ethercat_slave {
namespace {

static const uint16_t kEthercatHeaderSize = 2U;
static const uint16_t kEthercatDatagramHeaderSize = 10U;
static const uint16_t kEthercatWkcSize = 2U;
static const uint16_t kEthercatMaxPayload = 1498U;
static const uint16_t kEscRegisterSpaceSize = 4096U;
static const uint16_t kProcessImageSize = 512U;
static const uint16_t kPdoCommandOffset = 0x000U;
static const uint16_t kPdoStatusOffset = 0x100U;
static const uint32_t kLogicalProcessBase = 0x00000000UL;
static const uint32_t kLogicalProcessAltBase = 0x00001000UL;

static const uint16_t kEscRegConfiguredStationAddress = 0x0010U;
static const uint16_t kEscRegConfiguredStationAlias = 0x0012U;
static const uint16_t kEscRegAlControl = 0x0120U;
static const uint16_t kEscRegAlStatus = 0x0130U;
static const uint16_t kEscRegAlStatusCode = 0x0134U;
static const uint16_t kEscRegDlStatus = 0x0110U;
static const uint16_t kEscRegEepromConfig = 0x0500U;
static const uint16_t kEscRegEepromControlStatus = 0x0502U;
static const uint16_t kEscRegEepromAddress = 0x0504U;
static const uint16_t kEscRegEepromData = 0x0508U;
static const uint16_t kEscRegFmmu0 = 0x0600U;
static const uint16_t kEscRegSm0 = 0x0800U;
static const uint16_t kEscFmmuSize = 16U;
static const uint16_t kEscSmSize = 8U;
static const uint8_t kEscMaxFmmu = 4U;

static const uint16_t kSiiStartWord = 0x0040U;
static const uint16_t kSiiCategoryStrings = 10U;
static const uint16_t kSiiCategorySm = 41U;
static const uint16_t kSiiCategoryTxPdo = 50U;
static const uint16_t kSiiCategoryRxPdo = 51U;
static const uint16_t kSiiCategoryEnd = 0xFFFFU;
static const uint16_t kSiiWordCount = 512U;

static const uint16_t kEepromCommandMask = 0x0700U;
static const uint16_t kEepromCommandNop = 0x0000U;
static const uint16_t kEepromCommandRead = 0x0100U;
static const uint16_t kEepromCommandWrite = 0x0200U;
static const uint16_t kEepromCommandReload = 0x0300U;
static const uint16_t kEepromStatusReady32 = 0x0020U;
static const uint16_t kEepromStatusReady64 = 0x0060U;

static const uint8_t kAlStateInit = 0x01U;
static const uint8_t kAlStatePreOp = 0x02U;
static const uint8_t kAlStateSafeOp = 0x04U;
static const uint8_t kAlStateOp = 0x08U;
static const uint8_t kAlControlErrorAck = 0x10U;
static const uint16_t kAlStatusCodeNoError = 0x0000U;
static const uint16_t kAlStatusCodeUnsupportedRequest = 0x0011U;

enum EthercatCommand : uint8_t {
    kEcCmdNop = 0x00U,
    kEcCmdAprd = 0x01U,
    kEcCmdApwr = 0x02U,
    kEcCmdAprw = 0x03U,
    kEcCmdFprd = 0x04U,
    kEcCmdFpwr = 0x05U,
    kEcCmdFprw = 0x06U,
    kEcCmdBrd = 0x07U,
    kEcCmdBwr = 0x08U,
    kEcCmdBrw = 0x09U,
    kEcCmdLrd = 0x0AU,
    kEcCmdLwr = 0x0BU,
    kEcCmdLrw = 0x0CU
};

struct EthercatSlaveContext {
    struct netif *netif;
    EthercatPdoCommand command_image;
    EthercatPdoStatus status_image;
    EthercatSlaveStats stats;
    uint16_t configured_station_address;
    uint8_t esc_registers[kEscRegisterSpaceSize];
    uint16_t sii_eeprom_words[kSiiWordCount];
    uint8_t process_image[kProcessImageSize];
    uint32_t output_logical_base;
    uint32_t input_logical_base;
    bool output_logical_base_valid;
    bool input_logical_base_valid;
    struct eth_addr last_command_source;
    bool last_command_source_valid;
};

EthercatSlaveContext g_context = {};

struct EscFmmuConfig {
    uint32_t logical_start;
    uint16_t logical_length;
    uint16_t physical_start;
    uint8_t type;
    bool active;
};

uint16_t ReadLe16(const uint8_t *data) {
    return static_cast<uint16_t>(data[0]) |
           static_cast<uint16_t>(static_cast<uint16_t>(data[1]) << 8U);
}

void WriteLe16(uint8_t *data, uint16_t value) {
    data[0] = static_cast<uint8_t>(value & 0xFFU);
    data[1] = static_cast<uint8_t>((value >> 8U) & 0xFFU);
}

void WriteLe32(uint8_t *data, uint32_t value) {
    data[0] = static_cast<uint8_t>(value & 0xFFU);
    data[1] = static_cast<uint8_t>((value >> 8U) & 0xFFU);
    data[2] = static_cast<uint8_t>((value >> 16U) & 0xFFU);
    data[3] = static_cast<uint8_t>((value >> 24U) & 0xFFU);
}

uint32_t ReadLe32(const uint8_t *data) {
    return static_cast<uint32_t>(data[0]) |
           (static_cast<uint32_t>(data[1]) << 8U) |
           (static_cast<uint32_t>(data[2]) << 16U) |
           (static_cast<uint32_t>(data[3]) << 24U);
}

void SetSiiWord(uint16_t word_address, uint16_t value) {
    if (word_address < kSiiWordCount) {
        g_context.sii_eeprom_words[word_address] = value;
    }
}

void SetSiiByte(uint16_t byte_address, uint8_t value) {
    const uint16_t word_address = static_cast<uint16_t>(byte_address >> 1U);
    if (word_address >= kSiiWordCount) {
        return;
    }
    uint16_t word_value = g_context.sii_eeprom_words[word_address];
    if ((byte_address & 1U) == 0U) {
        word_value = static_cast<uint16_t>((word_value & 0xFF00U) | value);
    } else {
        word_value = static_cast<uint16_t>((word_value & 0x00FFU) |
                                           (static_cast<uint16_t>(value) << 8U));
    }
    g_context.sii_eeprom_words[word_address] = word_value;
}

void AppendSiiCategoryBytes(uint16_t category, const uint8_t *bytes, uint16_t byte_count,
                            uint16_t *section_word) {
    if (bytes == nullptr || section_word == nullptr) {
        return;
    }
    const uint16_t word_count = static_cast<uint16_t>((byte_count + 1U) / 2U);
    SetSiiWord((*section_word)++, category);
    SetSiiWord((*section_word)++, word_count);
    const uint16_t byte_base = static_cast<uint16_t>((*section_word) << 1U);
    for (uint16_t i = 0U; i < static_cast<uint16_t>(word_count * 2U); ++i) {
        const uint8_t value = (i < byte_count) ? bytes[i] : 0U;
        SetSiiByte(static_cast<uint16_t>(byte_base + i), value);
    }
    *section_word = static_cast<uint16_t>(*section_word + word_count);
}

uint8_t AppendSiiString(uint8_t *dest, uint8_t offset, const char *text) {
    if (dest == nullptr || text == nullptr) {
        return offset;
    }
    uint8_t length = 0U;
    while (text[length] != '\0' && length < 80U) {
        ++length;
    }
    dest[offset++] = length;
    for (uint8_t i = 0U; i < length; ++i) {
        dest[offset++] = static_cast<uint8_t>(text[i]);
    }
    return offset;
}

void WriteSiiPdoHeader(uint8_t *bytes, uint16_t pdo_index, uint8_t entry_count,
                       uint8_t sync_manager) {
    bytes[0] = static_cast<uint8_t>(pdo_index & 0xFFU);
    bytes[1] = static_cast<uint8_t>((pdo_index >> 8U) & 0xFFU);
    bytes[2] = entry_count;
    bytes[3] = sync_manager;
    bytes[4] = 0U;  // DC sync.
    bytes[5] = 0U;  // Name index.
    bytes[6] = 0U;  // Flags.
    bytes[7] = 0U;  // Reserved.
}

void WriteSiiPdoEntry(uint8_t *bytes, uint8_t entry_number, uint16_t object_index,
                      uint8_t sub_index, uint8_t bit_length) {
    const uint16_t offset = static_cast<uint16_t>(8U + (entry_number * 8U));
    bytes[offset + 0U] = static_cast<uint8_t>(object_index & 0xFFU);
    bytes[offset + 1U] = static_cast<uint8_t>((object_index >> 8U) & 0xFFU);
    bytes[offset + 2U] = sub_index;
    bytes[offset + 3U] = 0U;  // Name index.
    bytes[offset + 4U] = 0U;  // Data type index.
    bytes[offset + 5U] = bit_length;
    bytes[offset + 6U] = 0U;  // Flags low.
    bytes[offset + 7U] = 0U;  // Flags high.
}

void AppendSiiPdoCategory(uint16_t category, uint16_t pdo_index,
                          uint8_t sync_manager, uint16_t object_index,
                          const uint8_t *bit_lengths, uint8_t entry_count,
                          uint16_t *section_word) {
    if (bit_lengths == nullptr || section_word == nullptr) {
        return;
    }
    uint8_t pdo_bytes[160] = {};
    WriteSiiPdoHeader(pdo_bytes, pdo_index, entry_count, sync_manager);
    for (uint8_t i = 0U; i < entry_count; ++i) {
        WriteSiiPdoEntry(pdo_bytes, i, object_index, static_cast<uint8_t>(i + 1U),
                         bit_lengths[i]);
    }
    const uint16_t byte_count = static_cast<uint16_t>(8U + entry_count * 8U);
    AppendSiiCategoryBytes(category, pdo_bytes, byte_count, section_word);
}

bool IsReadCommand(uint8_t command) {
    return command == kEcCmdAprd || command == kEcCmdFprd || command == kEcCmdBrd ||
           command == kEcCmdLrd;
}

bool IsWriteCommand(uint8_t command) {
    return command == kEcCmdApwr || command == kEcCmdFpwr || command == kEcCmdBwr ||
           command == kEcCmdLwr;
}

bool IsReadWriteCommand(uint8_t command) {
    return command == kEcCmdAprw || command == kEcCmdFprw || command == kEcCmdBrw ||
           command == kEcCmdLrw;
}

bool IsLogicalCommand(uint8_t command) {
    return command == kEcCmdLrd || command == kEcCmdLwr || command == kEcCmdLrw;
}

bool IsAddressedToThisSlave(uint8_t command, uint16_t adp) {
    if (IsLogicalCommand(command)) {
        return true;
    }
    if (command == kEcCmdBrd || command == kEcCmdBwr || command == kEcCmdBrw) {
        return true;
    }
    if (command == kEcCmdAprd || command == kEcCmdApwr || command == kEcCmdAprw) {
        // Minimal single-slave handling for auto-increment addressing.
        return true;
    }
    if (command == kEcCmdFprd || command == kEcCmdFpwr || command == kEcCmdFprw) {
        return (adp == g_context.configured_station_address);
    }
    return false;
}

bool ReadSmWindow(uint8_t sm_index, uint16_t *start_address, uint16_t *length) {
    if (start_address == nullptr || length == nullptr) {
        return false;
    }
    const uint16_t base = static_cast<uint16_t>(kEscRegSm0 + sm_index * kEscSmSize);
    if (static_cast<uint16_t>(base + 3U) >= kEscRegisterSpaceSize) {
        return false;
    }
    *start_address = ReadLe16(&g_context.esc_registers[base]);
    *length = ReadLe16(&g_context.esc_registers[static_cast<uint16_t>(base + 2U)]);
    return *length > 0U;
}

bool ReadFmmuConfig(uint8_t fmmu_index, EscFmmuConfig *config) {
    if (config == nullptr) {
        return false;
    }
    const uint16_t base = static_cast<uint16_t>(kEscRegFmmu0 + fmmu_index * kEscFmmuSize);
    if (static_cast<uint16_t>(base + 12U) >= kEscRegisterSpaceSize) {
        return false;
    }
    config->logical_start = ReadLe32(&g_context.esc_registers[base]);
    config->logical_length = ReadLe16(&g_context.esc_registers[static_cast<uint16_t>(base + 4U)]);
    config->physical_start = ReadLe16(&g_context.esc_registers[static_cast<uint16_t>(base + 8U)]);
    config->type = g_context.esc_registers[static_cast<uint16_t>(base + 11U)];
    config->active = (g_context.esc_registers[static_cast<uint16_t>(base + 12U)] & 0x01U) != 0U;
    return config->active && config->logical_length > 0U;
}

void SetAlStatus(uint8_t state, uint16_t status_code) {
    WriteLe16(&g_context.esc_registers[kEscRegAlStatus],
              static_cast<uint16_t>(state & 0x0FU));
    WriteLe16(&g_context.esc_registers[kEscRegAlStatusCode], status_code);
}

void InitializeSmRegister(uint8_t sm_index, uint16_t start_address, uint16_t length,
                          uint8_t control, uint8_t activate) {
    const uint16_t base = static_cast<uint16_t>(kEscRegSm0 + sm_index * kEscSmSize);
    if (static_cast<uint16_t>(base + 7U) >= kEscRegisterSpaceSize) {
        return;
    }
    WriteLe16(&g_context.esc_registers[base], start_address);
    WriteLe16(&g_context.esc_registers[static_cast<uint16_t>(base + 2U)], length);
    g_context.esc_registers[static_cast<uint16_t>(base + 4U)] = control;
    g_context.esc_registers[static_cast<uint16_t>(base + 5U)] = 0x00U;
    g_context.esc_registers[static_cast<uint16_t>(base + 6U)] = activate;
    g_context.esc_registers[static_cast<uint16_t>(base + 7U)] = 0x00U;
}

void InitializeSiiEepromImage() {
    for (uint16_t i = 0; i < kSiiWordCount; ++i) {
        g_context.sii_eeprom_words[i] = kSiiCategoryEnd;
    }

    // Identity words read by masters during discovery.
    SetSiiWord(0x0008U, 0xFFFEU);  // vendor low word
    SetSiiWord(0x0009U, 0xFFFFU);  // vendor high word
    SetSiiWord(0x000AU, 0x0001U);  // product low word
    SetSiiWord(0x000BU, 0x0000U);  // product high word
    SetSiiWord(0x000CU, 0x0001U);  // revision low word
    SetSiiWord(0x000DU, 0x0000U);  // revision high word
    SetSiiWord(0x000EU, 0x0001U);  // serial low word
    SetSiiWord(0x000FU, 0x0000U);  // serial high word

    uint16_t section_word = kSiiStartWord;

    uint8_t string_category[96] = {};
    uint8_t string_offset = 0U;
    string_category[string_offset++] = 3U;
    string_offset = AppendSiiString(string_category, string_offset, "ClearCore Motion Stack");
    string_offset = AppendSiiString(string_category, string_offset, "ClearCore EtherCAT Slave");
    string_offset = AppendSiiString(string_category, string_offset, "ClearCore EtherCAT Slave");
    AppendSiiCategoryBytes(kSiiCategoryStrings, string_category, string_offset, &section_word);

    // SM category with 4 entries (SM0..SM3), 4 words each.
    SetSiiWord(section_word++, kSiiCategorySm);
    SetSiiWord(section_word++, 16U);
    // SM0
    SetSiiWord(section_word++, 0x0000U);
    SetSiiWord(section_word++, 0x0000U);
    SetSiiWord(section_word++, 0x0000U);
    SetSiiWord(section_word++, 0x0000U);
    // SM1
    SetSiiWord(section_word++, 0x0000U);
    SetSiiWord(section_word++, 0x0000U);
    SetSiiWord(section_word++, 0x0000U);
    SetSiiWord(section_word++, 0x0000U);
    // SM2 (outputs / command image)
    SetSiiWord(section_word++, 0x1000U);
    SetSiiWord(section_word++, sizeof(EthercatPdoCommand));
    SetSiiWord(section_word++, 0x0064U);
    SetSiiWord(section_word++, 0x0001U);
    // SM3 (inputs / status image)
    SetSiiWord(section_word++, 0x1100U);
    SetSiiWord(section_word++, sizeof(EthercatPdoStatus));
    SetSiiWord(section_word++, 0x0020U);
    SetSiiWord(section_word++, 0x0001U);

    const uint8_t txpdo_bit_lengths[] = {
        16U, 8U, 8U, 32U, 32U, 16U, 16U, 32U, 16U,
        8U, 8U, 64U, 64U, 64U, 32U, 32U, 32U, 32U};
    static_assert(sizeof(txpdo_bit_lengths) == 18U,
                  "TxPDO SII entry count must match EthercatPdoStatus fields.");
    AppendSiiPdoCategory(kSiiCategoryTxPdo, 0x1A00U, 3U, 0x6000U,
                         txpdo_bit_lengths, sizeof(txpdo_bit_lengths), &section_word);

    const uint8_t rxpdo_bit_lengths[] = {
        16U, 8U, 8U, 32U, 32U, 16U, 16U, 32U, 16U, 16U, 64U, 64U};
    static_assert(sizeof(rxpdo_bit_lengths) == 12U,
                  "RxPDO SII entry count must match EthercatPdoCommand fields.");
    AppendSiiPdoCategory(kSiiCategoryRxPdo, 0x1600U, 2U, 0x7000U,
                         rxpdo_bit_lengths, sizeof(rxpdo_bit_lengths), &section_word);

    SetSiiWord(section_word, kSiiCategoryEnd);
}

void ApplyRequestedAlState(uint8_t requested_state) {
    const uint8_t state = static_cast<uint8_t>(requested_state & 0x0FU);
    const bool acknowledge_error = (requested_state & kAlControlErrorAck) != 0U;
    if (acknowledge_error) {
        WriteLe16(&g_context.esc_registers[kEscRegAlStatusCode], kAlStatusCodeNoError);
    }

    switch (state) {
        case kAlStateInit:
        case kAlStatePreOp:
        case kAlStateSafeOp:
        case kAlStateOp:
            SetAlStatus(state, kAlStatusCodeNoError);
            break;
        default:
            SetAlStatus(kAlStateInit, kAlStatusCodeUnsupportedRequest);
            break;
    }
}

void SyncStatusImageToProcessImage() {
    std::memcpy(&g_context.process_image[kPdoStatusOffset], &g_context.status_image,
                sizeof(g_context.status_image));
}

void RefreshStatusFromCommandImage() {
    g_context.status_image.sequence_ack = g_context.command_image.sequence;
}

void ApplyControlWordMirror(uint16_t control_word) {
    g_context.command_image.control_word = control_word;
}

void SyncProcessImageToCommandImage() {
    std::memcpy(&g_context.command_image, &g_context.process_image[kPdoCommandOffset],
                sizeof(g_context.command_image));
    RefreshStatusFromCommandImage();
    SyncStatusImageToProcessImage();
}

void HandleEscSideEffects(uint16_t ado, uint16_t length) {
    if (ado <= kEscRegConfiguredStationAddress &&
        static_cast<uint16_t>(ado + length) >=
            static_cast<uint16_t>(kEscRegConfiguredStationAddress + 2U)) {
        g_context.configured_station_address =
            ReadLe16(&g_context.esc_registers[kEscRegConfiguredStationAddress]);
    }

    if (ado <= kEscRegAlControl &&
        static_cast<uint16_t>(ado + length) >= static_cast<uint16_t>(kEscRegAlControl + 2U)) {
        const uint16_t al_control = ReadLe16(&g_context.esc_registers[kEscRegAlControl]);
        ApplyRequestedAlState(static_cast<uint8_t>(al_control & 0x0FU));
    }

    if (ado <= kEscRegEepromControlStatus &&
        static_cast<uint16_t>(ado + length) >=
            static_cast<uint16_t>(kEscRegEepromControlStatus + 2U)) {
        const uint16_t command = static_cast<uint16_t>(
            ReadLe16(&g_context.esc_registers[kEscRegEepromControlStatus]) & kEepromCommandMask);
        if (command == kEepromCommandRead) {
            const uint32_t eeprom_address = ReadLe32(&g_context.esc_registers[kEscRegEepromAddress]);
            const uint16_t base_word = static_cast<uint16_t>(eeprom_address & 0xFFFFU);
            for (uint16_t i = 0; i < 4U; ++i) {
                const uint16_t word_address = static_cast<uint16_t>(base_word + i);
                const uint16_t value =
                    (word_address < kSiiWordCount) ? g_context.sii_eeprom_words[word_address] : 0xFFFFU;
                WriteLe16(&g_context.esc_registers[kEscRegEepromData + static_cast<uint16_t>(i * 2U)],
                          value);
            }
            WriteLe16(&g_context.esc_registers[kEscRegEepromControlStatus], kEepromStatusReady64);
        } else if (command == kEepromCommandReload || command == kEepromCommandNop ||
                   command == kEepromCommandWrite) {
            WriteLe16(&g_context.esc_registers[kEscRegEepromControlStatus], kEepromStatusReady64);
        } else {
            WriteLe16(&g_context.esc_registers[kEscRegEepromControlStatus], kEepromStatusReady32);
        }
    }

    // Keep EEPROM status in a ready state even when TwinCAT updates the address
    // and polls status in separate transactions.
    if (ado <= kEscRegEepromAddress &&
        static_cast<uint16_t>(ado + length) >=
            static_cast<uint16_t>(kEscRegEepromAddress + 4U)) {
        const uint16_t status = ReadLe16(&g_context.esc_registers[kEscRegEepromControlStatus]);
        if ((status & kEepromCommandMask) == kEepromCommandNop) {
            WriteLe16(&g_context.esc_registers[kEscRegEepromControlStatus], kEepromStatusReady64);
        }
    }
}

void InitializeEscRegisterMap() {
    std::memset(g_context.esc_registers, 0, sizeof(g_context.esc_registers));
    std::memset(g_context.process_image, 0, sizeof(g_context.process_image));
    InitializeSiiEepromImage();

    // ESC information registers. TwinCAT reads these early during scan.
    g_context.esc_registers[0x0000] = 0x11U;  // ESC type: emulated EtherCAT slave controller.
    g_context.esc_registers[0x0001] = 0x01U;  // ESC revision.
    WriteLe16(&g_context.esc_registers[0x0002], 0x0001U);  // Build.
    g_context.esc_registers[0x0004] = kEscMaxFmmu;  // Supported FMMUs.
    g_context.esc_registers[0x0005] = 4U;  // Supported SyncManagers (SM0..SM3).
    g_context.esc_registers[0x0006] = 16U;  // Emulated process RAM size in KiB.
    g_context.esc_registers[0x0007] = 0x11U;  // Two MII-like ports for scanner topology.
    WriteLe16(&g_context.esc_registers[0x0008], 0x0000U);  // ESC feature flags.
    WriteLe32(&g_context.esc_registers[0x000CU], 0x00000000UL);  // Supported distributed clocks.

    // Station address register.
    WriteLe16(&g_context.esc_registers[kEscRegConfiguredStationAddress],
              g_context.configured_station_address);
    WriteLe16(&g_context.esc_registers[kEscRegConfiguredStationAlias], 0x0000U);
    // DL status: PDI operational and link/signal present on the primary port.
    WriteLe16(&g_context.esc_registers[kEscRegDlStatus], 0x0111U);
    // AL status + AL status code (INIT, no error).
    WriteLe16(&g_context.esc_registers[kEscRegAlControl], kAlStateInit);
    SetAlStatus(kAlStateInit, kAlStatusCodeNoError);
    WriteLe16(&g_context.esc_registers[kEscRegEepromConfig], 0x0000U);
    WriteLe16(&g_context.esc_registers[kEscRegEepromControlStatus], kEepromStatusReady64);

    // Initialize SM windows to match the ESI/SII layout. SM0/SM1 are disabled
    // mailbox placeholders; SM2/SM3 are fixed process-data windows.
    InitializeSmRegister(0U, 0x0000U, 0U, 0x00U, 0x00U);
    InitializeSmRegister(1U, 0x0000U, 0U, 0x00U, 0x00U);
    InitializeSmRegister(2U, 0x1000U, sizeof(EthercatPdoCommand), 0x64U, 0x01U);
    InitializeSmRegister(3U, 0x1100U, sizeof(EthercatPdoStatus), 0x20U, 0x01U);
}

void ReadEscWindow(uint16_t ado, uint8_t *dest, uint16_t length) {
    for (uint16_t i = 0; i < length; ++i) {
        const uint16_t address = static_cast<uint16_t>(ado + i);
        if (address < sizeof(g_context.esc_registers)) {
            dest[i] = g_context.esc_registers[address];
        } else {
            dest[i] = 0U;
        }
    }
}

void WriteEscWindow(uint16_t ado, const uint8_t *src, uint16_t length) {
    for (uint16_t i = 0; i < length; ++i) {
        const uint16_t address = static_cast<uint16_t>(ado + i);
        if (address < sizeof(g_context.esc_registers)) {
            g_context.esc_registers[address] = src[i];
        }
    }

    HandleEscSideEffects(ado, length);
}

bool ResolveLogicalProcessWindow(uint32_t logical_address, uint16_t data_length,
                                 bool for_output_write, uint16_t *offset) {
    if (offset == nullptr || data_length > kProcessImageSize) {
        return false;
    }

    // Prefer runtime ESC FMMU/SM mapping programmed by the master.
    for (uint8_t fmmu_index = 0; fmmu_index < kEscMaxFmmu; ++fmmu_index) {
        EscFmmuConfig config = {};
        if (!ReadFmmuConfig(fmmu_index, &config)) {
            continue;
        }

        const uint8_t expected_type = for_output_write ? 2U : 1U;
        if (config.type != expected_type) {
            continue;
        }

        if (logical_address < config.logical_start) {
            continue;
        }
        const uint32_t logical_offset = logical_address - config.logical_start;
        if (logical_offset + data_length > config.logical_length) {
            continue;
        }

        const uint16_t physical_address =
            static_cast<uint16_t>(config.physical_start + logical_offset);
        const uint8_t sm_index = for_output_write ? 2U : 3U;
        uint16_t sm_start = 0U;
        uint16_t sm_length = 0U;
        if (!ReadSmWindow(sm_index, &sm_start, &sm_length)) {
            continue;
        }
        if (physical_address < sm_start) {
            continue;
        }
        const uint16_t sm_offset = static_cast<uint16_t>(physical_address - sm_start);
        if (sm_offset + data_length > sm_length) {
            continue;
        }

        const uint16_t process_base = for_output_write ? kPdoCommandOffset : kPdoStatusOffset;
        const uint16_t process_offset = static_cast<uint16_t>(process_base + sm_offset);
        if (process_offset + data_length > kProcessImageSize) {
            continue;
        }
        *offset = process_offset;
        return true;
    }

    // Fallback path learns logical base addresses from observed traffic.
    if (for_output_write && !g_context.output_logical_base_valid) {
        g_context.output_logical_base = logical_address;
        g_context.output_logical_base_valid = true;
    }
    if (!for_output_write && !g_context.input_logical_base_valid) {
        g_context.input_logical_base = logical_address;
        g_context.input_logical_base_valid = true;
    }

    const uint32_t default_base =
        for_output_write ? kLogicalProcessBase : static_cast<uint32_t>(kLogicalProcessBase + sizeof(EthercatPdoCommand));
    const uint32_t learned_base =
        for_output_write
            ? (g_context.output_logical_base_valid ? g_context.output_logical_base : default_base)
            : (g_context.input_logical_base_valid ? g_context.input_logical_base : default_base);

    if (logical_address >= learned_base) {
        const uint32_t relative = logical_address - learned_base;
        const uint16_t process_base = for_output_write ? kPdoCommandOffset : kPdoStatusOffset;
        const uint16_t window_length = for_output_write
                                           ? static_cast<uint16_t>(sizeof(EthercatPdoCommand))
                                           : static_cast<uint16_t>(sizeof(EthercatPdoStatus));
        if (relative + data_length <= window_length) {
            *offset = static_cast<uint16_t>(process_base + relative);
            return true;
        }
    }

    // Legacy fixed fallback.
    if (logical_address >= kLogicalProcessBase) {
        const uint32_t relative = logical_address - kLogicalProcessBase;
        if (relative + data_length <= kProcessImageSize) {
            const uint16_t process_base = for_output_write ? kPdoCommandOffset : kPdoStatusOffset;
            *offset = static_cast<uint16_t>(process_base + relative);
            return true;
        }
    }

    if (logical_address >= kLogicalProcessAltBase) {
        const uint32_t relative = logical_address - kLogicalProcessAltBase;
        if (relative + data_length <= kProcessImageSize) {
            const uint16_t process_base = for_output_write ? kPdoCommandOffset : kPdoStatusOffset;
            *offset = static_cast<uint16_t>(process_base + relative);
            return true;
        }
    }

    return false;
}

err_t SendEthercatPayload(const struct eth_addr &destination_mac,
                          const uint8_t *payload, uint16_t payload_length) {
    if (g_context.netif == nullptr || payload == nullptr || payload_length == 0U) {
        g_context.stats.tx_drop++;
        return ERR_ARG;
    }

    struct pbuf *packet = pbuf_alloc(PBUF_LINK, payload_length, PBUF_RAM);
    if (packet == nullptr) {
        g_context.stats.tx_drop++;
        return ERR_MEM;
    }

    const err_t copy_result = pbuf_take(packet, payload, payload_length);
    if (copy_result != ERR_OK) {
        g_context.stats.tx_drop++;
        pbuf_free(packet);
        return copy_result;
    }

    struct eth_addr source_mac = {};
    std::memcpy(source_mac.addr, g_context.netif->hwaddr, ETH_HWADDR_LEN);
    const err_t send_result = ethernet_output(g_context.netif, packet, &source_mac,
                                              &destination_mac, ETHTYPE_ETHERCAT);
    pbuf_free(packet);

    if (send_result == ERR_OK) {
        g_context.stats.tx_ok++;
    } else {
        g_context.stats.tx_drop++;
    }
    return send_result;
}

bool ProcessDatagramInPlace(uint8_t *payload, uint16_t payload_length) {
    if (payload == nullptr || payload_length < kEthercatHeaderSize) {
        return false;
    }

    const uint16_t ethercat_length = static_cast<uint16_t>(ReadLe16(payload) & 0x07FFU);
    const uint16_t usable_length = static_cast<uint16_t>(payload_length - kEthercatHeaderSize);
    if (ethercat_length > usable_length || ethercat_length < kEthercatDatagramHeaderSize) {
        return false;
    }

    uint8_t *datagram = payload + kEthercatHeaderSize;
    uint16_t remaining = ethercat_length;

    while (remaining >= (kEthercatDatagramHeaderSize + kEthercatWkcSize)) {
        const uint8_t command = datagram[0];
        const uint16_t adp = ReadLe16(&datagram[2]);
        const uint16_t ado = ReadLe16(&datagram[4]);
        const uint32_t logical_address =
            static_cast<uint32_t>(adp) | (static_cast<uint32_t>(ado) << 16U);
        const uint16_t len_flags = ReadLe16(&datagram[6]);
        const uint16_t data_length = static_cast<uint16_t>(len_flags & 0x07FFU);
        const uint16_t datagram_total = static_cast<uint16_t>(
            kEthercatDatagramHeaderSize + data_length + kEthercatWkcSize);

        if (datagram_total > remaining) {
            return false;
        }

        uint8_t *data = datagram + kEthercatDatagramHeaderSize;
        uint16_t wkc = 0U;

        if (IsLogicalCommand(command)) {
            if (command == kEcCmdLrd) {
                uint16_t input_offset = 0U;
                if (ResolveLogicalProcessWindow(logical_address, data_length, false, &input_offset)) {
                    RefreshStatusFromCommandImage();
                    SyncStatusImageToProcessImage();
                    std::memcpy(data, &g_context.process_image[input_offset], data_length);
                    wkc = 1U;
                }
                if (data_length >= 2U) {
                    // Deterministic fallback: always stamp status in logical read payload.
                    WriteLe16(data, g_context.status_image.status_word);
                    wkc = 1U;
                }
            } else if (command == kEcCmdLwr) {
                if (data_length >= 2U) {
                    ApplyControlWordMirror(ReadLe16(data));
                }
                uint16_t output_offset = 0U;
                if (ResolveLogicalProcessWindow(logical_address, data_length, true, &output_offset)) {
                    std::memcpy(&g_context.process_image[output_offset], data, data_length);
                    SyncProcessImageToCommandImage();
                    SyncStatusImageToProcessImage();
                    wkc = 1U;
                }
            } else if (command == kEcCmdLrw) {
                if (data_length >= 2U) {
                    ApplyControlWordMirror(ReadLe16(data));
                }
                uint16_t sm2_start = 0U;
                uint16_t sm2_length = static_cast<uint16_t>(sizeof(EthercatPdoCommand));
                uint16_t sm3_start = 0U;
                uint16_t sm3_length = static_cast<uint16_t>(sizeof(EthercatPdoStatus));
                if (!ReadSmWindow(2U, &sm2_start, &sm2_length)) {
                    sm2_length = static_cast<uint16_t>(sizeof(EthercatPdoCommand));
                }
                if (!ReadSmWindow(3U, &sm3_start, &sm3_length)) {
                    sm3_length = static_cast<uint16_t>(sizeof(EthercatPdoStatus));
                }

                const uint16_t output_bytes = static_cast<uint16_t>(sm2_length);
                const uint16_t input_bytes = static_cast<uint16_t>(sm3_length);
                const uint16_t output_copy_len =
                    (data_length < output_bytes) ? data_length : output_bytes;
                if (output_copy_len > 0U) {
                    std::memcpy(&g_context.process_image[kPdoCommandOffset], data, output_copy_len);
                }
                SyncProcessImageToCommandImage();
                SyncStatusImageToProcessImage();

                if (data_length > output_bytes) {
                    const uint16_t remaining = static_cast<uint16_t>(data_length - output_bytes);
                    const uint16_t input_copy_len =
                        (remaining < input_bytes) ? remaining : input_bytes;
                    std::memcpy(data + output_bytes, &g_context.process_image[kPdoStatusOffset],
                                input_copy_len);
                    if (input_copy_len >= 2U) {
                        WriteLe16(data + output_bytes, g_context.status_image.status_word);
                    }
                }

                // LRW contributes read + write + slave selection.
                wkc = 3U;
            }
        } else if (IsAddressedToThisSlave(command, adp)) {
            if (IsReadCommand(command)) {
                ReadEscWindow(ado, data, data_length);
                wkc = 1U;
            } else if (IsWriteCommand(command)) {
                WriteEscWindow(ado, data, data_length);
                wkc = 1U;
            } else if (IsReadWriteCommand(command)) {
                WriteEscWindow(ado, data, data_length);
                ReadEscWindow(ado, data, data_length);
                wkc = 1U;
            }
        }

        WriteLe16(data + data_length, wkc);

        const bool has_next = (len_flags & 0x8000U) != 0U;
        if (!has_next) {
            break;
        }

        datagram += datagram_total;
        remaining = static_cast<uint16_t>(remaining - datagram_total);
    }

    return true;
}

}  // namespace

void EthercatSlaveInit(struct netif *netif) {
    g_context.netif = netif;
    std::memset(&g_context.command_image, 0, sizeof(g_context.command_image));
    std::memset(&g_context.status_image, 0, sizeof(g_context.status_image));
    std::memset(&g_context.stats, 0, sizeof(g_context.stats));
    g_context.configured_station_address = 0U;
    g_context.output_logical_base = 0U;
    g_context.input_logical_base = 0U;
    g_context.output_logical_base_valid = false;
    g_context.input_logical_base_valid = false;
    InitializeEscRegisterMap();
    SyncStatusImageToProcessImage();
    std::memset(&g_context.last_command_source, 0, sizeof(g_context.last_command_source));
    g_context.last_command_source_valid = false;
}

void EthercatSlaveCyclic() {
    RefreshStatusFromCommandImage();
    SyncStatusImageToProcessImage();
}

err_t EthercatSlaveHandleFrame(struct pbuf *frame, struct netif *netif) {
    if (frame == nullptr || netif == nullptr) {
        g_context.stats.rx_drop++;
        return ERR_ARG;
    }

    const uint16_t minimum_length =
        static_cast<uint16_t>(SIZEOF_ETH_HDR + kEthercatHeaderSize + kEthercatDatagramHeaderSize +
                              kEthercatWkcSize);
    if (frame->tot_len < minimum_length) {
        g_context.stats.rx_drop++;
        return ERR_VAL;
    }

    struct eth_hdr header = {};
    const uint16_t copied = pbuf_copy_partial(frame, &header, SIZEOF_ETH_HDR, 0U);
    if (copied != SIZEOF_ETH_HDR) {
        g_context.stats.rx_drop++;
        return ERR_VAL;
    }

    const uint16_t payload_length =
        static_cast<uint16_t>(frame->tot_len - SIZEOF_ETH_HDR);
    if (payload_length > kEthercatMaxPayload) {
        g_context.stats.rx_drop++;
        return ERR_VAL;
    }

    uint8_t payload[kEthercatMaxPayload];
    const uint16_t payload_copied =
        pbuf_copy_partial(frame, payload, payload_length, SIZEOF_ETH_HDR);
    if (payload_copied != payload_length) {
        g_context.stats.rx_drop++;
        return ERR_VAL;
    }

    if (!ProcessDatagramInPlace(payload, payload_length)) {
        g_context.stats.rx_drop++;
        return ERR_VAL;
    }

    g_context.last_command_source = header.src;
    g_context.last_command_source_valid = true;
    g_context.netif = netif;
    g_context.stats.rx_ok++;
    return SendEthercatPayload(header.src, payload, payload_length);
}

err_t EthercatSlaveSendStatusFrame(const struct eth_addr &destination_mac) {
    return SendEthercatPayload(destination_mac,
                               reinterpret_cast<const uint8_t *>(&g_context.status_image),
                               sizeof(g_context.status_image));
}

const EthercatPdoCommand &CommandImage() {
    return g_context.command_image;
}

EthercatPdoStatus &StatusImage() {
    return g_context.status_image;
}

const EthercatSlaveStats &Stats() {
    return g_context.stats;
}

}  // namespace ethercat_slave

extern "C" err_t EthercatSlave_LwipUnknownEthProtocolHook(struct pbuf *packet,
                                                           struct netif *netif) {
    if (packet == nullptr || netif == nullptr) {
        return ERR_ARG;
    }

    struct eth_hdr header = {};
    const uint16_t copied = pbuf_copy_partial(packet, &header, SIZEOF_ETH_HDR, 0U);
    if (copied != SIZEOF_ETH_HDR) {
        return ERR_VAL;
    }

    if (header.type != PP_HTONS(ETHTYPE_ETHERCAT)) {
        return ERR_VAL;
    }

    const err_t status = ethercat_slave::EthercatSlaveHandleFrame(packet, netif);
    if (status == ERR_OK) {
        pbuf_free(packet);
    }
    return status;
}
