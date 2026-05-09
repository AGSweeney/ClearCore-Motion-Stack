#pragma once

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include "discovery/ListIdentityService.h"
#include "protocol/EtherNetIpClient.h"

namespace motion_bench {
namespace boardmode {

class BoardModeToolService : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList discoveredIps READ discoveredIps NOTIFY discoveredIpsChanged)
    Q_PROPERTY(QString targetIp READ targetIp WRITE setTargetIp NOTIFY targetIpChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool boardModeMConnector READ boardModeMConnector NOTIFY boardModeMConnectorChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit BoardModeToolService(QObject *parent = nullptr);

    QStringList discoveredIps() const;
    QString targetIp() const;
    bool connected() const;
    bool boardModeMConnector() const;
    QString lastError() const;

    void setTargetIp(const QString &ip);

    Q_INVOKABLE void discover(int timeoutMs = 500);
    Q_INVOKABLE bool connectDevice();
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE bool refreshBoardMode();
    Q_INVOKABLE bool setBoardModeMConnector(bool enabled);

signals:
    void discoveredIpsChanged();
    void targetIpChanged();
    void connectedChanged();
    void boardModeMConnectorChanged();
    void lastErrorChanged();

private:
    QString LoadTargetIpSetting() const;
    void SaveTargetIpSetting(const QString &target_ip) const;
    void SetError(const QString &error);

    discovery::ListIdentityService discovery_service_;
    protocol::EtherNetIpClient eip_client_;
    QStringList discovered_ips_;
    QString target_ip_;
    bool board_mode_mconnector_ = false;
    QString last_error_;
};

}  // namespace boardmode
}  // namespace motion_bench
