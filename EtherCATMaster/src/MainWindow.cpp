#include "MainWindow.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QButtonGroup>
#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QFrame>
#include <QFontDatabase>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QSvgRenderer>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

extern "C"
{
#include <soem/soem.h>
}

class ClearCoreBoardWidget final : public QWidget
{
public:
    explicit ClearCoreBoardWidget(QWidget* parent = nullptr)
        : QWidget(parent)
        , boardRenderer_(QStringLiteral(":/images/clearcore_board.svg"))
    {
        boardRenderer_.setAspectRatioMode(Qt::KeepAspectRatio);
        setMinimumSize(760, 480);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setSnapshot(const MasterSnapshot& snapshot)
    {
        ioState_ = snapshot.ioState;
        ioDirection_ = snapshot.ioDirectionApplied;
        diBits_ = snapshot.diBits;
        aDigitalBits_ = snapshot.aDigitalBits;
        aAnalogMask_ = snapshot.aAnalogMaskApplied;
        aRaw_ = {snapshot.a9Raw, snapshot.a10Raw, snapshot.a11Raw, snapshot.a12Raw};
        supplyCentivolts_ = snapshot.supplyCentivolts;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QWidget::paintEvent(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor(20, 22, 25));

        if (!boardRenderer_.isValid())
        {
            painter.setPen(QColor(220, 220, 220));
            painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("ClearCore board SVG not available"));
            return;
        }

        const QRectF sourceRect = boardRenderer_.viewBoxF().isEmpty()
                                      ? QRectF(0.0, 0.0, 1200.0, 760.0)
                                      : boardRenderer_.viewBoxF();
        const QRectF availableRect = QRectF(rect());
        const qreal scale = std::min(availableRect.width() / sourceRect.width(),
                                     availableRect.height() / sourceRect.height());
        const QSizeF scaledSize(sourceRect.width() * scale, sourceRect.height() * scale);
        const QRectF imageRect(availableRect.left() + ((availableRect.width() - scaledSize.width()) / 2.0),
                               availableRect.top() + ((availableRect.height() - scaledSize.height()) / 2.0),
                               scaledSize.width(),
                               scaledSize.height());
        if (imageRect.isEmpty())
        {
            return;
        }
        boardRenderer_.render(&painter, imageRect);

        painter.save();
        painter.translate(imageRect.left(), imageRect.top());
        painter.scale(scale, scale);
        painter.translate(-sourceRect.left(), -sourceRect.top());

        for (int i = 0; i < 6; ++i)
        {
            const bool outputMode = ((ioDirection_ >> i) & 0x01U) != 0U;
            const bool on = ((ioState_ >> i) & 0x01U) != 0U;
            const double x = 198.0 + (static_cast<double>(i) * 126.0);
            drawStatusPill(painter, QPointF(x, 668.0), QStringLiteral("IO%1").arg(i), on,
                           outputMode ? QStringLiteral("OUT") : QStringLiteral("IN"),
                           1.0);
        }

        for (int i = 0; i < 3; ++i)
        {
            const bool on = ((diBits_ >> i) & 0x01U) != 0U;
            const double x = 954.0 - (static_cast<double>(i) * 126.0);
            drawStatusPill(painter, QPointF(x, 68.0), QStringLiteral("DI%1").arg(6 + i), on,
                           QStringLiteral("IN"), 1.0);
        }

        for (int i = 0; i < 4; ++i)
        {
            const bool analogMode = ((aAnalogMask_ >> i) & 0x01U) != 0U;
            const bool digitalOn = ((aDigitalBits_ >> i) & 0x01U) != 0U;
            const double x = 576.0 - (static_cast<double>(i) * 126.0);
            const QString detail = analogMode
                                       ? QStringLiteral("A %1").arg(aRaw_[static_cast<std::size_t>(i)])
                                       : QStringLiteral("D %1").arg(digitalOn ? QStringLiteral("ON") : QStringLiteral("OFF"));
            drawStatusPill(painter, QPointF(x, 68.0), QStringLiteral("A%1").arg(9 + i),
                           analogMode || digitalOn, detail, 1.0);
        }

        const QString supplyDetail = QStringLiteral("%1 V").arg(
            static_cast<double>(supplyCentivolts_) / 100.0, 0, 'f', 2);
        drawStatusPill(painter, QPointF(954.0, 668.0), QStringLiteral("POWER"),
                       supplyCentivolts_ > 0U, supplyDetail, 1.0);
        painter.restore();
    }

private:
    void drawStatusPill(QPainter& painter, const QPointF& center, const QString& title,
                        const bool active, const QString& detail, const double scale)
    {
        const QColor fill = active ? QColor(24, 124, 64, 230) : QColor(50, 55, 62, 220);
        const QColor stroke = active ? QColor(139, 255, 77) : QColor(130, 138, 148);
        const double clampedScale = std::clamp(scale, 0.72, 1.65);
        const double pillWidth = 80.0 * clampedScale;
        const double pillHeight = 34.0 * clampedScale;
        const QRectF pill(center.x() - (pillWidth / 2.0), center.y() - (pillHeight / 2.0),
                          pillWidth, pillHeight);

        painter.setPen(QPen(stroke, 2.0 * clampedScale));
        painter.setBrush(fill);
        painter.drawRoundedRect(pill, 9.0 * clampedScale, 9.0 * clampedScale);

        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSizeF(9.0 * clampedScale);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(pill.adjusted(2.0 * clampedScale, 1.0 * clampedScale,
                                       -2.0 * clampedScale, -17.0 * clampedScale),
                         Qt::AlignCenter, title);

        font.setPointSizeF(7.5 * clampedScale);
        font.setBold(false);
        painter.setFont(font);
        painter.drawText(pill.adjusted(2.0 * clampedScale, 15.0 * clampedScale,
                                       -2.0 * clampedScale, -1.0 * clampedScale),
                         Qt::AlignCenter, detail);
    }

    QSvgRenderer boardRenderer_;
    quint8 ioState_ = 0U;
    quint8 ioDirection_ = 0U;
    quint8 diBits_ = 0U;
    quint8 aDigitalBits_ = 0U;
    quint8 aAnalogMask_ = 0U;
    quint16 supplyCentivolts_ = 0U;
    std::array<quint16, 4> aRaw_{};
};

