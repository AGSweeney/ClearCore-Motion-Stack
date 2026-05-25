#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"

err_t __attribute__((weak)) EthercatSlave_LwipUnknownEthProtocolHook(
    struct pbuf *packet, struct netif *netif) {
    (void)packet;
    (void)netif;
    return ERR_VAL;
}
