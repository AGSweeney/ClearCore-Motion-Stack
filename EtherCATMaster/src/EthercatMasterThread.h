#pragma once

#include "EthercatTypes.h"

#include <QMutex>
#include <QThread>

#include <atomic>
#include <cstdint>

class EthercatMasterThread final : public QThread
{
    Q_OBJECT

public:
    explicit EthercatMasterThread(QObject* parent = nullptr);

    void configure(const QString& adapterName, int cycleTimeMs);
    void requestStop();
    void setIoLevelMask(std::uint8_t levelMask);
    void setIoDirectionMask(std::uint8_t directionMask);
    void setAAnalogMask(std::uint8_t analogMask);
    void setCcioControl(bool enabled, std::uint64_t outputMask, std::uint64_t directionMask);

signals:
    void snapshotUpdated(const MasterSnapshot& snapshot);
    void logMessage(const QString& message);

protected:
    void run() override;

private:
    QString stateToText(std::uint16_t state) const;

    QMutex configMutex_;
    QString adapterName_;
    int cycleTimeMs_ = 10;
    bool configured_ = false;

    std::atomic_bool stopRequested_ = false;
    std::atomic_uint8_t ioLevelMask_ = 0;
    std::atomic_uint8_t ioDirectionMask_ = 0;
    std::atomic_uint8_t aAnalogMask_ = 0;
    std::atomic_bool ccioEnabled_ = false;
    std::atomic<std::uint64_t> ccioOutputMask_ = 0;
    std::atomic<std::uint64_t> ccioDirectionMask_ = 0;
};