namespace
{
constexpr auto kSettingsOrg = "EtherCATMaster";
constexpr auto kSettingsApp = "EtherCATMasterQt";
constexpr auto kLastAdapterKey = "ui/lastAdapter";

QLabel* createValueLabel(QWidget* parent, const QString& text = QStringLiteral("-"))
{
    auto* label = new QLabel(text, parent);
    label->setMinimumWidth(110);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QLabel* createMonospaceValueLabel(QWidget* parent, const QString& text)
{
    auto* label = createValueLabel(parent, text);
    auto monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monoFont.setBold(true);
    label->setFont(monoFont);
    return label;
}

void setBadgeStyle(QLabel* label, const bool active, const QString& activeText, const QString& inactiveText)
{
    if (label == nullptr)
    {
        return;
    }

    const QString text = active ? activeText : inactiveText;
    const QString style = active
                              ? QStringLiteral("QLabel { color: #0B7D30; font-weight: 700; }")
                              : QStringLiteral("QLabel { color: #676767; font-weight: 600; }");
    if (label->text() != text)
    {
        label->setText(text);
    }
    if (label->styleSheet() != style)
    {
        label->setStyleSheet(style);
    }
}

QString selectedAdapterName(const QComboBox* combo)
{
    QString adapterName = combo->currentData().toString().trimmed();
    if (adapterName.isEmpty())
    {
        adapterName = combo->currentText().trimmed();
    }
    return adapterName;
}

void saveLastAdapterSelection(const QComboBox* combo)
{
    const QString adapterName = selectedAdapterName(combo);
    if (adapterName.isEmpty())
    {
        return;
    }

    QSettings settings(QString::fromLatin1(kSettingsOrg), QString::fromLatin1(kSettingsApp));
    settings.setValue(QString::fromLatin1(kLastAdapterKey), adapterName);
}

void restoreLastAdapterSelection(QComboBox* combo)
{
    QSettings settings(QString::fromLatin1(kSettingsOrg), QString::fromLatin1(kSettingsApp));
    const QString adapterName = settings.value(QString::fromLatin1(kLastAdapterKey)).toString().trimmed();
    if (adapterName.isEmpty())
    {
        return;
    }

    const int existingIndex = combo->findData(adapterName);
    if (existingIndex >= 0)
    {
        combo->setCurrentIndex(existingIndex);
    }
    else
    {
        combo->setEditText(adapterName);
    }
}

QString ccioPinName(const int bit)
{
    const QChar board = QChar(static_cast<ushort>('A' + (bit / 8)));
    return QStringLiteral("CCIO%1%2").arg(board).arg(bit % 8);
}
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("EtherCAT Master for ClearCore ECSlave"));
    resize(1180, 760);
    setStyleSheet(QStringLiteral(
        "QMainWindow, QWidget { background: #202225; color: #E6E9EF; }"
        "QGroupBox { background: #282B2F; border: 1px solid #3A3F46; border-radius: 6px; font-weight: 600; margin-top: 12px; padding-top: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 6px; color: #D7DEE8; }"
        "QLabel { min-height: 20px; }"
        "QLineEdit, QComboBox, QSpinBox { background: #151719; border: 1px solid #444A52; border-radius: 4px; color: #F2F5F8; min-height: 24px; padding: 2px 6px; }"
        "QPushButton { background: #2E6F9A; border: 1px solid #3E8BBD; border-radius: 4px; color: white; font-weight: 600; min-height: 30px; padding: 4px 12px; }"
        "QPushButton:hover { background: #357EAE; }"
        "QPushButton:disabled { background: #3A3D42; color: #898F98; border-color: #454A51; }"
        "QPushButton[navButton=\"true\"] { background: transparent; border: none; border-radius: 4px; color: #CAD3DF; text-align: left; padding: 8px 10px; }"
        "QPushButton[navButton=\"true\"]:hover { background: #30343A; }"
        "QPushButton[navButton=\"true\"]:checked { background: #2E6F9A; color: white; }"
        "QCheckBox { spacing: 6px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid #7C8794; border-radius: 3px; background: #101214; }"
        "QCheckBox::indicator:hover { border: 1px solid #57E5FF; }"
        "QCheckBox::indicator:checked { background: #2E9BFF; border: 1px solid #8DDCFF; image: none; }"
        "QCheckBox::indicator:checked:disabled { background: #496C83; border: 1px solid #708795; }"
        "QCheckBox::indicator:unchecked:disabled { background: #24272B; border: 1px solid #4A5058; }"
        "QScrollArea { border: none; background: #202225; }"
        "QTextEdit { background: #111315; border: 1px solid #3A3F46; border-radius: 4px; color: #D9E2EC; }"));

    auto* central = new QWidget(this);
    auto* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(10);

    auto* sidebar = new QFrame(central);
    sidebar->setFixedWidth(190);
    sidebar->setStyleSheet(QStringLiteral("QFrame { background: #17191C; border: 1px solid #30343A; border-radius: 6px; }"));
    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(10, 12, 10, 10);
    sidebarLayout->setSpacing(8);

    auto* title = new QLabel(QStringLiteral("ClearCore\nEtherCAT"), sidebar);
    QFont titleFont = title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet(QStringLiteral("QLabel { color: #F2F5F8; padding-bottom: 8px; }"));
    sidebarLayout->addWidget(title);

    auto* navGroup = new QButtonGroup(sidebar);
    navGroup->setExclusive(true);
    auto* navConnection = new QPushButton(QStringLiteral("Connection + Control"), sidebar);
    auto* navBoard = new QPushButton(QStringLiteral("Board Overview"), sidebar);
    auto* navCcio = new QPushButton(QStringLiteral("CCIO"), sidebar);
    auto* navStatus = new QPushButton(QStringLiteral("Live Status"), sidebar);
    auto* navTiming = new QPushButton(QStringLiteral("Sync / Pulse Timing"), sidebar);
    auto* navLog = new QPushButton(QStringLiteral("Runtime Log"), sidebar);
    const std::array<QPushButton*, 6> navButtons = {navConnection, navBoard, navCcio, navStatus, navTiming, navLog};
    for (int i = 0; i < static_cast<int>(navButtons.size()); ++i)
    {
        navButtons[static_cast<std::size_t>(i)]->setCheckable(true);
        navButtons[static_cast<std::size_t>(i)]->setProperty("navButton", true);
        navGroup->addButton(navButtons[static_cast<std::size_t>(i)], i);
        sidebarLayout->addWidget(navButtons[static_cast<std::size_t>(i)]);
    }

    sidebarLayout->addStretch(1);

    startStopButton_ = new QPushButton(QStringLiteral("Start"), sidebar);
    startStopButton_->setToolTip(QStringLiteral("Start or stop EtherCAT cyclic exchange."));
    sidebarLayout->addWidget(startStopButton_);

    contentStack_ = new QStackedWidget(central);
    contentStack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(contentStack_, 1);

    auto* configGroup = new QGroupBox(QStringLiteral("Master Configuration"), central);
    auto* configLayout = new QFormLayout(configGroup);
    configLayout->setLabelAlignment(Qt::AlignLeft);

    auto* adapterRow = new QWidget(configGroup);
    auto* adapterRowLayout = new QHBoxLayout(adapterRow);
    adapterRowLayout->setContentsMargins(0, 0, 0, 0);

    adapterCombo_ = new QComboBox(adapterRow);
    adapterCombo_->setEditable(true);
    adapterCombo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    adapterCombo_->setToolTip(QStringLiteral("Choose discovered adapter or type one manually."));
    if (adapterCombo_->lineEdit() != nullptr)
    {
        adapterCombo_->lineEdit()->setPlaceholderText(QStringLiteral("\\Device\\NPF_{YOUR-ADAPTER-GUID}"));
    }

    refreshAdaptersButton_ = new QPushButton(QStringLiteral("Refresh"), adapterRow);
    adapterRowLayout->addWidget(adapterCombo_, 1);
    adapterRowLayout->addWidget(refreshAdaptersButton_);
    configLayout->addRow(QStringLiteral("Npcap Adapter"), adapterRow);

    cycleSpin_ = new QSpinBox(configGroup);
    cycleSpin_->setRange(1, 1000);
    cycleSpin_->setValue(10);
    cycleSpin_->setSuffix(QStringLiteral(" ms"));
    cycleSpin_->setToolTip(QStringLiteral("Master cycle period in milliseconds."));
    configLayout->addRow(QStringLiteral("Cycle Time"), cycleSpin_);

