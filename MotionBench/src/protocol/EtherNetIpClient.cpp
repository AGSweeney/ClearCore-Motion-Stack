#include "protocol/EtherNetIpClient.h"

#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QRandomGenerator>
#include <QtCore/QScopedValueRollback>
#include <QtEndian>
#include <QtNetwork/QUdpSocket>
#include <algorithm>

namespace motion_bench::protocol {

namespace {

constexpr std::uint16_t kCommandNop = 0x0000;
constexpr std::uint16_t kCommandListIdentity = 0x0063;
constexpr std::uint16_t kCommandRegisterSession = 0x0065;
constexpr std::uint16_t kCommandUnregisterSession = 0x0066;
constexpr std::uint16_t kCommandSendRRData = 0x006F;

constexpr std::uint8_t kServiceGetAttributeSingle = 0x0E;
constexpr std::uint8_t kServiceSetAttributeSingle = 0x10;

constexpr std::uint16_t kNullAddressItemType = 0x0000;
constexpr std::uint16_t kUnconnectedDataItemType = 0x00B2;

void AppendU16(QByteArray *bytes, std::uint16_t value) {
    const auto little = qToLittleEndian(value);
    bytes->append(reinterpret_cast<const char *>(&little), sizeof(little));
}

void AppendU32(QByteArray *bytes, std::uint32_t value) {
    const auto little = qToLittleEndian(value);
    bytes->append(reinterpret_cast<const char *>(&little), sizeof(little));
}

void AppendU64(QByteArray *bytes, std::uint64_t value) {
    const auto little = qToLittleEndian(value);
    bytes->append(reinterpret_cast<const char *>(&little), sizeof(little));
}

std::uint16_t ReadU16(const QByteArray &bytes, int offset) {
    return qFromLittleEndian<std::uint16_t>(
        reinterpret_cast<const unsigned char *>(bytes.constData() + offset));
}

std::uint32_t ReadU32(const QByteArray &bytes, int offset) {
    return qFromLittleEndian<std::uint32_t>(
        reinterpret_cast<const unsigned char *>(bytes.constData() + offset));
}

}  // namespace

EtherNetIpClient::EtherNetIpClient(QObject *parent) : QObject(parent) {
    sender_context_ = static_cast<std::uint64_t>(QDateTime::currentMSecsSinceEpoch())
        ^ QRandomGenerator::global()->generate64();
}

bool EtherNetIpClient::Connect(const QString &ip_address, std::uint16_t port, int timeout_ms) {
    if (socket_.state() == QAbstractSocket::ConnectedState) {
        Disconnect(timeout_ms);
    }

    socket_.connectToHost(ip_address, port);
    if (!socket_.waitForConnected(timeout_ms)) {
        return false;
    }
    return RegisterSession(timeout_ms);
}

bool EtherNetIpClient::Disconnect(int timeout_ms) {
    if (socket_.state() != QAbstractSocket::ConnectedState) {
        session_handle_ = 0;
        return true;
    }
    UnregisterSession();
    socket_.disconnectFromHost();
    if (socket_.state() != QAbstractSocket::UnconnectedState) {
        socket_.waitForDisconnected(timeout_ms);
    }
    session_handle_ = 0;
    return true;
}

bool EtherNetIpClient::IsConnected() const {
    return socket_.state() == QAbstractSocket::ConnectedState && session_handle_ != 0;
}

CipResult EtherNetIpClient::GetAttributeSingle(const CipPath &path, int timeout_ms) {
    return SendExplicitMessage(kServiceGetAttributeSingle, path, QByteArray(), timeout_ms);
}

CipResult EtherNetIpClient::SetAttributeSingle(const CipPath &path, const QByteArray &payload, int timeout_ms) {
    return SendExplicitMessage(kServiceSetAttributeSingle, path, payload, timeout_ms);
}

QVector<ListIdentityResult> EtherNetIpClient::ListIdentity(int timeout_ms) {
    QVector<ListIdentityResult> results;
    QUdpSocket socket;

    if (!socket.bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        return results;
    }

    QByteArray request;
    AppendU16(&request, kCommandListIdentity);
    AppendU16(&request, 0);
    AppendU32(&request, 0);
    AppendU32(&request, 0);
    AppendU64(&request, 0);
    AppendU32(&request, 0);

    socket.writeDatagram(request, QHostAddress::Broadcast, 44818);
    socket.flush();

    const qint64 end_ms = QDateTime::currentMSecsSinceEpoch() + timeout_ms;
    while (QDateTime::currentMSecsSinceEpoch() < end_ms) {
        const int remaining = static_cast<int>(end_ms - QDateTime::currentMSecsSinceEpoch());
        if (!socket.waitForReadyRead(std::max(1, remaining))) {
            continue;
        }
        while (socket.hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(static_cast<int>(socket.pendingDatagramSize()));
            QHostAddress sender;
            quint16 sender_port = 0;
            socket.readDatagram(datagram.data(), datagram.size(), &sender, &sender_port);

            if (datagram.size() < 24) {
                continue;
            }
            const std::uint16_t command = ReadU16(datagram, 0);
            const std::uint16_t length = ReadU16(datagram, 2);
            if (command != kCommandListIdentity || datagram.size() < 24 + length || length < 2) {
                continue;
            }
            const QByteArray payload = datagram.mid(24, length);
            const std::uint16_t item_count = ReadU16(payload, 0);
            if (item_count == 0 || payload.size() < 6) {
                continue;
            }

            int offset = 2;
            for (std::uint16_t i = 0; i < item_count; ++i) {
                if (offset + 4 > payload.size()) {
                    break;
                }
                const std::uint16_t item_type = ReadU16(payload, offset);
                const std::uint16_t item_len = ReadU16(payload, offset + 2);
                offset += 4;
                if (offset + item_len > payload.size()) {
                    break;
                }
                if (item_type == 0x000C && item_len >= 33) {
                    const QByteArray item = payload.mid(offset, item_len);
                    ListIdentityResult result;
                    result.encapsulation_version = ReadU16(item, 0);
                    result.socket_port = qFromBigEndian<std::uint16_t>(
                        reinterpret_cast<const unsigned char *>(item.constData() + 2));
                    result.vendor_id = ReadU16(item, 14);
                    result.device_type = ReadU16(item, 16);
                    result.product_code = ReadU16(item, 18);
                    result.revision = ReadU16(item, 20);
                    result.serial_number = ReadU32(item, 22);
                    const std::uint8_t product_len = static_cast<std::uint8_t>(item.at(26));
                    if (27 + product_len <= item.size()) {
                        result.product_name = QString::fromLatin1(item.constData() + 27, product_len);
                    }
                    result.ip_address = sender.toString();
                    results.push_back(result);
                }
                offset += item_len;
            }
        }
    }
    return results;
}

EtherNetIpClient::EncapsulationPacket EtherNetIpClient::BuildPacket(
    std::uint16_t command,
    const QByteArray &payload) const {
    EncapsulationPacket packet;
    packet.command = command;
    packet.length = static_cast<std::uint16_t>(payload.size());
    packet.session_handle = session_handle_;
    packet.status = 0;
    packet.sender_context = sender_context_;
    packet.options = 0;
    packet.payload = payload;
    return packet;
}

QByteArray EtherNetIpClient::SerializePacket(const EncapsulationPacket &packet) const {
    QByteArray bytes;
    bytes.reserve(24 + packet.payload.size());
    AppendU16(&bytes, packet.command);
    AppendU16(&bytes, packet.length);
    AppendU32(&bytes, packet.session_handle);
    AppendU32(&bytes, packet.status);
    AppendU64(&bytes, packet.sender_context);
    AppendU32(&bytes, packet.options);
    bytes.append(packet.payload);
    return bytes;
}

bool EtherNetIpClient::ReadPacket(EncapsulationPacket *packet, int timeout_ms) {
    if (!socket_.waitForReadyRead(timeout_ms)) {
        return false;
    }

    QByteArray bytes = socket_.readAll();
    if (bytes.size() < 24) {
        return false;
    }
    const std::uint16_t payload_len = ReadU16(bytes, 2);
    while (bytes.size() < 24 + payload_len) {
        if (!socket_.waitForReadyRead(timeout_ms)) {
            return false;
        }
        bytes.append(socket_.readAll());
    }

    packet->command = ReadU16(bytes, 0);
    packet->length = payload_len;
    packet->session_handle = ReadU32(bytes, 4);
    packet->status = ReadU32(bytes, 8);
    packet->sender_context = qFromLittleEndian<std::uint64_t>(
        reinterpret_cast<const unsigned char *>(bytes.constData() + 12));
    packet->options = ReadU32(bytes, 20);
    packet->payload = bytes.mid(24, payload_len);
    return true;
}

bool EtherNetIpClient::WritePacket(const EncapsulationPacket &packet, int timeout_ms) {
    const QByteArray bytes = SerializePacket(packet);
    if (socket_.write(bytes) != bytes.size()) {
        return false;
    }
    return socket_.waitForBytesWritten(timeout_ms);
}

bool EtherNetIpClient::RegisterSession(int timeout_ms) {
    QByteArray payload;
    AppendU16(&payload, 1);  // protocol version
    AppendU16(&payload, 0);  // options
    const EncapsulationPacket request = BuildPacket(kCommandRegisterSession, payload);
    if (!WritePacket(request, timeout_ms)) {
        return false;
    }
    EncapsulationPacket response;
    if (!ReadPacket(&response, timeout_ms)) {
        return false;
    }
    if (response.command != kCommandRegisterSession || response.status != 0) {
        return false;
    }
    session_handle_ = response.session_handle;
    return session_handle_ != 0;
}

bool EtherNetIpClient::UnregisterSession() {
    if (session_handle_ == 0) {
        return true;
    }
    const EncapsulationPacket request = BuildPacket(kCommandUnregisterSession, QByteArray());
    WritePacket(request, 250);
    session_handle_ = 0;
    return true;
}

CipResult EtherNetIpClient::SendExplicitMessage(
    std::uint8_t service,
    const CipPath &path,
    const QByteArray &data,
    int timeout_ms) {
    CipResult result;
    if (!IsConnected()) {
        result.error = "Session is not connected.";
        return result;
    }

    QByteArray cip_payload;
    cip_payload.append(static_cast<char>(service));
    const QByteArray encoded_path = EncodePath(path);
    cip_payload.append(static_cast<char>(encoded_path.size() / 2));
    cip_payload.append(encoded_path);
    cip_payload.append(data);

    const QByteArray rr_payload = BuildSendRRData(cip_payload);
    const EncapsulationPacket request = BuildPacket(kCommandSendRRData, rr_payload);
    if (!WritePacket(request, timeout_ms)) {
        result.error = "Failed to send explicit message.";
        return result;
    }

    EncapsulationPacket response;
    if (!ReadPacket(&response, timeout_ms)) {
        result.error = "Timed out waiting for explicit response.";
        return result;
    }
    if (response.command != kCommandSendRRData) {
        result.error = "Unexpected encapsulation response command.";
        return result;
    }
    if (response.status != 0) {
        result.error = QString("Encapsulation status error: 0x%1")
                           .arg(response.status, 8, 16, QLatin1Char('0'));
        return result;
    }

    QByteArray cip_response;
    QString parse_error;
    if (!ParseSendRRDataResponse(response.payload, &cip_response, &parse_error)) {
        result.error = parse_error;
        return result;
    }
    return ParseCipResponse(cip_response);
}

QByteArray EtherNetIpClient::BuildSendRRData(const QByteArray &cip_payload) const {
    QByteArray payload;
    AppendU32(&payload, 0);  // interface handle
    AppendU16(&payload, 0);  // timeout
    AppendU16(&payload, 2);  // item count
    AppendU16(&payload, kNullAddressItemType);
    AppendU16(&payload, 0);
    AppendU16(&payload, kUnconnectedDataItemType);
    AppendU16(&payload, static_cast<std::uint16_t>(cip_payload.size()));
    payload.append(cip_payload);
    return payload;
}

bool EtherNetIpClient::ParseSendRRDataResponse(
    const QByteArray &payload,
    QByteArray *cip_response,
    QString *error) const {
    if (payload.size() < 10) {
        *error = "Malformed SendRRData payload.";
        return false;
    }
    const std::uint16_t item_count = ReadU16(payload, 6);
    int offset = 8;
    for (std::uint16_t i = 0; i < item_count; ++i) {
        if (offset + 4 > payload.size()) {
            *error = "Malformed CPF item header.";
            return false;
        }
        const std::uint16_t item_type = ReadU16(payload, offset);
        const std::uint16_t item_len = ReadU16(payload, offset + 2);
        offset += 4;
        if (offset + item_len > payload.size()) {
            *error = "Malformed CPF item payload.";
            return false;
        }
        if (item_type == kUnconnectedDataItemType) {
            *cip_response = payload.mid(offset, item_len);
            return true;
        }
        offset += item_len;
    }
    *error = "No unconnected data item in CPF response.";
    return false;
}

CipResult EtherNetIpClient::ParseCipResponse(const QByteArray &cip_response) const {
    CipResult result;
    if (cip_response.size() < 4) {
        result.error = "Malformed CIP response.";
        return result;
    }

    result.general_status = static_cast<std::uint8_t>(cip_response.at(2));
    const std::uint8_t ext_count = static_cast<std::uint8_t>(cip_response.at(3));

    const int ext_bytes = ext_count * 2;
    if (cip_response.size() < 4 + ext_bytes) {
        result.error = "Malformed CIP extended status.";
        return result;
    }

    for (int idx = 0; idx < ext_count; ++idx) {
        result.extended_status.push_back(ReadU16(cip_response, 4 + idx * 2));
    }
    result.data = cip_response.mid(4 + ext_bytes);
    result.success = (result.general_status == 0);
    if (!result.success) {
        result.error = QString("CIP error status: 0x%1").arg(result.general_status, 2, 16, QLatin1Char('0'));
    }
    return result;
}

}  // namespace motion_bench::protocol
