#pragma once

#include <QMetaType>
#include <QString>
#include <QtGlobal>

struct MasterSnapshot
{
    bool connected = false;
    bool operational = false;
    int slaveCount = 0;
    int expectedWkc = 0;
    int lastWkc = 0;
    quint64 cycleCount = 0;
    quint16 controlWord = 0;
    quint16 statusWord = 0;
    quint8 ioLevelRequest = 0;
    quint8 ioDirectionRequest = 0;
    quint8 ioState = 0;
    quint8 ioDirectionApplied = 0;
    quint8 faultCode = 0;
    quint8 aAnalogMaskRequest = 0;
    quint8 aAnalogMaskApplied = 0;
    quint8 diBits = 0;
    quint8 aDigitalBits = 0;
    quint16 a9Raw = 0;
    quint16 a10Raw = 0;
    quint16 a11Raw = 0;
    quint16 a12Raw = 0;
    quint16 supplyCentivolts = 0;
    bool ccioEnabled = false;
    bool ccioLinkBroken = false;
    bool ccioAnyOverload = false;
    quint8 ccioBoardCount = 0;
    quint64 ccioOutputRequest = 0;
    quint64 ccioDirectionRequest = 0;
    quint64 ccioInputBits = 0;
    quint64 ccioStatusBits = 0;
    quint64 ccioDirectionApplied = 0;
    quint32 sequenceAck = 0;
    double loopTimeMs = 0.0;
    double cyclePeriodMs = 0.0;
    double loopJitterMs = 0.0;
    double maxLoopJitterMs = 0.0;
    double frameLatencyMs = 0.0;
    double maxFrameLatencyMs = 0.0;
    double updateConsistencyMs = 0.0;
    double maxUpdateConsistencyMs = 0.0;
    quint32 firmwareLoopPeriodUs = 0;
    quint32 firmwareLoopRuntimeUs = 0;
    quint32 firmwareLoopJitterUs = 0;
    quint32 firmwareTransportUs = 0;
    QString isrTimingText = QStringLiteral("0 us");
    QString stateText = QStringLiteral("IDLE");
    QString lastError;
};

Q_DECLARE_METATYPE(MasterSnapshot)