    auto* ioControlContainer = new QWidget(configGroup);
    auto* ioControlLayout = new QGridLayout(ioControlContainer);
    ioControlLayout->setContentsMargins(0, 0, 0, 0);
    ioControlLayout->setHorizontalSpacing(10);
    ioControlLayout->setVerticalSpacing(4);
    ioControlLayout->addWidget(new QLabel(QStringLiteral("Channel"), ioControlContainer), 0, 0);
    ioControlLayout->addWidget(new QLabel(QStringLiteral("Output Mode"), ioControlContainer), 0, 1);
    ioControlLayout->addWidget(new QLabel(QStringLiteral("Drive"), ioControlContainer), 0, 2);
    for (int bit = 0; bit < 6; ++bit)
    {
        ioControlLayout->addWidget(new QLabel(QStringLiteral("IO%1").arg(bit), ioControlContainer), bit + 1, 0);

        auto* modeCheck = new QCheckBox(QStringLiteral("OUT"), ioControlContainer);
        auto* driveCheck = new QCheckBox(QStringLiteral("HIGH"), ioControlContainer);
        modeCheck->setToolTip(QStringLiteral("Set this channel as digital output."));
        driveCheck->setToolTip(QStringLiteral("Output level request when OUT is enabled."));
        ioOutputModeChecks_[bit] = modeCheck;
        ioOutputDriveChecks_[bit] = driveCheck;

        ioControlLayout->addWidget(modeCheck, bit + 1, 1);
        ioControlLayout->addWidget(driveCheck, bit + 1, 2);

        connect(modeCheck, &QCheckBox::toggled, this, &MainWindow::onIoControlsChanged);
        connect(driveCheck, &QCheckBox::toggled, this, &MainWindow::onIoControlsChanged);
    }
    configLayout->addRow(QStringLiteral("IO0..IO5"), ioControlContainer);

    auto* aModeContainer = new QWidget(configGroup);
    auto* aModeLayout = new QHBoxLayout(aModeContainer);
    aModeLayout->setContentsMargins(0, 0, 0, 0);
    aModeLayout->setSpacing(14);
    for (int i = 0; i < 4; ++i)
    {
        auto* modeCheck = new QCheckBox(QStringLiteral("A%1 analog").arg(9 + i), aModeContainer);
        aAnalogModeChecks_[i] = modeCheck;
        aModeLayout->addWidget(modeCheck);
        connect(modeCheck, &QCheckBox::toggled, this, &MainWindow::onAChannelModeChanged);
    }
    aModeLayout->addStretch(1);
    configLayout->addRow(QStringLiteral("A9..A12 Mode"), aModeContainer);

    auto* controlHint = new QLabel(QStringLiteral("Use the Start/Stop button in the left navigation bar."), configGroup);
    controlHint->setStyleSheet(QStringLiteral("QLabel { color: #AEB7C2; }"));
    configLayout->addRow(QStringLiteral("Control"), controlHint);

    auto* boardGroup = new QGroupBox(QStringLiteral("ClearCore I/O Overview"), central);
    auto* boardLayout = new QVBoxLayout(boardGroup);
    boardWidget_ = new ClearCoreBoardWidget(boardGroup);
    boardLayout->addWidget(boardWidget_);

    auto* ccioGroup = new QGroupBox(QStringLiteral("CCIO-8 Expansion I/O"), central);
    auto* ccioLayout = new QVBoxLayout(ccioGroup);
    ccioEnableCheck_ = new QCheckBox(QStringLiteral("Enable CCIO on COM-1"), ccioGroup);
    ccioEnableCheck_->setToolTip(QStringLiteral("Open COM-1 in CCIO mode and discover attached CCIO-8 boards."));
    ccioLayout->addWidget(ccioEnableCheck_);

    ccioSummaryValue_ = createValueLabel(ccioGroup, QStringLiteral("Disabled"));
    ccioLayout->addWidget(ccioSummaryValue_);

    auto* ccioScroll = new QScrollArea(ccioGroup);
    ccioScroll->setWidgetResizable(true);
    auto* ccioBody = new QWidget(ccioScroll);
    auto* ccioGrid = new QGridLayout(ccioBody);
    ccioGrid->setContentsMargins(0, 0, 0, 0);
    ccioGrid->setHorizontalSpacing(12);
    ccioGrid->setVerticalSpacing(4);
    ccioGrid->setAlignment(Qt::AlignTop);
    ccioGrid->addWidget(new QLabel(QStringLiteral("Pin"), ccioBody), 0, 0);
    ccioGrid->addWidget(new QLabel(QStringLiteral("Output"), ccioBody), 0, 1);
    ccioGrid->addWidget(new QLabel(QStringLiteral("Drive"), ccioBody), 0, 2);
    ccioGrid->addWidget(new QLabel(QStringLiteral("State"), ccioBody), 0, 3);
    ccioGrid->addWidget(new QLabel(QStringLiteral("Applied"), ccioBody), 0, 4);
    ccioGrid->addWidget(new QLabel(QStringLiteral("Fault"), ccioBody), 0, 5);
    for (int bit = 0; bit < 64; ++bit)
    {
        const int row = bit + 1;
        auto* pinLabel = new QLabel(ccioPinName(bit), ccioBody);
        ccioPinLabels_[static_cast<std::size_t>(bit)] = pinLabel;
        ccioGrid->addWidget(pinLabel, row, 0);

        auto* modeCheck = new QCheckBox(QStringLiteral("OUT"), ccioBody);
        auto* driveCheck = new QCheckBox(QStringLiteral("HIGH"), ccioBody);
        auto* stateLabel = createValueLabel(ccioBody, QStringLiteral("OFF"));
        auto* dirLabel = createValueLabel(ccioBody, QStringLiteral("IN"));
        auto* faultLabel = createValueLabel(ccioBody, QStringLiteral("-"));
        ccioOutputModeChecks_[static_cast<std::size_t>(bit)] = modeCheck;
        ccioOutputDriveChecks_[static_cast<std::size_t>(bit)] = driveCheck;
        ccioStateValues_[static_cast<std::size_t>(bit)] = stateLabel;
        ccioDirectionValues_[static_cast<std::size_t>(bit)] = dirLabel;
        ccioFaultValues_[static_cast<std::size_t>(bit)] = faultLabel;

        ccioGrid->addWidget(modeCheck, row, 1);
        ccioGrid->addWidget(driveCheck, row, 2);
        ccioGrid->addWidget(stateLabel, row, 3);
        ccioGrid->addWidget(dirLabel, row, 4);
        ccioGrid->addWidget(faultLabel, row, 5);

        connect(modeCheck, &QCheckBox::toggled, this, &MainWindow::onCcioControlsChanged);
        connect(driveCheck, &QCheckBox::toggled, this, &MainWindow::onCcioControlsChanged);
    }
    connect(ccioEnableCheck_, &QCheckBox::toggled, this, &MainWindow::onCcioControlsChanged);
    updateCcioRowAvailability(0);
    ccioScroll->setWidget(ccioBody);
    ccioLayout->addWidget(ccioScroll, 1);

    auto* statusGroup = new QGroupBox(QStringLiteral("Live Status"), central);
    auto* statusLayout = new QFormLayout(statusGroup);

