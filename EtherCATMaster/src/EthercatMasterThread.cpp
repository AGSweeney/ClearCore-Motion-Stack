#include "EthercatMasterThread.h"

#include <QMutexLocker>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <thread>

extern "C"
{
#include <soem/soem.h>
}

namespace
{
constexpr std::size_t kIoMapSize = 4096;
constexpr std::uint32_t kManualControlLogicalAddress = 0x0000U;
constexpr std::uint32_t kManualStatusLogicalAddress = 0x0100U;
constexpr std::uint16_t kCommandPdoLength = 40U;
constexpr std::uint16_t kStatusPdoLength = 64U;
constexpr std::uint32_t kCommandReserved0ByteOffset = 3U;
constexpr std::uint32_t kCommandReserved1ByteOffset = 14U;
constexpr std::uint32_t kCommandCcioControlByteOffset = 20U;
constexpr std::uint32_t kCommandCcioOutputByteOffset = 24U;
constexpr std::uint32_t kCommandCcioDirectionByteOffset = 32U;
constexpr std::uint16_t kCommandDirectionValidMask = 0x8000U;
constexpr std::uint16_t kCommandCcioEnableMask = 0x0001U;
constexpr std::uint32_t kCommandControlWordByteOffset = 0U;
constexpr std::uint32_t kStatusFaultCodeByteOffset = 3U;
constexpr std::uint32_t kStatusModeDisplayByteOffset = 2U;
constexpr std::uint32_t kStatusActualPositionByteOffset = 4U;
constexpr std::uint32_t kStatusActualVelocityByteOffset = 8U;
constexpr std::uint32_t kStatusSupplyCentivoltsByteOffset = 12U;
constexpr std::uint32_t kStatusReserved0ByteOffset = 14U;
constexpr std::uint32_t kStatusSequenceAckByteOffset = 16U;
constexpr std::uint32_t kStatusCcioStatusByteOffset = 20U;
constexpr std::uint32_t kStatusCcioBoardCountByteOffset = 22U;
constexpr std::uint32_t kStatusCcioInputByteOffset = 24U;
constexpr std::uint32_t kStatusCcioStatusBitsByteOffset = 32U;
constexpr std::uint32_t kStatusCcioDirectionByteOffset = 40U;
constexpr std::uint32_t kStatusFirmwareLoopPeriodByteOffset = 48U;
constexpr std::uint32_t kStatusFirmwareLoopRuntimeByteOffset = 52U;
constexpr std::uint32_t kStatusFirmwareLoopJitterByteOffset = 56U;
constexpr std::uint32_t kStatusFirmwareTransportByteOffset = 60U;
constexpr std::uint16_t kStatusCcioEnabledMask = 0x0001U;
constexpr std::uint16_t kStatusCcioLinkBrokenMask = 0x0002U;
constexpr std::uint16_t kStatusCcioAnyOverloadMask = 0x0004U;
constexpr auto kUiUpdatePeriod = std::chrono::milliseconds(20);

std::uint32_t readBitsLe(const std::uint8_t* data, const std::uint32_t byteLength, const std::uint32_t startBit, const std::uint8_t width)
{
    if (data == nullptr || byteLength == 0U || width == 0U || width > 32U)
    {
        return 0U;
    }

    std::uint32_t value = 0U;
    for (std::uint8_t i = 0; i < width; ++i)
    {
        const std::uint32_t absoluteBit = startBit + i;
        const std::uint32_t byteIndex = absoluteBit / 8U;
        if (byteIndex >= byteLength)
        {
            break;
        }

        const std::uint8_t bitIndex = static_cast<std::uint8_t>(absoluteBit % 8U);
        const std::uint8_t bitValue = static_cast<std::uint8_t>((data[byteIndex] >> bitIndex) & 0x01U);
        value = static_cast<std::uint32_t>(value | (static_cast<std::uint32_t>(bitValue) << i));
    }
    return value;
}

void writeBitsLe(std::uint8_t* data, const std::uint32_t byteLength, const std::uint32_t startBit, const std::uint8_t width, const std::uint32_t value)
{
    if (data == nullptr || byteLength == 0U || width == 0U || width > 32U)
    {
        return;
    }

    for (std::uint8_t i = 0; i < width; ++i)
    {
        const std::uint32_t absoluteBit = startBit + i;
        const std::uint32_t byteIndex = absoluteBit / 8U;
        if (byteIndex >= byteLength)
        {
            break;
        }

        const std::uint8_t bitIndex = static_cast<std::uint8_t>(absoluteBit % 8U);
        const std::uint8_t bitMask = static_cast<std::uint8_t>(1U << bitIndex);
        const bool set = ((value >> i) & 0x01U) != 0U;
        if (set)
        {
            data[byteIndex] = static_cast<std::uint8_t>(data[byteIndex] | bitMask);
        }
        else
        {
            data[byteIndex] = static_cast<std::uint8_t>(data[byteIndex] & static_cast<std::uint8_t>(~bitMask));
        }
    }
}

std::uint16_t readU16At(const std::uint8_t* data, const std::uint32_t byteLength, const std::uint32_t startBit, const std::uint32_t byteOffset)
{
    return static_cast<std::uint16_t>(readBitsLe(data, byteLength, startBit + (byteOffset * 8U), 16U));
}

std::uint32_t readU32At(const std::uint8_t* data, const std::uint32_t byteLength, const std::uint32_t startBit, const std::uint32_t byteOffset)
{
    return readBitsLe(data, byteLength, startBit + (byteOffset * 8U), 32U);
}

std::uint64_t readU64At(const std::uint8_t* data, const std::uint32_t byteLength, const std::uint32_t byteOffset)
{
    if (data == nullptr || byteOffset + 8U > byteLength)
    {
        return 0U;
    }
    std::uint64_t value = 0U;
    for (std::uint32_t i = 0; i < 8U; ++i)
    {
        value |= static_cast<std::uint64_t>(data[byteOffset + i]) << (8U * i);
    }
    return value;
}

void writeU16At(std::uint8_t* data, const std::uint32_t byteLength, const std::uint32_t startBit, const std::uint32_t byteOffset, const std::uint16_t value)
{
    writeBitsLe(data, byteLength, startBit + (byteOffset * 8U), 16U, value);
}

void writeU64At(std::uint8_t* data, const std::uint32_t byteLength, const std::uint32_t byteOffset, const std::uint64_t value)
{
    if (data == nullptr || byteOffset + 8U > byteLength)
    {
        return;
    }
    for (std::uint32_t i = 0; i < 8U; ++i)
    {
        data[byteOffset + i] = static_cast<std::uint8_t>((value >> (8U * i)) & 0xFFU);
    }
}

void encodeCommandPdo(std::uint8_t* outputs, const std::uint32_t outputLength,
                      const std::uint32_t startBit, const std::uint8_t ioLevels,
                      const std::uint8_t ioDirections, const std::uint8_t aAnalogMask,
                      const bool ccioEnabled, const std::uint64_t ccioOutputs,
                      const std::uint64_t ccioDirections)
{
    writeU16At(outputs, outputLength, startBit, kCommandControlWordByteOffset,
               static_cast<std::uint16_t>(ioLevels & 0x3FU));
    writeBitsLe(outputs, outputLength, startBit + (kCommandReserved0ByteOffset * 8U), 8U,
                aAnalogMask & 0x0FU);
    writeU16At(outputs, outputLength, startBit, kCommandReserved1ByteOffset,
               static_cast<std::uint16_t>(kCommandDirectionValidMask | (ioDirections & 0x3FU)));
    writeU16At(outputs, outputLength, startBit, kCommandCcioControlByteOffset,
               ccioEnabled ? kCommandCcioEnableMask : 0U);
    if (startBit == 0U)
    {
        writeU64At(outputs, outputLength, kCommandCcioOutputByteOffset, ccioOutputs);
        writeU64At(outputs, outputLength, kCommandCcioDirectionByteOffset, ccioDirections);
    }
}

void decodeStatusPdo(const std::uint8_t* inputs, const std::uint32_t inputLength,
                     const std::uint32_t startBit, MasterSnapshot* snapshot)
{
    if (inputs == nullptr || snapshot == nullptr)
    {
        return;
    }

    const std::uint16_t statusWord = readU16At(inputs, inputLength, startBit, kCommandControlWordByteOffset);
    const std::uint16_t reserved0 = readU16At(inputs, inputLength, startBit, kStatusReserved0ByteOffset);
    const std::uint8_t modeDisplay = static_cast<std::uint8_t>(
        readBitsLe(inputs, inputLength, startBit + (kStatusModeDisplayByteOffset * 8U), 8U));
    const std::uint32_t actualPosition = readU32At(inputs, inputLength, startBit, kStatusActualPositionByteOffset);
    const std::uint32_t actualVelocity = readU32At(inputs, inputLength, startBit, kStatusActualVelocityByteOffset);
    const std::uint16_t ccioStatus = readU16At(inputs, inputLength, startBit, kStatusCcioStatusByteOffset);

    snapshot->statusWord = statusWord;
    snapshot->ioState = static_cast<quint8>(statusWord & 0x003FU);
    snapshot->ioDirectionApplied = static_cast<quint8>((reserved0 >> 8U) & 0x003FU);
    snapshot->faultCode = static_cast<std::uint8_t>(
        readBitsLe(inputs, inputLength, startBit + (kStatusFaultCodeByteOffset * 8U), 8U));
    snapshot->diBits = static_cast<quint8>(reserved0 & 0x07U);
    snapshot->aDigitalBits = static_cast<quint8>((reserved0 >> 3U) & 0x0FU);
    snapshot->aAnalogMaskApplied = static_cast<quint8>(modeDisplay & 0x0FU);
    snapshot->a9Raw = static_cast<quint16>(actualPosition & 0xFFFFU);
    snapshot->a10Raw = static_cast<quint16>((actualPosition >> 16U) & 0xFFFFU);
    snapshot->a11Raw = static_cast<quint16>(actualVelocity & 0xFFFFU);
    snapshot->a12Raw = static_cast<quint16>((actualVelocity >> 16U) & 0xFFFFU);
    snapshot->supplyCentivolts = readU16At(inputs, inputLength, startBit, kStatusSupplyCentivoltsByteOffset);
    snapshot->sequenceAck = readU32At(inputs, inputLength, startBit, kStatusSequenceAckByteOffset);
    snapshot->ccioEnabled = (ccioStatus & kStatusCcioEnabledMask) != 0U;
    snapshot->ccioLinkBroken = (ccioStatus & kStatusCcioLinkBrokenMask) != 0U;
    snapshot->ccioAnyOverload = (ccioStatus & kStatusCcioAnyOverloadMask) != 0U;
    snapshot->ccioBoardCount = static_cast<quint8>(
        readBitsLe(inputs, inputLength, startBit + (kStatusCcioBoardCountByteOffset * 8U), 8U));
    if (startBit == 0U)
    {
        snapshot->ccioInputBits = readU64At(inputs, inputLength, kStatusCcioInputByteOffset);
        snapshot->ccioStatusBits = readU64At(inputs, inputLength, kStatusCcioStatusBitsByteOffset);
        snapshot->ccioDirectionApplied = readU64At(inputs, inputLength, kStatusCcioDirectionByteOffset);
        snapshot->firmwareLoopPeriodUs = readU32At(inputs, inputLength, startBit, kStatusFirmwareLoopPeriodByteOffset);
        snapshot->firmwareLoopRuntimeUs = readU32At(inputs, inputLength, startBit, kStatusFirmwareLoopRuntimeByteOffset);
        snapshot->firmwareLoopJitterUs = readU32At(inputs, inputLength, startBit, kStatusFirmwareLoopJitterByteOffset);
        snapshot->firmwareTransportUs = readU32At(inputs, inputLength, startBit, kStatusFirmwareTransportByteOffset);
        snapshot->isrTimingText = QStringLiteral("%1 us").arg(snapshot->firmwareTransportUs);
    }
}

int combineWkc(const int a, const int b)
{
    if (a < 0 || b < 0)
    {
        return -1;
    }
    return a + b;
}
}

