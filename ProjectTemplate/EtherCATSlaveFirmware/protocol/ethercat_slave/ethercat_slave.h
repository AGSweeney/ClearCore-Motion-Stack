#ifndef ETHERCAT_SLAVE_H_
#define ETHERCAT_SLAVE_H_

#include <stdint.h>

#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "netif/ethernet.h"
#include "protocol/ethercat_slave/ethercat_pdo_layout.h"

namespace ethercat_slave {

/**
 * @brief Runtime counters for minimal EtherCAT transport diagnostics.
 */
struct EthercatSlaveStats {
    uint32_t rx_ok;
    uint32_t rx_drop;
    uint32_t tx_ok;
    uint32_t tx_drop;
};

/**
 * @brief Initialize the EtherCAT raw-frame transport context.
 *
 * @param netif Active lwIP interface used for frame transmission.
 */
void EthercatSlaveInit(struct netif *netif);

/**
 * @brief Cyclic processing for the EtherCAT raw-frame transport.
 *
 * This transport-only stage intentionally excludes CiA-402 behavior.
 */
void EthercatSlaveCyclic();

/**
 * @brief Handle one received EtherType 0x88A4 frame.
 *
 * @param frame Received frame including Ethernet header.
 * @param netif Interface that received the frame.
 * @return ERR_OK when consumed; other lwIP error codes otherwise.
 */
err_t EthercatSlaveHandleFrame(struct pbuf *frame, struct netif *netif);

/**
 * @brief Send one fixed status image as raw EtherCAT payload.
 *
 * @param destination_mac Destination MAC for status egress.
 * @return ERR_OK when sent; other lwIP error codes otherwise.
 */
err_t EthercatSlaveSendStatusFrame(const struct eth_addr &destination_mac);

/**
 * @brief Access the latest consumed command image.
 */
const EthercatPdoCommand &CommandImage();

/**
 * @brief Access mutable transport status image for population.
 */
EthercatPdoStatus &StatusImage();

/**
 * @brief Access transport diagnostics counters.
 */
const EthercatSlaveStats &Stats();

}  // namespace ethercat_slave

extern "C" {
/**
 * @brief lwIP unknown-EtherType hook for EtherType 0x88A4 handling.
 */
err_t EthercatSlave_LwipUnknownEthProtocolHook(struct pbuf *packet,
                                               struct netif *netif);
}

#endif  // ETHERCAT_SLAVE_H_
