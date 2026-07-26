// EXCERPT — source: ProjectTemplate/ClearLinkCompatibilityFirmware/main.cpp
// EVIDENCE: E1 | symbol: main opener_init | lines: 19-75
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