    stateValue_ = createValueLabel(statusGroup, QStringLiteral("IDLE"));
    slavesValue_ = createValueLabel(statusGroup, QStringLiteral("0"));
    wkcValue_ = createValueLabel(statusGroup, QStringLiteral("0 / 0"));
    cycleValue_ = createValueLabel(statusGroup, QStringLiteral("0.0 ms"));
    controlWordValue_ = createMonospaceValueLabel(statusGroup, QStringLiteral("0x0000"));
    statusWordValue_ = createMonospaceValueLabel(statusGroup, QStringLiteral("0x0000"));
    supplyVoltageValue_ = createMonospaceValueLabel(statusGroup, QStringLiteral("0.00 V"));
    ccioLiveSummaryValue_ = createValueLabel(statusGroup, QStringLiteral("Disabled, 0 board(s)"));
    sequenceAckValue_ = createMonospaceValueLabel(statusGroup, QStringLiteral("0x00000000"));
    errorValue_ = createValueLabel(statusGroup, QStringLiteral("-"));
    errorValue_->setWordWrap(true);
    errorValue_->setStyleSheet(QStringLiteral("QLabel { color: #8B1D1D; }"));

    auto* ioFeedbackContainer = new QWidget(statusGroup);
    auto* ioFeedbackLayout = new QGridLayout(ioFeedbackContainer);
    ioFeedbackLayout->setContentsMargins(0, 0, 0, 0);
    ioFeedbackLayout->setHorizontalSpacing(12);
    ioFeedbackLayout->setVerticalSpacing(4);
    ioFeedbackLayout->addWidget(new QLabel(QStringLiteral("Channel"), ioFeedbackContainer), 0, 0);
    ioFeedbackLayout->addWidget(new QLabel(QStringLiteral("State"), ioFeedbackContainer), 0, 1);
    ioFeedbackLayout->addWidget(new QLabel(QStringLiteral("Direction"), ioFeedbackContainer), 0, 2);
    for (int i = 0; i < 6; ++i)
    {
        ioFeedbackLayout->addWidget(new QLabel(QStringLiteral("IO%1").arg(i), ioFeedbackContainer), i + 1, 0);
        auto* stateLabel = createValueLabel(ioFeedbackContainer, QStringLiteral("OFF"));
        auto* dirLabel = createValueLabel(ioFeedbackContainer, QStringLiteral("IN"));
        ioStateValues_[i] = stateLabel;
        ioDirectionValues_[i] = dirLabel;
        ioFeedbackLayout->addWidget(stateLabel, i + 1, 1);
        ioFeedbackLayout->addWidget(dirLabel, i + 1, 2);
    }

    auto* diContainer = new QWidget(statusGroup);
    auto* diLayout = new QHBoxLayout(diContainer);
    diLayout->setContentsMargins(0, 0, 0, 0);
    diLayout->setSpacing(14);
    for (int i = 0; i < 3; ++i)
    {
        auto* label = createValueLabel(diContainer, QStringLiteral("DI%1: OFF").arg(6 + i));
        diValues_[i] = label;
        diLayout->addWidget(label);
    }
    diLayout->addStretch(1);

    auto* aFeedbackContainer = new QWidget(statusGroup);
    auto* aFeedbackLayout = new QGridLayout(aFeedbackContainer);
    aFeedbackLayout->setContentsMargins(0, 0, 0, 0);
    aFeedbackLayout->setHorizontalSpacing(12);
    aFeedbackLayout->setVerticalSpacing(4);
    aFeedbackLayout->addWidget(new QLabel(QStringLiteral("Channel"), aFeedbackContainer), 0, 0);
    aFeedbackLayout->addWidget(new QLabel(QStringLiteral("Mode"), aFeedbackContainer), 0, 1);
    aFeedbackLayout->addWidget(new QLabel(QStringLiteral("Digital"), aFeedbackContainer), 0, 2);
    aFeedbackLayout->addWidget(new QLabel(QStringLiteral("Analog Raw"), aFeedbackContainer), 0, 3);
    for (int i = 0; i < 4; ++i)
    {
        aFeedbackLayout->addWidget(new QLabel(QStringLiteral("A%1").arg(9 + i), aFeedbackContainer), i + 1, 0);
        auto* modeLabel = createValueLabel(aFeedbackContainer, QStringLiteral("DIGITAL"));
        auto* digLabel = createValueLabel(aFeedbackContainer, QStringLiteral("OFF"));
        auto* rawLabel = createMonospaceValueLabel(aFeedbackContainer, QStringLiteral("0"));
        aModeAppliedValues_[i] = modeLabel;
        aDigitalValues_[i] = digLabel;
        aAnalogRawValues_[i] = rawLabel;
        aFeedbackLayout->addWidget(modeLabel, i + 1, 1);
        aFeedbackLayout->addWidget(digLabel, i + 1, 2);
        aFeedbackLayout->addWidget(rawLabel, i + 1, 3);
    }

    auto* ccioFeedbackContainer = new QWidget(statusGroup);
    auto* ccioFeedbackLayout = new QGridLayout(ccioFeedbackContainer);
    ccioFeedbackLayout->setContentsMargins(0, 0, 0, 0);
    ccioFeedbackLayout->setHorizontalSpacing(12);
    ccioFeedbackLayout->setVerticalSpacing(4);
    ccioFeedbackLayout->setAlignment(Qt::AlignTop);
    ccioFeedbackLayout->addWidget(new QLabel(QStringLiteral("Channel"), ccioFeedbackContainer), 0, 0);
    ccioFeedbackLayout->addWidget(new QLabel(QStringLiteral("State"), ccioFeedbackContainer), 0, 1);
    ccioFeedbackLayout->addWidget(new QLabel(QStringLiteral("Direction"), ccioFeedbackContainer), 0, 2);
    ccioFeedbackLayout->addWidget(new QLabel(QStringLiteral("Fault"), ccioFeedbackContainer), 0, 3);
    for (int bit = 0; bit < 64; ++bit)
    {
        const int row = bit + 1;
        auto* pinLabel = new QLabel(ccioPinName(bit), ccioFeedbackContainer);
        auto* stateLabel = createValueLabel(ccioFeedbackContainer, QStringLiteral("OFF"));
        auto* dirLabel = createValueLabel(ccioFeedbackContainer, QStringLiteral("IN"));
        auto* faultLabel = createValueLabel(ccioFeedbackContainer, QStringLiteral("-"));
        ccioLivePinLabels_[static_cast<std::size_t>(bit)] = pinLabel;
        ccioLiveStateValues_[static_cast<std::size_t>(bit)] = stateLabel;
        ccioLiveDirectionValues_[static_cast<std::size_t>(bit)] = dirLabel;
        ccioLiveFaultValues_[static_cast<std::size_t>(bit)] = faultLabel;
        ccioFeedbackLayout->addWidget(pinLabel, row, 0);
        ccioFeedbackLayout->addWidget(stateLabel, row, 1);
        ccioFeedbackLayout->addWidget(dirLabel, row, 2);
        ccioFeedbackLayout->addWidget(faultLabel, row, 3);
        pinLabel->setVisible(false);
        stateLabel->setVisible(false);
        dirLabel->setVisible(false);
        faultLabel->setVisible(false);
    }