EthercatMasterThread::EthercatMasterThread(QObject* parent)
    : QThread(parent)
{
}

void EthercatMasterThread::configure(const QString& adapterName, const int cycleTimeMs)
{
    QMutexLocker lock(&configMutex_);
    adapterName_ = adapterName.trimmed();
    cycleTimeMs_ = std::max(1, cycleTimeMs);
    configured_ = true;
    stopRequested_.store(false);
}

void EthercatMasterThread::requestStop()
{
    stopRequested_.store(true);
}

void EthercatMasterThread::setIoLevelMask(const std::uint8_t levelMask)
{
    ioLevelMask_.store(static_cast<std::uint8_t>(levelMask & 0x3FU));
}

void EthercatMasterThread::setIoDirectionMask(const std::uint8_t directionMask)
{
    ioDirectionMask_.store(static_cast<std::uint8_t>(directionMask & 0x3FU));
}

void EthercatMasterThread::setAAnalogMask(const std::uint8_t analogMask)
{
    aAnalogMask_.store(static_cast<std::uint8_t>(analogMask & 0x0FU));
}

void EthercatMasterThread::setCcioControl(const bool enabled, const std::uint64_t outputMask, const std::uint64_t directionMask)
{
    ccioEnabled_.store(enabled);
    ccioOutputMask_.store(outputMask);
    ccioDirectionMask_.store(directionMask);
}

