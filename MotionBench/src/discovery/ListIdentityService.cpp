#include "discovery/ListIdentityService.h"

namespace motion_bench::discovery {

ListIdentityService::ListIdentityService(QObject *parent) : QObject(parent) {}

QVector<protocol::ListIdentityResult> ListIdentityService::Discover(int timeout_ms) const {
    return protocol::EtherNetIpClient::ListIdentity(timeout_ms);
}

}  // namespace motion_bench::discovery
