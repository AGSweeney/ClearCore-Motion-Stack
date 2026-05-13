/*
 * ClearLink Compatibility Firmware -- Main Entry Point
 *
 * Copyright (c) 2025 Adam G. Sweeney <agsweeney@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "ClearCore.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/ip4_addr.h"
#include "ports/ClearCore/opener.h"
#include "ciptcpipinterface.h"
#include <stdio.h>

/* Wrapper API stays available for shared motion/control integration. */
#include "ports/ClearCore/clearcore_wrapper.h"

int main(void) {
    ConnectorUsb.PortOpen();
    Delay_ms(100);

    ConnectorUsb.SendLine("\r\n========================================");
    ConnectorUsb.SendLine("  ClearLink Compatibility Firmware -- ClearCore");
    ConnectorUsb.SendLine("========================================\r\n");

    ConnectorUsb.SendLine("Waiting for Ethernet link...");
    ConnectorUsb.Flush();

    uint32_t linkWaitStart = Milliseconds();
    while (!EthernetMgr.PhyLinkActive()) {
        if (Milliseconds() - linkWaitStart > 5000) {
            ConnectorUsb.SendLine("ERROR: Ethernet link timeout!");
            while (true) {
                Delay_ms(1000);
            }
        }
        Delay_ms(100);
    }

    ConnectorUsb.SendLine("Ethernet link detected!\r\n");
    ConnectorUsb.Flush();

    EthernetMgr.Setup();
    Delay_ms(100);

    ConnectorUsb.SendLine("Initializing network (OpENer will apply stored TCP/IP startup config)...");

    Delay_ms(500);

    struct netif *netif = EthernetMgr.MacInterface();
    if (netif == nullptr) {
        ConnectorUsb.SendLine("ERROR: Failed to get netif pointer!");
        while (true) {
            Delay_ms(1000);
        }
    }

    ConnectorUsb.SendLine("\r\n--- Initializing OpENer ---\r\n");
    ConnectorUsb.Flush();

    opener_init(netif);

    Delay_ms(500);
    ConnectorUsb.Flush();

    int opener_status = opener_get_status();
    if (opener_status == 0) {
        ConnectorUsb.SendLine("OpENer init: SUCCESS (g_end_stack=0)");
    } else {
        char statusMsg[50];
        snprintf(statusMsg, sizeof(statusMsg), "OpENer init: FAILED (g_end_stack=%d)", opener_status);
        ConnectorUsb.SendLine(statusMsg);
    }

    if (netif->ip_addr.addr != 0) {
        char ipStr[32];
        snprintf(ipStr, sizeof(ipStr), "IP Address: %d.%d.%d.%d",
                 ip4_addr1(&netif->ip_addr),
                 ip4_addr2(&netif->ip_addr),
                 ip4_addr3(&netif->ip_addr),
                 ip4_addr4(&netif->ip_addr));
        ConnectorUsb.SendLine(ipStr);
    } else {
        ConnectorUsb.SendLine("WARNING: IP address not assigned yet");
    }
    ConnectorUsb.SendLine("\r\n--- Initialization complete -- entering main loop ---\r\n");
    ConnectorUsb.Flush();

    /* ---- Main loop ---- */
    uint32_t lastOpenerCall  = 0;
    uint32_t lastLedBlink    = 0;
    bool     ledState        = false;
    bool     prevLinkUp      = false;   /* track link transitions */

    while (true) {
        uint32_t currentTime = Milliseconds();

        /* Refresh the Ethernet stack */
        EthernetMgr.Refresh();

        /* OpENer cyclic processing (1 ms interval for snappier explicit messaging). */
        if (currentTime - lastOpenerCall >= 1) {
            opener_cyclic();
            lastOpenerCall = currentTime;
        }

        /* LED blink while interface config is pending */
        if ((g_tcpip.status & kTcpipStatusIfaceCfgPend) != 0) {
            if (currentTime - lastLedBlink >= 250) {
                ledState = !ledState;
                ConnectorLed.State(ledState);
                lastLedBlink = currentTime;
            }
        } else {
            if (ledState) {
                ConnectorLed.State(false);
                ledState = false;
            }
        }

        /* --- Network link transition logging (on change only) --- */
        {
            bool linkUp = EthernetMgr.PhyLinkActive();
            if (linkUp && !prevLinkUp) {
                /* Link just came up (or first detection) */
                char statusStr[80];
                if (netif->ip_addr.addr != 0) {
                    snprintf(statusStr, sizeof(statusStr),
                             "NET: Link UP  IP=%d.%d.%d.%d",
                             ip4_addr1(&netif->ip_addr),
                             ip4_addr2(&netif->ip_addr),
                             ip4_addr3(&netif->ip_addr),
                             ip4_addr4(&netif->ip_addr));
                } else {
                    snprintf(statusStr, sizeof(statusStr), "NET: Link UP  (no IP yet)");
                }
                ConnectorUsb.SendLine(statusStr);
            } else if (!linkUp && prevLinkUp) {
                /* Link just went down */
                ConnectorUsb.SendLine("NET: Link DOWN");
            }
            prevLinkUp = linkUp;
        }

        /* Minimal delay -- EthernetMgr.Refresh() and opener_cyclic()
         * need frequent calls for responsive EIP communication. */
        Delay_ms(1);
    }
}