#ifndef ETHERCAT_SLAVE_LWIP_HOOKS_H_
#define ETHERCAT_SLAVE_LWIP_HOOKS_H_

#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Personality-level handler entry for unknown Ethernet protocols.
 */
err_t EthercatSlave_LwipUnknownEthProtocolHook(struct pbuf *packet,
                                               struct netif *netif);

#ifdef __cplusplus
}
#endif

#define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(p, netif) \
    EthercatSlave_LwipUnknownEthProtocolHook((p), (netif))

#endif  // ETHERCAT_SLAVE_LWIP_HOOKS_H_
