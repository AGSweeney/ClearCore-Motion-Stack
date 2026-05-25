#pragma once

#include "EthercatMasterThread.h"

#include <QMainWindow>

#include <array>
#include <cstdint>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTextEdit;
class QWidget;
class ClearCoreBoardWidget;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onStartStopClicked();
    void onRefreshAdaptersClicked();
    void onIoControlsChanged();
    void onAChannelModeChanged();
    void onCcioControlsChanged();
    void onSnapshotUpdated(const MasterSnapshot& snapshot);
    void appendLog(const QString& line);

private:
    std::uint8_t currentIoLevelMask() const;
    std::uint8_t currentIoDirectionMask() const;
    std::uint8_t currentAAnalogMask() const;
    std::uint64_t currentCcioOutputMask() const;
    std::uint64_t currentCcioDirectionMask() const;
    void applySnapshotReadback(const MasterSnapshot& snapshot);
    void updateCcioRowAvailability(int enabledPinCount);
    void refreshAdapters();
    void setRunningUi(bool running);

    EthercatMasterThread worker_;

    QComboBox* adapterCombo_ = nullptr;
    QPushButton* refreshAdaptersButton_ = nullptr;
    QSpinBox* cycleSpin_ = nullptr;
    std::array<QCheckBox*, 6> ioOutputModeChecks_{};
    std::array<QCheckBox*, 6> ioOutputDriveChecks_{};
    std::array<QLabel*, 6> ioStateValues_{};
    std::array<QLabel*, 6> ioDirectionValues_{};
    std::array<QLabel*, 3> diValues_{};
    std::array<QCheckBox*, 4> aAnalogModeChecks_{};
    std::array<QLabel*, 4> aModeAppliedValues_{};
    std::array<QLabel*, 4> aDigitalValues_{};
    std::array<QLabel*, 4> aAnalogRawValues_{};
    QCheckBox* ccioEnableCheck_ = nullptr;
    std::array<QLabel*, 64> ccioPinLabels_{};
    std::array<QCheckBox*, 64> ccioOutputModeChecks_{};
    std::array<QCheckBox*, 64> ccioOutputDriveChecks_{};
    std::array<QLabel*, 64> ccioStateValues_{};
    std::array<QLabel*, 64> ccioDirectionValues_{};
    std::array<QLabel*, 64> ccioFaultValues_{};
    std::array<QLabel*, 64> ccioLivePinLabels_{};
    std::array<QLabel*, 64> ccioLiveStateValues_{};
    std::array<QLabel*, 64> ccioLiveDirectionValues_{};
    std::array<QLabel*, 64> ccioLiveFaultValues_{};
    ClearCoreBoardWidget* boardWidget_ = nullptr;
    QPushButton* startStopButton_ = nullptr;

    QLabel* stateValue_ = nullptr;
    QLabel* slavesValue_ = nullptr;
    QLabel* wkcValue_ = nullptr;
    QLabel* cycleValue_ = nullptr;
    QLabel* controlWordValue_ = nullptr;
    QLabel* statusWordValue_ = nullptr;
    QLabel* supplyVoltageValue_ = nullptr;
    QLabel* ccioSummaryValue_ = nullptr;
    QLabel* ccioLiveSummaryValue_ = nullptr;
    QLabel* sequenceAckValue_ = nullptr;
    QLabel* errorValue_ = nullptr;
    QLabel* timingCyclePeriodValue_ = nullptr;
    QLabel* timingLoopRuntimeValue_ = nullptr;
    QLabel* timingLoopJitterValue_ = nullptr;
    QLabel* timingMaxLoopJitterValue_ = nullptr;
    QLabel* timingFrameLatencyValue_ = nullptr;
    QLabel* timingMaxFrameLatencyValue_ = nullptr;
    QLabel* timingUpdateConsistencyValue_ = nullptr;
    QLabel* timingMaxUpdateConsistencyValue_ = nullptr;
    QLabel* timingFirmwareLoopPeriodValue_ = nullptr;
    QLabel* timingFirmwareLoopRuntimeValue_ = nullptr;
    QLabel* timingFirmwareLoopJitterValue_ = nullptr;
    QLabel* timingFirmwareTransportValue_ = nullptr;
    QLabel* timingIsrValue_ = nullptr;

    QStackedWidget* contentStack_ = nullptr;
    QWidget* boardPage_ = nullptr;
    QWidget* ccioPage_ = nullptr;
    QWidget* statusPage_ = nullptr;
    QWidget* timingPage_ = nullptr;
    int ccioVisiblePinCount_ = -1;
    int ccioLiveVisiblePinCount_ = -1;
    QTextEdit* logView_ = nullptr;
};