    statusLayout->addRow(QStringLiteral("Bus State"), stateValue_);
    statusLayout->addRow(QStringLiteral("Slaves"), slavesValue_);
    statusLayout->addRow(QStringLiteral("WKC (last / expected)"), wkcValue_);
    statusLayout->addRow(QStringLiteral("Loop Runtime"), cycleValue_);
    statusLayout->addRow(QStringLiteral("Control Word"), controlWordValue_);
    statusLayout->addRow(QStringLiteral("Status Word"), statusWordValue_);
    statusLayout->addRow(QStringLiteral("Supply Voltage"), supplyVoltageValue_);
    statusLayout->addRow(QStringLiteral("Sequence Ack"), sequenceAckValue_);
    statusLayout->addRow(QStringLiteral("IO0..IO5 Readback"), ioFeedbackContainer);
    statusLayout->addRow(QStringLiteral("DI6..DI8"), diContainer);
    statusLayout->addRow(QStringLiteral("A9..A12 Readback"), aFeedbackContainer);
    statusLayout->addRow(QStringLiteral("CCIO Status"), ccioLiveSummaryValue_);
    statusLayout->addRow(QStringLiteral("CCIO Readback"), ccioFeedbackContainer);
    statusLayout->addRow(QStringLiteral("Error"), errorValue_);

    auto* timingGroup = new QGroupBox(QStringLiteral("Sync / Pulse Timing"), central);
    auto* timingLayout = new QFormLayout(timingGroup);
    timingCyclePeriodValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0.000 ms"));
    timingLoopRuntimeValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0.000 ms"));
    timingLoopJitterValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0.000 ms"));
    timingMaxLoopJitterValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0.000 ms"));
    timingFrameLatencyValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0.000 ms"));
    timingMaxFrameLatencyValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0.000 ms"));
    timingUpdateConsistencyValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0.000 ms"));
    timingMaxUpdateConsistencyValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0.000 ms"));
    timingFirmwareLoopPeriodValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0 us"));
    timingFirmwareLoopRuntimeValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0 us"));
    timingFirmwareLoopJitterValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0 us"));
    timingFirmwareTransportValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0 us"));
    timingIsrValue_ = createMonospaceValueLabel(timingGroup, QStringLiteral("0 us"));

    auto* timingHint = new QLabel(
        QStringLiteral("Master values are measured around SOEM process-data exchange. "
                       "Firmware values are reported by the EtherCAT status PDO in microseconds. "
                       "ISR timing currently tracks the firmware Ethernet/EtherCAT service path."),
        timingGroup);
    timingHint->setWordWrap(true);
    timingHint->setStyleSheet(QStringLiteral("QLabel { color: #AEB7C2; }"));
    timingLayout->addRow(QStringLiteral("Cycle Period"), timingCyclePeriodValue_);
    timingLayout->addRow(QStringLiteral("Loop Runtime"), timingLoopRuntimeValue_);
    timingLayout->addRow(QStringLiteral("Loop Jitter"), timingLoopJitterValue_);
    timingLayout->addRow(QStringLiteral("Max Loop Jitter"), timingMaxLoopJitterValue_);
    timingLayout->addRow(QStringLiteral("Frame Latency"), timingFrameLatencyValue_);
    timingLayout->addRow(QStringLiteral("Max Frame Latency"), timingMaxFrameLatencyValue_);
    timingLayout->addRow(QStringLiteral("Update Consistency"), timingUpdateConsistencyValue_);
    timingLayout->addRow(QStringLiteral("Max Update Consistency"), timingMaxUpdateConsistencyValue_);
    timingLayout->addRow(QStringLiteral("Firmware Loop Period"), timingFirmwareLoopPeriodValue_);
    timingLayout->addRow(QStringLiteral("Firmware Loop Runtime"), timingFirmwareLoopRuntimeValue_);
    timingLayout->addRow(QStringLiteral("Firmware Loop Jitter"), timingFirmwareLoopJitterValue_);
    timingLayout->addRow(QStringLiteral("Firmware Transport Time"), timingFirmwareTransportValue_);
    timingLayout->addRow(QStringLiteral("ISR Timing"), timingIsrValue_);
    timingLayout->addRow(QStringLiteral("Notes"), timingHint);

    auto* logGroup = new QGroupBox(QStringLiteral("Runtime Log"), central);
    auto* logLayout = new QVBoxLayout(logGroup);
    logView_ = new QTextEdit(logGroup);
    logView_->setReadOnly(true);
    logView_->document()->setMaximumBlockCount(1200);
    logView_->setLineWrapMode(QTextEdit::NoWrap);
    logView_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    logLayout->addWidget(logView_);

    auto* configPage = new QWidget(contentStack_);
    auto* configPageLayout = new QVBoxLayout(configPage);
    configPageLayout->setContentsMargins(0, 0, 0, 0);
    configPageLayout->addWidget(configGroup);
    configPageLayout->addStretch(1);

    boardPage_ = new QWidget(contentStack_);
    auto* boardPageLayout = new QVBoxLayout(boardPage_);
    boardPageLayout->setContentsMargins(0, 0, 0, 0);
    boardPageLayout->addWidget(boardGroup, 1);

    ccioPage_ = new QWidget(contentStack_);
    auto* ccioPageLayout = new QVBoxLayout(ccioPage_);
    ccioPageLayout->setContentsMargins(0, 0, 0, 0);
    ccioPageLayout->addWidget(ccioGroup, 1);

    auto* statusScrollPage = new QScrollArea(contentStack_);
    statusPage_ = statusScrollPage;
    statusScrollPage->setWidgetResizable(true);
    auto* statusPageBody = new QWidget(statusScrollPage);
    auto* statusPageLayout = new QVBoxLayout(statusPageBody);
    statusPageLayout->setContentsMargins(0, 0, 0, 0);
    statusPageLayout->addWidget(statusGroup);
    statusPageLayout->addStretch(1);
    statusScrollPage->setWidget(statusPageBody);

    timingPage_ = new QWidget(contentStack_);
    auto* timingPageLayout = new QVBoxLayout(timingPage_);
    timingPageLayout->setContentsMargins(0, 0, 0, 0);
    timingPageLayout->addWidget(timingGroup);
    timingPageLayout->addStretch(1);

    auto* logPage = new QWidget(contentStack_);
    auto* logPageLayout = new QVBoxLayout(logPage);
    logPageLayout->setContentsMargins(0, 0, 0, 0);
    logPageLayout->addWidget(logGroup, 1);

    contentStack_->addWidget(configPage);
    contentStack_->addWidget(boardPage_);
    contentStack_->addWidget(ccioPage_);
    contentStack_->addWidget(statusPage_);
    contentStack_->addWidget(timingPage_);
    contentStack_->addWidget(logPage);
    navConnection->setChecked(true);
    setCentralWidget(central);

    connect(startStopButton_, &QPushButton::clicked, this, &MainWindow::onStartStopClicked);
    connect(refreshAdaptersButton_, &QPushButton::clicked, this, &MainWindow::onRefreshAdaptersClicked);
    connect(navGroup, &QButtonGroup::idClicked, contentStack_, &QStackedWidget::setCurrentIndex);
    connect(adapterCombo_, &QComboBox::currentIndexChanged, this, [this](int) {
        saveLastAdapterSelection(adapterCombo_);
    });
    if (adapterCombo_->lineEdit() != nullptr)
    {
        connect(adapterCombo_->lineEdit(), &QLineEdit::editingFinished, this, [this]() {
            saveLastAdapterSelection(adapterCombo_);
        });
    }

