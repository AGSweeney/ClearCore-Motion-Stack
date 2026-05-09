#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include <cstdint>

#include "protocol/CipPath.h"

namespace motion_bench::protocol {

struct CipResult {
    bool success = false;
    QString error;
    std::uint8_t general_status = 0;
    QVector<std::uint16_t> extended_status;
    QByteArray data;
};

struct ListIdentityResult {
    QString product_name;
    QString ip_address;
    std::uint16_t vendor_id = 0;
    std::uint16_t product_code = 0;
    std::uint16_t device_type = 0;
    std::uint16_t revision = 0;
    std::uint32_t serial_number = 0;
    std::uint16_t encapsulation_version = 0;
    std::uint16_t socket_port = 0;
};

class EtherNetIpClient : public QObject {
    Q_OBJECT

public:
    explicit EtherNetIpClient(QObject *parent = nullptr);

    bool Connect(const QString &ip_address, std::uint16_t port = 44818, int timeout_ms = 3000);
    bool Disconnect(int timeout_ms = 2000);
    bool IsConnected() const;

    CipResult GetAttributeSingle(const CipPath &path, int timeout_ms = 1000);
    CipResult SetAttributeSingle(const CipPath &path, const QByteArray &payload, int timeout_ms = 1000);

    static QVector<ListIdentityResult> ListIdentity(int timeout_ms = 500);

private:
    struct EncapsulationPacket {
        std::uint16_t command = 0;
        std::uint16_t length = 0;
        std::uint32_t session_handle = 0;
        std::uint32_t status = 0;
        std::uint64_t sender_context = 0;
        std::uint32_t options = 0;
        QByteArray payload;
    };

    EncapsulationPacket BuildPacket(std::uint16_t command, const QByteArray &payload) const;
    QByteArray SerializePacket(const EncapsulationPacket &packet) const;
    bool ReadPacket(EncapsulationPacket *packet, int timeout_ms);
    bool WritePacket(const EncapsulationPacket &packet, int timeout_ms);
    bool RegisterSession(int timeout_ms);
    bool UnregisterSession();

    CipResult SendExplicitMessage(std::uint8_t service, const CipPath &path, const QByteArray &data, int timeout_ms);
    QByteArray BuildSendRRData(const QByteArray &cip_payload) const;
    bool ParseSendRRDataResponse(const QByteArray &payload, QByteArray *cip_response, QString *error) const;
    CipResult ParseCipResponse(const QByteArray &cip_response) const;

    QTcpSocket socket_;
    std::uint32_t session_handle_ = 0;
    std::uint64_t sender_context_ = 0;
};

}  // namespace motion_bench::protocol
