// EXCERPT — source: ProjectTemplate/LwIP/LwIP/port/ethercat_unknown_eth_hook.c
// EVIDENCE: E1 | symbol: EthercatSlave_LwipUnknownEthProtocolHook | lines: 1-20
#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"

err_t __attribute__((weak)) EthercatSlave_LwipUnknownEthProtocolHook(
    struct pbuf *packet, struct netif *netif) {
    (void)packet;
    (void)netif;
    return ERR_VAL;
}
