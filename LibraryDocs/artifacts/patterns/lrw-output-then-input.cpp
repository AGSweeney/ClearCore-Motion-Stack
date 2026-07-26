// EXCERPT — source: ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/ethercat_slave.cpp
// EVIDENCE: E1 | symbol: LRW | lines: 717-757
            } else if (command == kEcCmdLrw) {
                if (data_length >= 2U) {
                    ApplyControlWordMirror(ReadLe16(data));
                }
                uint16_t sm2_start = 0U;
                uint16_t sm2_length = static_cast<uint16_t>(sizeof(EthercatPdoCommand));
                uint16_t sm3_start = 0U;
                uint16_t sm3_length = static_cast<uint16_t>(sizeof(EthercatPdoStatus));
                if (!ReadSmWindow(2U, &sm2_start, &sm2_length)) {
                    sm2_length = static_cast<uint16_t>(sizeof(EthercatPdoCommand));
                }
                if (!ReadSmWindow(3U, &sm3_start, &sm3_length)) {
                    sm3_length = static_cast<uint16_t>(sizeof(EthercatPdoStatus));
                }

                const uint16_t output_bytes = static_cast<uint16_t>(sm2_length);
                const uint16_t input_bytes = static_cast<uint16_t>(sm3_length);
                const uint16_t output_copy_len =
                    (data_length < output_bytes) ? data_length : output_bytes;
                if (output_copy_len > 0U) {
                    std::memcpy(&g_context.process_image[kPdoCommandOffset], data, output_copy_len);
                }
                SyncProcessImageToCommandImage();
                SyncStatusImageToProcessImage();

                if (data_length > output_bytes) {
                    const uint16_t remaining = static_cast<uint16_t>(data_length - output_bytes);
                    const uint16_t input_copy_len =
                        (remaining < input_bytes) ? remaining : input_bytes;
                    std::memcpy(data + output_bytes, &g_context.process_image[kPdoStatusOffset],
                                input_copy_len);
                    if (input_copy_len >= 2U) {
                        WriteLe16(data + output_bytes, g_context.status_image.status_word);
                    }
                }

                // LRW contributes read + write + slave selection.
                wkc = 3U;
            }
        } else if (IsAddressedToThisSlave(command, adp)) {
            if (IsReadCommand(command)) {
