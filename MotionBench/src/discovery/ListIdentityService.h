#pragma once

#include <QtCore/QObject>
#include <QtCore/QVector>

#include "protocol/EtherNetIpClient.h"

namespace motion_bench::discovery {

class ListIdentityService : public QObject {
    Q_OBJECT

public:
    explicit ListIdentityService(QObject *parent = nullptr);

    QVector<protocol::ListIdentityResult> Discover(int timeout_ms = 500) const;
};

}  // namespace motion_bench::discovery