QString EthercatMasterThread::stateToText(const std::uint16_t state) const
{
    if ((state & EC_STATE_ERROR) != 0U)
    {
        return QStringLiteral("ERROR");
    }

    switch (state & 0x0FU)
    {
    case EC_STATE_NONE:
        return QStringLiteral("NONE");
    case EC_STATE_INIT:
        return QStringLiteral("INIT");
    case EC_STATE_PRE_OP:
        return QStringLiteral("PRE-OP");
    case EC_STATE_BOOT:
        return QStringLiteral("BOOT");
    case EC_STATE_SAFE_OP:
        return QStringLiteral("SAFE-OP");
    case EC_STATE_OPERATIONAL:
        return QStringLiteral("OP");
    default:
        return QStringLiteral("UNKNOWN");
    }
}

void EthercatMasterThread::run()
{
    QString adapterName;
    int cycleMs = 10;
    {
        QMutexLocker lock(&configMutex_);
        if (!configured_)
        {
            emit logMessage(QStringLiteral("Thread started without configuration."));
            return;
        }
        adapterName = adapterName_;
        cycleMs = cycleTimeMs_;
    }

    if (adapterName.isEmpty())
    {
        emit logMessage(QStringLiteral("Adapter name is empty. Example: \\Device\\NPF_{...}"));
        return;
    }

    MasterSnapshot snapshot;
    snapshot.loopTimeMs = static_cast<double>(cycleMs);

    ecx_contextt context{};
    ecx_contextt* ctx = &context;
    ec_groupt* group = ctx->grouplist;

    std::array<std::uint8_t, kIoMapSize> ioMap{};

    const QByteArray adapterBytes = adapterName.toLocal8Bit();
    if (ecx_init(ctx, adapterBytes.constData()) <= 0)
    {
        snapshot.lastError = QStringLiteral("ecx_init failed. Ensure Npcap is installed and run as Administrator.");
        emit snapshotUpdated(snapshot);
        emit logMessage(snapshot.lastError);
        return;
    }

    emit logMessage(QStringLiteral("EtherCAT adapter opened: %1").arg(adapterName));

    const int slaveCount = ecx_config_init(ctx);
    snapshot.slaveCount = slaveCount;
    snapshot.connected = slaveCount > 0;

    if (slaveCount <= 0)
    {
        snapshot.lastError = QStringLiteral("No EtherCAT slaves found.");
        emit snapshotUpdated(snapshot);
        emit logMessage(snapshot.lastError);
        ecx_close(ctx);
        return;
    }

    emit logMessage(QStringLiteral("Discovered %1 slave(s).").arg(slaveCount));

    ecx_config_map_group(ctx, ioMap.data(), 0);
    ecx_configdc(ctx);

    ecx_statecheck(ctx, 0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

    ec_slavet* slave1 = (slaveCount >= 1) ? &ctx->slavelist[1] : nullptr;
    const bool mappedOutputsValid = (slave1 != nullptr) &&
                                     (slave1->Obytes >= kCommandPdoLength) &&
                                     (slave1->outputs != nullptr);
    const bool mappedInputsValid = (slave1 != nullptr) &&
                                   (slave1->Ibytes >= kStatusPdoLength) &&
                                   (slave1->inputs != nullptr);
    const bool hasMappedProcessData = mappedOutputsValid && mappedInputsValid;
    const bool useManualIo = !hasMappedProcessData;

    snapshot.expectedWkc = (group->outputsWKC * 2) + group->inputsWKC;
    if (useManualIo)
    {
        snapshot.expectedWkc = 2;
    }
    snapshot.stateText = stateToText(ctx->slavelist[0].state);

    emit logMessage(QStringLiteral("Mapped IO check (slave 1): Obytes=%1 Ibytes=%2 outputs=%3 inputs=%4")
                        .arg(slave1 != nullptr ? static_cast<qulonglong>(slave1->Obytes) : 0ULL)
                        .arg(slave1 != nullptr ? static_cast<qulonglong>(slave1->Ibytes) : 0ULL)
                        .arg(mappedOutputsValid ? QStringLiteral("ok") : QStringLiteral("missing"))
                        .arg(mappedInputsValid ? QStringLiteral("ok") : QStringLiteral("missing")));
    if (slave1 != nullptr)
    {
        emit logMessage(QStringLiteral("Mapped bit offsets (slave 1): Ostartbit=%1 Istartbit=%2")
                            .arg(slave1->Ostartbit)
                            .arg(slave1->Istartbit));
    }

    if (useManualIo)
    {
        emit logMessage(QStringLiteral("Mapped PDO bytes missing or too small for CCIO. Using manual I/O via logical addresses 0x000 and 0x100."));
    }
    emit snapshotUpdated(snapshot);

    ctx->slavelist[0].state = EC_STATE_OPERATIONAL;
    if (useManualIo)
    {
        const std::uint8_t ioLevels = static_cast<std::uint8_t>(ioLevelMask_.load() & 0x3FU);
        const std::uint8_t ioDirs = static_cast<std::uint8_t>(ioDirectionMask_.load() & 0x3FU);
        const std::uint8_t aAnalogMask = static_cast<std::uint8_t>(aAnalogMask_.load() & 0x0FU);
        const bool ccioEnabled = ccioEnabled_.load();
        const std::uint64_t ccioOutputs = ccioOutputMask_.load();
        const std::uint64_t ccioDirections = ccioDirectionMask_.load();
        std::array<std::uint8_t, kCommandPdoLength> manualOutputs{};
        std::array<std::uint8_t, kStatusPdoLength> manualInputs{};
        encodeCommandPdo(manualOutputs.data(), kCommandPdoLength, 0U, ioLevels, ioDirs,
                         aAnalogMask, ccioEnabled, ccioOutputs, ccioDirections);

        const int outWkc = ecx_LWR(&ctx->port, kManualControlLogicalAddress, kCommandPdoLength,
                                   manualOutputs.data(), EC_TIMEOUTRET);
        const int inWkc = ecx_LRD(&ctx->port, kManualStatusLogicalAddress, kStatusPdoLength,
                                  manualInputs.data(), EC_TIMEOUTRET);
        snapshot.lastWkc = combineWkc(outWkc, inWkc);
        snapshot.controlWord = static_cast<quint16>(
            readU16At(manualOutputs.data(), kCommandPdoLength, 0U, kCommandControlWordByteOffset));
        snapshot.ioLevelRequest = static_cast<quint8>(ioLevels);
        snapshot.ioDirectionRequest = ioDirs;
        snapshot.ccioOutputRequest = ccioOutputs;
        snapshot.ccioDirectionRequest = ccioDirections;
        decodeStatusPdo(manualInputs.data(), kStatusPdoLength, 0U, &snapshot);
    }
    else
    {
        ecx_send_processdata(ctx);
        ecx_receive_processdata(ctx, EC_TIMEOUTRET);
    }
    ecx_writestate(ctx, 0);

    const int opState = ecx_statecheck(ctx, 0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE * 8);
    snapshot.operational = (opState == EC_STATE_OPERATIONAL);
    snapshot.stateText = stateToText(ctx->slavelist[0].state);
    if (!snapshot.operational)
    {
        snapshot.lastError = QStringLiteral("Unable to reach OP state. Slave state: %1").arg(snapshot.stateText);
        emit snapshotUpdated(snapshot);
        emit logMessage(snapshot.lastError);
        ctx->slavelist[0].state = EC_STATE_INIT;
        ecx_writestate(ctx, 0);
        ecx_close(ctx);
        return;
    }

    emit logMessage(QStringLiteral("Bus reached OPERATIONAL."));

    auto lastUiUpdate = std::chrono::steady_clock::now() - kUiUpdatePeriod;
    auto previousCycleStart = std::chrono::steady_clock::now();
    bool hasPreviousCycleStart = false;
    while (!stopRequested_.load())
    {
        const auto cycleStart = std::chrono::steady_clock::now();
        if (hasPreviousCycleStart)
        {
            const double periodMs = std::chrono::duration<double, std::milli>(cycleStart - previousCycleStart).count();
            const double jitterMs = (periodMs > static_cast<double>(cycleMs))
                                        ? periodMs - static_cast<double>(cycleMs)
                                        : static_cast<double>(cycleMs) - periodMs;
            snapshot.cyclePeriodMs = periodMs;
            snapshot.loopJitterMs = jitterMs;
            snapshot.maxLoopJitterMs = std::max(snapshot.maxLoopJitterMs, jitterMs);
            snapshot.updateConsistencyMs = jitterMs;
            snapshot.maxUpdateConsistencyMs = std::max(snapshot.maxUpdateConsistencyMs, jitterMs);
        }
        previousCycleStart = cycleStart;
        hasPreviousCycleStart = true;

        const std::uint8_t requestedIoLevels = static_cast<std::uint8_t>(ioLevelMask_.load() & 0x3FU);
        const std::uint8_t requestedIoDirections = static_cast<std::uint8_t>(ioDirectionMask_.load() & 0x3FU);
        const std::uint8_t requestedAAnalogMask = static_cast<std::uint8_t>(aAnalogMask_.load() & 0x0FU);
        const bool requestedCcioEnabled = ccioEnabled_.load();
        const std::uint64_t requestedCcioOutputs = ccioOutputMask_.load();
        const std::uint64_t requestedCcioDirections = ccioDirectionMask_.load();
        const std::uint16_t requestedControlWord = requestedIoLevels;
        const std::uint16_t controlWord = requestedControlWord;
        snapshot.controlWord = controlWord;
        snapshot.ioLevelRequest = static_cast<quint8>(controlWord & 0x003FU);
        snapshot.ioDirectionRequest = requestedIoDirections;
        snapshot.aAnalogMaskRequest = requestedAAnalogMask;
        snapshot.ccioOutputRequest = requestedCcioOutputs;
        snapshot.ccioDirectionRequest = requestedCcioDirections;

        if (useManualIo)
        {
            std::array<std::uint8_t, kCommandPdoLength> manualOutputs{};
            std::array<std::uint8_t, kStatusPdoLength> manualInputs{};
            encodeCommandPdo(manualOutputs.data(), kCommandPdoLength, 0U, requestedIoLevels,
                             requestedIoDirections, requestedAAnalogMask, requestedCcioEnabled,
                             requestedCcioOutputs, requestedCcioDirections);

            const auto frameStart = std::chrono::steady_clock::now();
            const int outWkc = ecx_LWR(&ctx->port, kManualControlLogicalAddress, kCommandPdoLength,
                                       manualOutputs.data(), EC_TIMEOUTRET);
            const int inWkc = ecx_LRD(&ctx->port, kManualStatusLogicalAddress, kStatusPdoLength,
                                      manualInputs.data(), EC_TIMEOUTRET);
            const double frameLatencyMs =
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - frameStart).count();
            snapshot.frameLatencyMs = frameLatencyMs;
            snapshot.maxFrameLatencyMs = std::max(snapshot.maxFrameLatencyMs, frameLatencyMs);
            snapshot.lastWkc = combineWkc(outWkc, inWkc);
            decodeStatusPdo(manualInputs.data(), kStatusPdoLength, 0U, &snapshot);
        }
        else if (slave1 != nullptr)
        {
            auto* outputs = reinterpret_cast<std::uint8_t*>(slave1->outputs);
            encodeCommandPdo(outputs, slave1->Obytes, slave1->Ostartbit, requestedIoLevels,
                             requestedIoDirections, requestedAAnalogMask, requestedCcioEnabled,
                             requestedCcioOutputs, requestedCcioDirections);
        }

        int wkc = snapshot.lastWkc;
        if (!useManualIo)
        {
            const auto frameStart = std::chrono::steady_clock::now();
            ecx_send_processdata(ctx);
            wkc = ecx_receive_processdata(ctx, EC_TIMEOUTRET);
            const double frameLatencyMs =
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - frameStart).count();
            snapshot.frameLatencyMs = frameLatencyMs;
            snapshot.maxFrameLatencyMs = std::max(snapshot.maxFrameLatencyMs, frameLatencyMs);
            snapshot.lastWkc = wkc;
        }

        snapshot.connected = true;
        snapshot.operational = (ctx->slavelist[0].state == EC_STATE_OPERATIONAL);
        snapshot.stateText = stateToText(ctx->slavelist[0].state);
        snapshot.cycleCount++;

        if (useManualIo)
        {
            // Manual I/O path already decoded the full 20-byte status PDO above.
        }
        else if (slave1 != nullptr)
        {
            auto* inputs = reinterpret_cast<std::uint8_t*>(slave1->inputs);
            decodeStatusPdo(inputs, slave1->Ibytes, slave1->Istartbit, &snapshot);
        }
        else
        {
            snapshot.statusWord = 0U;
            snapshot.ioState = static_cast<quint8>(snapshot.statusWord & 0x003FU);
            snapshot.ioDirectionApplied = 0U;
            snapshot.faultCode = 0U;
            snapshot.diBits = 0U;
            snapshot.aDigitalBits = 0U;
            snapshot.aAnalogMaskApplied = 0U;
            snapshot.a9Raw = 0U;
            snapshot.a10Raw = 0U;
            snapshot.a11Raw = 0U;
            snapshot.a12Raw = 0U;
            snapshot.supplyCentivolts = 0U;
            snapshot.ccioEnabled = false;
            snapshot.ccioLinkBroken = false;
            snapshot.ccioAnyOverload = false;
            snapshot.ccioBoardCount = 0U;
            snapshot.ccioInputBits = 0U;
            snapshot.ccioStatusBits = 0U;
            snapshot.ccioDirectionApplied = 0U;
            snapshot.sequenceAck = 0U;
        }

        if (wkc < snapshot.expectedWkc)
        {
            if (useManualIo)
            {
                snapshot.lastError = QStringLiteral("Manual I/O WKC low (%1, expected >= %2).")
                                         .arg(wkc)
                                         .arg(snapshot.expectedWkc);
            }
            else
            {
                snapshot.lastError = QStringLiteral("Low WKC %1 (expected >= %2).")
                                         .arg(wkc)
                                         .arg(snapshot.expectedWkc);
            }
            ecx_readstate(ctx);
            if (ctx->slavelist[0].state != EC_STATE_OPERATIONAL)
            {
                ctx->slavelist[0].state = EC_STATE_OPERATIONAL;
                ecx_writestate(ctx, 0);
            }
        }
        else
        {
            snapshot.lastError.clear();
        }

        const auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - cycleStart).count();
        snapshot.loopTimeMs = elapsed;
        const auto now = std::chrono::steady_clock::now();
        if (now - lastUiUpdate >= kUiUpdatePeriod)
        {
            emit snapshotUpdated(snapshot);
            lastUiUpdate = now;
        }

        const auto targetSleep = std::chrono::milliseconds(cycleMs) - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - cycleStart);
        if (targetSleep.count() > 0)
        {
            std::this_thread::sleep_for(targetSleep);
        }
    }

    emit logMessage(QStringLiteral("Stopping EtherCAT loop."));
    ctx->slavelist[0].state = EC_STATE_INIT;
    ecx_writestate(ctx, 0);
    ecx_close(ctx);

    snapshot.operational = false;
    snapshot.stateText = QStringLiteral("STOPPED");
    emit snapshotUpdated(snapshot);
}
