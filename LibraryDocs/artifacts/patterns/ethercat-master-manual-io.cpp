// EXCERPT — source: EtherCATMaster/src/EthercatMasterThread.cpp
// EVIDENCE: E1 | symbol: manual LWR/LRD | lines: 385-415
        emit logMessage(QStringLiteral("Mapped PDO bytes missing or too small for CCIO. Using manual I/O via logical addresses 0x000 and 0x100."));
    }
    emit snapshotUpdated(snapshot);

    ctx->slavelist[0].state = EC_STATE_OPERATIONAL;
    if (useManualIo)
    {
        const std::uint8_t ioLevels = static_cast<std::uint8_t>(ioLevelMask_.load() & 0x3FU);
        const std::uint8_t ioDirs = static_cast<std::uint8_t>(ioDirectionMask_.load() & 0x3FU);
        const std::uint8_t aAnalogMask = static_cast<std::uint8_t>(aAnalogMask_.load() & 0x0FU);
        const bool ccioEnabled = ccioEnabled_.load();
        const std::uint64_t ccioOutputs = ccioOutputMask_.load();
        const std::uint64_t ccioDirections = ccioDirectionMask_.load();
        std::array<std::uint8_t, kCommandPdoLength> manualOutputs{};
        std::array<std::uint8_t, kStatusPdoLength> manualInputs{};
        encodeCommandPdo(manualOutputs.data(), kCommandPdoLength, 0U, ioLevels, ioDirs,
                         aAnalogMask, ccioEnabled, ccioOutputs, ccioDirections);

        const int outWkc = ecx_LWR(&ctx->port, kManualControlLogicalAddress, kCommandPdoLength,
                                   manualOutputs.data(), EC_TIMEOUTRET);
        const int inWkc = ecx_LRD(&ctx->port, kManualStatusLogicalAddress, kStatusPdoLength,
                                  manualInputs.data(), EC_TIMEOUTRET);
        snapshot.lastWkc = combineWkc(outWkc, inWkc);
        snapshot.controlWord = static_cast<quint16>(
            readU16At(manualOutputs.data(), kCommandPdoLength, 0U, kCommandControlWordByteOffset));
        snapshot.ioLevelRequest = static_cast<quint8>(ioLevels);
        snapshot.ioDirectionRequest = ioDirs;
        snapshot.ccioOutputRequest = ccioOutputs;
        snapshot.ccioDirectionRequest = ccioDirections;
        decodeStatusPdo(manualInputs.data(), kStatusPdoLength, 0U, &snapshot);
    }