    connect(&worker_, &EthercatMasterThread::snapshotUpdated, this, &MainWindow::onSnapshotUpdated);
    connect(&worker_, &EthercatMasterThread::logMessage, this, &MainWindow::appendLog);
    connect(&worker_, &QThread::finished, this, [this]() {
        setRunningUi(false);
        appendLog(QStringLiteral("Worker thread stopped."));
    });

    onIoControlsChanged();
    onAChannelModeChanged();
    onCcioControlsChanged();
    refreshAdapters();
    restoreLastAdapterSelection(adapterCombo_);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveLastAdapterSelection(adapterCombo_);
    if (worker_.isRunning())
    {
        appendLog(QStringLiteral("Stopping worker before exit..."));
        worker_.requestStop();
        worker_.wait(3000);
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::onStartStopClicked()
{
    if (worker_.isRunning())
    {
        appendLog(QStringLiteral("Stop requested."));
        worker_.requestStop();
        startStopButton_->setEnabled(false);
        return;
    }

    const QString adapterName = selectedAdapterName(adapterCombo_);
    if (adapterName.isEmpty())
    {
        appendLog(QStringLiteral("Pick an adapter (or type one) before starting."));
        return;
    }

    saveLastAdapterSelection(adapterCombo_);
    worker_.configure(adapterName, cycleSpin_->value());
    worker_.setIoLevelMask(currentIoLevelMask());
    worker_.setIoDirectionMask(currentIoDirectionMask());
    worker_.setAAnalogMask(currentAAnalogMask());
    worker_.setCcioControl(ccioEnableCheck_ != nullptr && ccioEnableCheck_->isChecked(),
                           currentCcioOutputMask(), currentCcioDirectionMask());
    worker_.start();

    setRunningUi(true);
    appendLog(QStringLiteral("Starting master on adapter: %1").arg(adapterName));
}

void MainWindow::onRefreshAdaptersClicked()
{
    refreshAdapters();
}

void MainWindow::onIoControlsChanged()
{
    for (int i = 0; i < 6; ++i)
    {
        if (ioOutputDriveChecks_[i] != nullptr && ioOutputModeChecks_[i] != nullptr)
        {
            ioOutputDriveChecks_[i]->setEnabled(ioOutputModeChecks_[i]->isChecked());
        }
    }

    worker_.setIoLevelMask(currentIoLevelMask());
    worker_.setIoDirectionMask(currentIoDirectionMask());
}

void MainWindow::onAChannelModeChanged()
{
    worker_.setAAnalogMask(currentAAnalogMask());
}

void MainWindow::onCcioControlsChanged()
{
    updateCcioRowAvailability(std::max(0, ccioVisiblePinCount_));
    const bool enabled = ccioEnableCheck_ != nullptr && ccioEnableCheck_->isChecked();
    worker_.setCcioControl(enabled, currentCcioOutputMask(), currentCcioDirectionMask());
}

void MainWindow::onSnapshotUpdated(const MasterSnapshot& snapshot)
{
    stateValue_->setText(snapshot.stateText);
    slavesValue_->setText(QString::number(snapshot.slaveCount));
    if (snapshot.expectedWkc <= 0)
    {
        wkcValue_->setText(QStringLiteral("N/A (no PDO mapping)"));
    }
    else
    {
        wkcValue_->setText(QStringLiteral("%1 / %2").arg(snapshot.lastWkc).arg(snapshot.expectedWkc));
    }
    cycleValue_->setText(QStringLiteral("%1 ms").arg(snapshot.loopTimeMs, 0, 'f', 3));
    controlWordValue_->setText(QStringLiteral("0x%1").arg(snapshot.controlWord, 4, 16, QChar('0')).toUpper());
    statusWordValue_->setText(QStringLiteral("0x%1").arg(snapshot.statusWord, 4, 16, QChar('0')).toUpper());
    supplyVoltageValue_->setText(QStringLiteral("%1 V").arg(
        static_cast<double>(snapshot.supplyCentivolts) / 100.0, 0, 'f', 2));
    timingCyclePeriodValue_->setText(QStringLiteral("%1 ms").arg(snapshot.cyclePeriodMs, 0, 'f', 3));
    timingLoopRuntimeValue_->setText(QStringLiteral("%1 ms").arg(snapshot.loopTimeMs, 0, 'f', 3));
    timingLoopJitterValue_->setText(QStringLiteral("%1 ms").arg(snapshot.loopJitterMs, 0, 'f', 3));
    timingMaxLoopJitterValue_->setText(QStringLiteral("%1 ms").arg(snapshot.maxLoopJitterMs, 0, 'f', 3));
    timingFrameLatencyValue_->setText(QStringLiteral("%1 ms").arg(snapshot.frameLatencyMs, 0, 'f', 3));
    timingMaxFrameLatencyValue_->setText(QStringLiteral("%1 ms").arg(snapshot.maxFrameLatencyMs, 0, 'f', 3));
    timingUpdateConsistencyValue_->setText(QStringLiteral("%1 ms").arg(snapshot.updateConsistencyMs, 0, 'f', 3));
    timingMaxUpdateConsistencyValue_->setText(QStringLiteral("%1 ms").arg(snapshot.maxUpdateConsistencyMs, 0, 'f', 3));
    timingFirmwareLoopPeriodValue_->setText(QStringLiteral("%1 us").arg(snapshot.firmwareLoopPeriodUs));
    timingFirmwareLoopRuntimeValue_->setText(QStringLiteral("%1 us").arg(snapshot.firmwareLoopRuntimeUs));
    timingFirmwareLoopJitterValue_->setText(QStringLiteral("%1 us").arg(snapshot.firmwareLoopJitterUs));
    timingFirmwareTransportValue_->setText(QStringLiteral("%1 us").arg(snapshot.firmwareTransportUs));
    timingIsrValue_->setText(snapshot.isrTimingText);
    const QString ccioState = snapshot.ccioEnabled ? QStringLiteral("Enabled") : QStringLiteral("Disabled");
    const QString ccioLink = snapshot.ccioLinkBroken ? QStringLiteral(", link broken") : QStringLiteral("");
    const QString ccioOverload = snapshot.ccioAnyOverload ? QStringLiteral(", overload") : QStringLiteral("");
    ccioLiveSummaryValue_->setText(QStringLiteral("%1, %2 board(s)%3%4")
                                       .arg(ccioState)
                                       .arg(snapshot.ccioBoardCount)
                                       .arg(ccioLink, ccioOverload));
    ccioLiveSummaryValue_->setStyleSheet(snapshot.ccioLinkBroken || snapshot.ccioAnyOverload
                                             ? QStringLiteral("QLabel { color: #E38B29; font-weight: 700; }")
                                             : QStringLiteral("QLabel { color: #D6DDE6; font-weight: 600; }"));
    sequenceAckValue_->setText(QStringLiteral("0x%1").arg(snapshot.sequenceAck, 8, 16, QChar('0')).toUpper());
    if (boardWidget_ != nullptr && contentStack_->currentWidget() == boardPage_)
    {
        boardWidget_->setSnapshot(snapshot);
    }
    applySnapshotReadback(snapshot);
    errorValue_->setText(snapshot.lastError.isEmpty() ? QStringLiteral("-") : snapshot.lastError);
}

void MainWindow::appendLog(const QString& line)
{
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss.zzz"));
    logView_->append(QStringLiteral("[%1] %2").arg(stamp, line));
}

void MainWindow::setRunningUi(const bool running)
{
    startStopButton_->setEnabled(true);
    startStopButton_->setText(running ? QStringLiteral("Stop") : QStringLiteral("Start"));
    adapterCombo_->setEnabled(!running);
    refreshAdaptersButton_->setEnabled(!running);
    cycleSpin_->setEnabled(!running);
}

std::uint8_t MainWindow::currentIoLevelMask() const
{
    std::uint8_t mask = 0;
    for (int i = 0; i < 6; ++i)
    {
        if (ioOutputDriveChecks_[i] != nullptr && ioOutputDriveChecks_[i]->isChecked())
        {
            mask = static_cast<std::uint8_t>(mask | (1U << i));
        }
    }
    return mask;
}

std::uint8_t MainWindow::currentIoDirectionMask() const
{
    std::uint8_t mask = 0;
    for (int i = 0; i < 6; ++i)
    {
        if (ioOutputModeChecks_[i] != nullptr && ioOutputModeChecks_[i]->isChecked())
        {
            mask = static_cast<std::uint8_t>(mask | (1U << i));
        }
    }
    return mask;
}

std::uint8_t MainWindow::currentAAnalogMask() const
{
    std::uint8_t mask = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (aAnalogModeChecks_[i] != nullptr && aAnalogModeChecks_[i]->isChecked())
        {
            mask = static_cast<std::uint8_t>(mask | (1U << i));
        }
    }
    return mask;
}

std::uint64_t MainWindow::currentCcioOutputMask() const
{
    std::uint64_t mask = 0U;
    for (int bit = 0; bit < 64; ++bit)
    {
        const auto* check = ccioOutputDriveChecks_[static_cast<std::size_t>(bit)];
        if (check != nullptr && check->isChecked())
        {
            mask |= (1ULL << bit);
        }
    }
    return mask;
}

std::uint64_t MainWindow::currentCcioDirectionMask() const
{
    std::uint64_t mask = 0U;
    for (int bit = 0; bit < 64; ++bit)
    {
        const auto* check = ccioOutputModeChecks_[static_cast<std::size_t>(bit)];
        if (check != nullptr && check->isChecked())
        {
            mask |= (1ULL << bit);
        }
    }
    return mask;
}

void MainWindow::updateCcioRowAvailability(const int enabledPinCount)
{
    const int clampedPinCount = std::clamp(enabledPinCount, 0, 64);
    const bool ccioRequested = ccioEnableCheck_ != nullptr && ccioEnableCheck_->isChecked();
    const bool countChanged = ccioVisiblePinCount_ != clampedPinCount;
    ccioVisiblePinCount_ = clampedPinCount;

    for (int bit = 0; bit < 64; ++bit)
    {
        const bool rowAvailable = bit < clampedPinCount;
        if (countChanged)
        {
            QWidget* rowParent = nullptr;
            if (ccioPinLabels_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioPinLabels_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
                rowParent = ccioPinLabels_[static_cast<std::size_t>(bit)]->parentWidget();
            }
            if (ccioOutputModeChecks_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioOutputModeChecks_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
            }
            if (ccioOutputDriveChecks_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioOutputDriveChecks_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
            }
            if (ccioStateValues_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioStateValues_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
            }
            if (ccioDirectionValues_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioDirectionValues_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
            }
            if (ccioFaultValues_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioFaultValues_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
            }
            if (rowParent != nullptr && rowParent->layout() != nullptr)
            {
                auto* grid = qobject_cast<QGridLayout*>(rowParent->layout());
                if (grid != nullptr)
                {
                    grid->setRowMinimumHeight(bit + 1, rowAvailable ? 22 : 0);
                    grid->setRowStretch(bit + 1, 0);
                }
            }
        }

        auto* modeCheck = ccioOutputModeChecks_[static_cast<std::size_t>(bit)];
        auto* driveCheck = ccioOutputDriveChecks_[static_cast<std::size_t>(bit)];
        if (modeCheck != nullptr)
        {
            modeCheck->setEnabled(ccioRequested && rowAvailable);
        }
        if (driveCheck != nullptr)
        {
            driveCheck->setEnabled(ccioRequested && rowAvailable && modeCheck != nullptr && modeCheck->isChecked());
        }
    }
}

void MainWindow::applySnapshotReadback(const MasterSnapshot& snapshot)
{
    if (ccioSummaryValue_ != nullptr)
    {
        const QString state = snapshot.ccioEnabled ? QStringLiteral("Enabled") : QStringLiteral("Disabled");
        const QString link = snapshot.ccioLinkBroken ? QStringLiteral(", link broken") : QStringLiteral("");
        const QString overload = snapshot.ccioAnyOverload ? QStringLiteral(", overload") : QStringLiteral("");
        ccioSummaryValue_->setText(QStringLiteral("%1, %2 board(s)%3%4")
                                       .arg(state)
                                       .arg(snapshot.ccioBoardCount)
                                       .arg(link, overload));
        ccioSummaryValue_->setStyleSheet(snapshot.ccioLinkBroken || snapshot.ccioAnyOverload
                                             ? QStringLiteral("QLabel { color: #E38B29; font-weight: 700; }")
                                             : QStringLiteral("QLabel { color: #D6DDE6; font-weight: 600; }"));
    }

    if (contentStack_->currentWidget() == statusPage_)
    {
        const int ccioPinCount = snapshot.ccioEnabled
                                     ? std::clamp(static_cast<int>(snapshot.ccioBoardCount) * 8, 0, 64)
                                     : 0;
        const bool countChanged = ccioLiveVisiblePinCount_ != ccioPinCount;
        ccioLiveVisiblePinCount_ = ccioPinCount;
        for (int bit = 0; bit < 64; ++bit)
        {
            const bool rowAvailable = bit < ccioPinCount;
            if (countChanged)
            {
                if (ccioLivePinLabels_[static_cast<std::size_t>(bit)] != nullptr)
                {
                    ccioLivePinLabels_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
                }
                if (ccioLiveStateValues_[static_cast<std::size_t>(bit)] != nullptr)
                {
                    ccioLiveStateValues_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
                }
                if (ccioLiveDirectionValues_[static_cast<std::size_t>(bit)] != nullptr)
                {
                    ccioLiveDirectionValues_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
                }
                if (ccioLiveFaultValues_[static_cast<std::size_t>(bit)] != nullptr)
                {
                    ccioLiveFaultValues_[static_cast<std::size_t>(bit)]->setVisible(rowAvailable);
                }
            }
        }

        for (int bit = 0; bit < ccioPinCount; ++bit)
        {
            const std::uint64_t bitMask = 1ULL << bit;
            const bool active = (snapshot.ccioInputBits & bitMask) != 0U;
            const bool output = (snapshot.ccioDirectionApplied & bitMask) != 0U;
            const bool fault = (snapshot.ccioStatusBits & bitMask) != 0U;
            if (ccioLiveStateValues_[static_cast<std::size_t>(bit)] != nullptr)
            {
                setBadgeStyle(ccioLiveStateValues_[static_cast<std::size_t>(bit)], active,
                              QStringLiteral("ON"), QStringLiteral("OFF"));
            }
            if (ccioLiveDirectionValues_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioLiveDirectionValues_[static_cast<std::size_t>(bit)]->setText(output ? QStringLiteral("OUT") : QStringLiteral("IN"));
                ccioLiveDirectionValues_[static_cast<std::size_t>(bit)]->setStyleSheet(output
                                                                                          ? QStringLiteral("QLabel { color: #1E9BFF; font-weight: 700; }")
                                                                                          : QStringLiteral("QLabel { color: #676767; font-weight: 600; }"));
            }
            if (ccioLiveFaultValues_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioLiveFaultValues_[static_cast<std::size_t>(bit)]->setText(fault ? QStringLiteral("FAULT") : QStringLiteral("-"));
                ccioLiveFaultValues_[static_cast<std::size_t>(bit)]->setStyleSheet(fault
                                                                                       ? QStringLiteral("QLabel { color: #E04F5F; font-weight: 700; }")
                                                                                       : QStringLiteral("QLabel { color: #676767; font-weight: 600; }"));
            }
        }
    }

    if (contentStack_->currentWidget() == ccioPage_)
    {
        const int ccioPinCount = snapshot.ccioEnabled
                                     ? std::clamp(static_cast<int>(snapshot.ccioBoardCount) * 8, 0, 64)
                                     : 0;
        updateCcioRowAvailability(ccioPinCount);
        for (int bit = 0; bit < ccioPinCount; ++bit)
        {
            const std::uint64_t bitMask = 1ULL << bit;
            const bool active = (snapshot.ccioInputBits & bitMask) != 0U;
            const bool output = (snapshot.ccioDirectionApplied & bitMask) != 0U;
            const bool fault = (snapshot.ccioStatusBits & bitMask) != 0U;
            if (ccioStateValues_[static_cast<std::size_t>(bit)] != nullptr)
            {
                setBadgeStyle(ccioStateValues_[static_cast<std::size_t>(bit)], active,
                              QStringLiteral("ON"), QStringLiteral("OFF"));
            }
            if (ccioDirectionValues_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioDirectionValues_[static_cast<std::size_t>(bit)]->setText(output ? QStringLiteral("OUT") : QStringLiteral("IN"));
                ccioDirectionValues_[static_cast<std::size_t>(bit)]->setStyleSheet(output
                                                                                      ? QStringLiteral("QLabel { color: #1E9BFF; font-weight: 700; }")
                                                                                      : QStringLiteral("QLabel { color: #676767; font-weight: 600; }"));
            }
            if (ccioFaultValues_[static_cast<std::size_t>(bit)] != nullptr)
            {
                ccioFaultValues_[static_cast<std::size_t>(bit)]->setText(fault ? QStringLiteral("FAULT") : QStringLiteral("-"));
                ccioFaultValues_[static_cast<std::size_t>(bit)]->setStyleSheet(fault
                                                                                   ? QStringLiteral("QLabel { color: #E04F5F; font-weight: 700; }")
                                                                                   : QStringLiteral("QLabel { color: #676767; font-weight: 600; }"));
            }
        }
    }

    for (int i = 0; i < 6; ++i)
    {
        const bool ioState = ((snapshot.ioState >> i) & 0x01U) != 0U;
        const bool ioOut = ((snapshot.ioDirectionApplied >> i) & 0x01U) != 0U;
        if (ioStateValues_[i] != nullptr)
        {
            setBadgeStyle(ioStateValues_[i], ioState, QStringLiteral("ON"), QStringLiteral("OFF"));
        }
        if (ioDirectionValues_[i] != nullptr)
        {
            ioDirectionValues_[i]->setText(ioOut ? QStringLiteral("OUT") : QStringLiteral("IN"));
            ioDirectionValues_[i]->setStyleSheet(ioOut
                                                     ? QStringLiteral("QLabel { color: #1E4FA8; font-weight: 700; }")
                                                     : QStringLiteral("QLabel { color: #676767; font-weight: 600; }"));
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        const bool diState = ((snapshot.diBits >> i) & 0x01U) != 0U;
        if (diValues_[i] != nullptr)
        {
            diValues_[i]->setText(QStringLiteral("DI%1: %2").arg(6 + i).arg(diState ? QStringLiteral("ON") : QStringLiteral("OFF")));
            diValues_[i]->setStyleSheet(diState
                                            ? QStringLiteral("QLabel { color: #0B7D30; font-weight: 700; }")
                                            : QStringLiteral("QLabel { color: #676767; font-weight: 600; }"));
        }
    }

    const std::array<quint16, 4> analogRaw = {snapshot.a9Raw, snapshot.a10Raw, snapshot.a11Raw, snapshot.a12Raw};
    for (int i = 0; i < 4; ++i)
    {
        const bool analogMode = ((snapshot.aAnalogMaskApplied >> i) & 0x01U) != 0U;
        const bool digitalState = ((snapshot.aDigitalBits >> i) & 0x01U) != 0U;
        if (aModeAppliedValues_[i] != nullptr)
        {
            aModeAppliedValues_[i]->setText(analogMode ? QStringLiteral("ANALOG") : QStringLiteral("DIGITAL"));
        }
        if (aDigitalValues_[i] != nullptr)
        {
            if (analogMode)
            {
                aDigitalValues_[i]->setText(QStringLiteral("n/a"));
                aDigitalValues_[i]->setStyleSheet(QStringLiteral("QLabel { color: #888888; }"));
            }
            else
            {
                setBadgeStyle(aDigitalValues_[i], digitalState, QStringLiteral("ON"), QStringLiteral("OFF"));
            }
        }
        if (aAnalogRawValues_[i] != nullptr)
        {
            aAnalogRawValues_[i]->setText(QString::number(analogRaw[static_cast<std::size_t>(i)]));
        }
    }
}

void MainWindow::refreshAdapters()
{
    const QString previousValue = selectedAdapterName(adapterCombo_);
    const QSignalBlocker comboSignalBlocker(adapterCombo_);
    adapterCombo_->clear();

    ec_adaptert* adapters = ec_find_adapters();
    int discoveredCount = 0;

    for (ec_adaptert* current = adapters; current != nullptr; current = current->next)
    {
        const QString adapterName = QString::fromLocal8Bit(current->name != nullptr ? current->name : "");
        if (adapterName.isEmpty())
        {
            continue;
        }

        const QString adapterDescription = QString::fromLocal8Bit(current->desc != nullptr ? current->desc : "");
        const QString label = adapterDescription.isEmpty()
                                  ? adapterName
                                  : QStringLiteral("%1 (%2)").arg(adapterDescription, adapterName);
        adapterCombo_->addItem(label, adapterName);
        ++discoveredCount;
    }

    if (adapters != nullptr)
    {
        ec_free_adapters(adapters);
    }

    if (!previousValue.isEmpty())
    {
        const int existingIndex = adapterCombo_->findData(previousValue);
        if (existingIndex >= 0)
        {
            adapterCombo_->setCurrentIndex(existingIndex);
        }
        else
        {
            adapterCombo_->setEditText(previousValue);
        }
    }
    else if (adapterCombo_->count() > 0)
    {
        adapterCombo_->setCurrentIndex(0);
    }

    if (discoveredCount > 0)
    {
        appendLog(QStringLiteral("Discovered %1 adapter(s).").arg(discoveredCount));
    }
    else
    {
        appendLog(QStringLiteral("No adapters discovered. Ensure Npcap is installed and app runs as Administrator."));
    }
}
