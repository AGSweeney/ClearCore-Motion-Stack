import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1480
    height: 895
    minimumWidth: 1480
    minimumHeight: 895
    visible: true
    title: "MotionBench for ClearLink Compatible Devices"

    // Industrial palette (above property bindings that reference it)
    QtObject {
        id: benchUi
        readonly property color panelBgOuter: "#d8dce2"
        readonly property color panelBg: "#f4f6f8"
        readonly property color panelStroke: "#8b96a8"
        readonly property color insetBg: "#e8ecf0"
        readonly property color insetStroke: "#bdc6d2"
        readonly property color titleInk: "#1a2433"
        readonly property color sectionInk: "#2d3f56"
        readonly property color labelMuted: "#5a6b7f"
        readonly property color valueInk: "#0f1419"
        readonly property color onColor: "#0d9d61"
        readonly property color offColor: "#a8b4c4"
        readonly property color warnColor: "#c47f00"
        readonly property color barBg: "#1e2836"
        readonly property string monoFamily: "Consolas"
        readonly property font valueFont: Qt.font({
            family: benchUi.monoFamily,
            pointSize: 9
        })
    }

    color: benchUi.panelBgOuter
    font.family: "Segoe UI"
    font.pointSize: 9
    palette.highlight: "#3d5a80"
    palette.highlightedText: "#ffffff"
    palette.base: "#ffffff"
    palette.text: benchUi.valueInk
    palette.button: "#f4f6f8"
    palette.buttonText: benchUi.valueInk

    property var deviceService: appViewModel.deviceService
    property bool pendingOperatorFieldSync: false
    property int pendingOperatorFieldSyncMotor: 1

    readonly property var statusBitLabels: [
        "At Target Position", "Steps Active", "At Velocity", "Move Direction",
        "In Positive Limit", "In Negative Limit", "In E-Stop", "In Home Sensor",
        "Homing", "Motor In Fault", "Enabled", "At/Outside Soft Limits",
        "Positional Move", "Has Homed", "HLFB On", "Has Torque Measurement",
        "Ready To Home", "Shutdowns Present", "Add To Position Ack",
        "Load Position Move Ack", "Load Velocity Move Ack", "Clear Motor Fault Ack"
    ]
    readonly property var alertBitLabels: [
        "Command While Shutdown", "Positive Limit", "Negative Limit", "Sensor E-Stop",
        "SW E-Stop", "Motor Disabled", "Soft Limits Exceeded", "Follower Axis Fault",
        "Command While Following", "Motion Canceled Homing Not Ready",
        "Motor Faulted", "Following Overspeed"
    ]
    readonly property var ccioRowLabels: [
        "CCIO:1", "CCIO:2", "CCIO:3", "CCIO:4",
        "CCIO:5", "CCIO:6", "CCIO:7", "CCIO:8"
    ]
    readonly property var discreteInputLabels: [
        "I/O-0", "I/O-1", "I/O-2", "I/O-3", "I/O-4", "I/O-5",
        "DI-6", "DI-7", "DI-8",
        "A-9", "A-10", "A-11", "A-12"
    ]
    readonly property var mconnectorStatusLabels: [
        "HLFB_On", "Has PWM", "Trigger Pulses Active", "Trigger Pulses ACK", "Disable Pulse ACK"
    ]
    readonly property var mconnectorOutputLabels: [
        "Enabled", "Output A", "Output B", "Disable Pulse"
    ]

    function bitValue(bits, idx) {
        if (!bits || idx >= bits.length)
            return false
        return !!bits[idx]
    }

    /// Integer display string (never scientific notation)
    function intValue(value, fallback) {
        if (value === undefined || value === null)
            return fallback
        return parseInt(Number(value), 10).toString()
    }

    function indicatorColor(isOn) {
        return isOn ? benchUi.onColor : benchUi.offColor
    }

    /// Device uses -9999 as “no torque” / invalid (legacy ClearLink behavior)
    function formatTorque(raw) {
        const n = Number(raw)
        if (!isFinite(n) || Math.abs(n + 9999) < 0.25)
            return "---"
        return n.toFixed(4)
    }

    function formatNumericOrDash(raw, decimals) {
        const n = Number(raw)
        if (!isFinite(n))
            return "---"
        return n.toFixed(decimals)
    }

    function mergedIpChoices() {
        const merged = []
        const seen = {}
        const current = (deviceService.targetIp || "").trim()
        if (current.length > 0) {
            merged.push(current)
            seen[current] = true
        }
        const discovered = appViewModel.discoveryIps || []
        for (let i = 0; i < discovered.length; ++i) {
            const ip = (discovered[i] || "").trim()
            if (ip.length > 0 && !seen[ip]) {
                merged.push(ip)
                seen[ip] = true
            }
        }
        return merged
    }

    function ccioBit(row, col) {
        return bitValue(deviceService.monitorData.ccioBits, row * 8 + col)
    }

    function ccioOutputBit(row, col) {
        const outputBits = deviceService.monitorData.ccioOutputBits
        if (!outputBits || outputBits.length === 0)
            return false
        return bitValue(outputBits, row * 8 + col)
    }

    function detectedCcioBoardCount() {
        const raw = Number(deviceService.monitorData.ccioBoardCount)
        if (!isFinite(raw))
            return 0
        return Math.max(0, Math.min(8, Math.trunc(raw)))
    }

    function externalScannerOwnsIo() {
        return !!deviceService.monitorData.scannerConnected
    }

    function operatorWritesAllowed() {
        return deviceService.connected
                && deviceService.operatorControlsEnabled
                && !externalScannerOwnsIo()
    }

    function isMConnectorModeActive() {
        return !!(deviceService.connected
                  && deviceService.monitorData
                  && deviceService.monitorData.boardModeMConnector)
    }

    function setMotorInstance(instanceId) {
        deviceService.selectedMotorInstance = instanceId
    }

    function clearLoadMoveControlBits() {
        deviceService.writeControlBit(3, false)
        deviceService.writeControlBit(4, false)
    }

    function parseCfgInt(textValue, fallbackValue) {
        const parsed = parseInt((textValue || "").trim(), 10)
        if (!isFinite(parsed))
            return fallbackValue
        return parsed
    }

    function parseCfgConnector(textValue) {
        const trimmed = (textValue || "").trim().toUpperCase()
        if (trimmed.length === 0 || trimmed === "NA")
            return -1
        const parsed = parseInt(trimmed, 10)
        if (!isFinite(parsed))
            return -1
        return parsed
    }

    function openMotorConfigDialog() {
        if (isMConnectorModeActive()) {
            cfgEditMcEnableConnector.text = intValue(deviceService.monitorData.mconnectorCfgEnableConnector, "-1")
            cfgEditMcInputAConnector.text = intValue(deviceService.monitorData.mconnectorCfgInputAConnector, "-1")
            cfgEditMcInputBConnector.text = intValue(deviceService.monitorData.mconnectorCfgInputBConnector, "-1")
            cfgEditMcTriggerPulseTime.text = intValue(deviceService.monitorData.mconnectorTriggerPulseTime, "30")
            motorConfigDialog.open()
            return
        }

        const cfgBits = deviceService.monitorData.cfgRegBits || []
        cfgEditHomingEnable.checked = bitValue(cfgBits, 0)
        cfgEditHomeSensorActiveHigh.checked = bitValue(cfgBits, 1)
        cfgEditEnableInversion.checked = bitValue(cfgBits, 2)
        cfgEditHlfbInversion.checked = bitValue(cfgBits, 3)
        cfgEditPosCaptureActiveHigh.checked = bitValue(cfgBits, 4)
        cfgEditSoftLimitEnable.checked = bitValue(cfgBits, 5)

        cfgEditSoftLimit1.text = intValue(deviceService.monitorData.cfgSoftLimitPos1, "0")
        cfgEditSoftLimit2.text = intValue(deviceService.monitorData.cfgSoftLimitPos2, "0")
        cfgEditPosLimitInput.text = deviceService.monitorData.cfgPosLimitInput || "NA"
        cfgEditNegLimitInput.text = deviceService.monitorData.cfgNegLimitInput || "NA"
        cfgEditHomeSensor.text = deviceService.monitorData.cfgHomeSensor || "NA"
        cfgEditBrakeOutput.text = deviceService.monitorData.cfgBrakeOutput || "NA"
        cfgEditPosCapture.text = deviceService.monitorData.cfgPosCapture || "NA"
        cfgEditMaxDecelRate.text = intValue(deviceService.monitorData.cfgMaxDecelRate, "0")
        cfgEditStopSensor.text = deviceService.monitorData.cfgStopSensor || "NA"
        cfgEditFollowEncoder.text = deviceService.monitorData.cfgFollowEncoder || "NA"
        cfgEditFollowDivisor.text = intValue(deviceService.monitorData.cfgFollowDivisor, "1")
        cfgEditFollowMultiplier.text = intValue(deviceService.monitorData.cfgFollowMultiplier, "1")

        motorConfigDialog.open()
    }

    function saveMotorConfigDialog() {
        if (isMConnectorModeActive()) {
            let okMc = true
            okMc = deviceService.writeMotorConfigField("mconnector_cfg.enable_connector", parseCfgConnector(cfgEditMcEnableConnector.text)) && okMc
            okMc = deviceService.writeMotorConfigField("mconnector_cfg.input_a_connector", parseCfgConnector(cfgEditMcInputAConnector.text)) && okMc
            okMc = deviceService.writeMotorConfigField("mconnector_cfg.input_b_connector", parseCfgConnector(cfgEditMcInputBConnector.text)) && okMc
            okMc = deviceService.writeMotorConfigField(
                        "mconnector_cfg.trigger_pulse_time_ms",
                        Math.max(0, parseCfgInt(cfgEditMcTriggerPulseTime.text, 30))) && okMc
            if (okMc) {
                deviceService.refreshOnce()
                motorConfigDialog.close()
            }
            return
        }

        let cfgReg = 0
        if (cfgEditHomingEnable.checked) cfgReg |= (1 << 0)
        if (cfgEditHomeSensorActiveHigh.checked) cfgReg |= (1 << 1)
        if (cfgEditEnableInversion.checked) cfgReg |= (1 << 2)
        if (cfgEditHlfbInversion.checked) cfgReg |= (1 << 3)
        if (cfgEditPosCaptureActiveHigh.checked) cfgReg |= (1 << 4)
        if (cfgEditSoftLimitEnable.checked) cfgReg |= (1 << 5)

        let ok = true
        ok = deviceService.writeMotorConfigField("motor_cfg.config_reg", cfgReg) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.soft_limit_pos1", parseCfgInt(cfgEditSoftLimit1.text, 0)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.soft_limit_pos2", parseCfgInt(cfgEditSoftLimit2.text, 0)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.pos_limit_input", parseCfgConnector(cfgEditPosLimitInput.text)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.neg_limit_input", parseCfgConnector(cfgEditNegLimitInput.text)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.home_sensor", parseCfgConnector(cfgEditHomeSensor.text)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.brake_output", parseCfgConnector(cfgEditBrakeOutput.text)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.pos_capture", parseCfgConnector(cfgEditPosCapture.text)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.max_decel_rate", parseCfgInt(cfgEditMaxDecelRate.text, 0)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.stop_sensor", parseCfgConnector(cfgEditStopSensor.text)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.follow_encoder", parseCfgConnector(cfgEditFollowEncoder.text)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.follow_divisor", parseCfgInt(cfgEditFollowDivisor.text, 1)) && ok
        ok = deviceService.writeMotorConfigField("motor_cfg.follow_multiplier", parseCfgInt(cfgEditFollowMultiplier.text, 1)) && ok

        if (ok) {
            deviceService.refreshOnce()
            motorConfigDialog.close()
        }
    }

    function queueOperatorFieldSync(triggerRefresh) {
        pendingOperatorFieldSync = true
        pendingOperatorFieldSyncMotor = deviceService.selectedMotorInstance
        if (triggerRefresh && deviceService.connected)
            deviceService.refreshOnce()
    }

    function syncOperatorFieldsOnce() {
        if (!pendingOperatorFieldSync)
            return
        if (pendingOperatorFieldSyncMotor !== deviceService.selectedMotorInstance)
            return

        velocityField.text = intValue(deviceService.monitorData.velocity, "0")
        accelField.text = intValue(deviceService.monitorData.accel, "0")
        decelField.text = intValue(deviceService.monitorData.decel, "0")
        distanceField.text = intValue(deviceService.monitorData.moveDistance, "0")
        moveVelocityField.text = intValue(deviceService.monitorData.moveVelocity, "0")

        pendingOperatorFieldSync = false
    }

    Connections {
        target: deviceService

        function onConnectionChanged() {
            if (deviceService.connected) {
                queueOperatorFieldSync(false)
            } else {
                pendingOperatorFieldSync = false
            }
        }

        function onSelectedMotorInstanceChanged() {
            if (deviceService.connected)
                queueOperatorFieldSync(true)
        }

        function onMonitorDataChanged() {
            syncOperatorFieldsOnce()
        }

        function onTargetIpChanged() {
            if (targetIpCombo.editText !== deviceService.targetIp)
                targetIpCombo.editText = deviceService.targetIp
        }
    }

    component ValueCell : Rectangle {
        property alias text: t.text

        implicitHeight: 22
        Layout.minimumWidth: 72
        Layout.fillWidth: true
        radius: 2
        color: benchUi.insetBg
        border.color: benchUi.insetStroke
        border.width: 1
        Label {
            id: t
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 6
            anchors.rightMargin: 6
            font: benchUi.valueFont
            color: benchUi.valueInk
        }
    }

    component LedRow : RowLayout {
        property string ledLabel
        property bool ledOn
        property int ledLabelWidth: root.regLabelWidth

        spacing: 8
        Label {
            text: ledLabel + ":"
            Layout.preferredWidth: ledLabelWidth
            horizontalAlignment: Text.AlignRight
            color: benchUi.labelMuted
            elide: Text.ElideRight
            font.pointSize: 9
        }
        Rectangle {
            implicitWidth: 11
            implicitHeight: 11
            radius: 6
            color: root.indicatorColor(ledOn)
            border.width: 1
            border.color: "#7a8698"
        }
        Item { Layout.fillWidth: true }
    }

    component PanelCard : Rectangle {
        id: cardRoot
        property string cardTitle
        default property alias body: bodySlot.data

        color: benchUi.panelBg
        border.color: benchUi.panelStroke
        border.width: 1
        radius: 3
        clip: true
        implicitHeight: panelColumn.implicitHeight

        ColumnLayout {
            id: panelColumn
            width: cardRoot.width
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 28
                color: "#e2e6ec"
                Rectangle {
                    width: 4
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    color: "#3d5a80"
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    text: cardRoot.cardTitle
                    font.bold: true
                    font.pointSize: 9
                    color: benchUi.titleInk
                }
            }

            ColumnLayout {
                id: bodySlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 12
                Layout.bottomMargin: 12
            }
        }
    }

    component ToolBtn : Button {
        implicitHeight: 28
        font.pointSize: 9
    }

    readonly property int regLabelWidth: 178

    header: ToolBar {
        implicitHeight: 48
        background: Rectangle { color: benchUi.barBg }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

            Label { text: "Target IP"; color: "#9eb0c7"; font.pointSize: 9 }
            ComboBox {
                id: targetIpCombo
                editable: true
                Layout.preferredWidth: 220
                implicitHeight: 28
                font.pointSize: 9
                model: mergedIpChoices()
                editText: deviceService.targetIp
                palette.highlight: "#3d5a80"
                palette.highlightedText: "#ffffff"
                palette.base: "#f4f6f8"
                palette.text: benchUi.valueInk
                palette.button: "#f4f6f8"
                palette.buttonText: benchUi.valueInk
                background: Rectangle {
                    color: targetIpCombo.enabled ? "#ffffff" : "#e8ecf0"
                    border.color: benchUi.insetStroke
                    border.width: 1
                    radius: 1
                }
                onAccepted: {
                    deviceService.targetIp = editText
                }
                onActivated: {
                    if (currentIndex >= 0) {
                        const ip = model[currentIndex]
                        editText = ip
                        deviceService.targetIp = ip
                    }
                }
            }
            ToolBtn { text: "Discover"; onClicked: appViewModel.discover() }
            ToolBtn {
                text: deviceService.connected ? "Disconnect" : "Connect"
                onClicked: {
                    if (deviceService.connected) {
                        deviceService.disconnectDevice()
                    } else {
                        deviceService.targetIp = targetIpCombo.editText
                        deviceService.connectDevice()
                    }
                }
            }
            ToolBtn { text: "Refresh"; enabled: deviceService.connected; onClicked: deviceService.refreshOnce() }
            ToolBtn {
                text: deviceService.pollingEnabled ? "Stop Polling" : "Start Polling"
                enabled: deviceService.connected
                onClicked: deviceService.pollingEnabled ? deviceService.stopPolling() : deviceService.startPolling()
            }
            Label { text: "Poll (ms)"; color: "#9eb0c7"; font.pointSize: 9 }
            SpinBox {
                value: deviceService.pollIntervalMs
                from: 50
                to: 5000
                stepSize: 50
                Layout.preferredWidth: 96
                implicitHeight: 28
                font.pointSize: 9
                palette.highlight: "#3d5a80"
                palette.highlightedText: "#ffffff"
                onValueModified: deviceService.pollIntervalMs = value
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 9
                height: 9
                radius: 5
                color: deviceService.connected ? benchUi.onColor : "#e14545"
            }
            Label {
                text: deviceService.connected ? "ONLINE" : "OFFLINE"
                color: deviceService.connected ? benchUi.onColor : "#e14545"
                font.bold: true
                font.pointSize: 9
            }
            Label {
                text: deviceService.lastError
                color: "#f0c84a"
                font.pointSize: 8
                visible: text.length > 0
                Layout.maximumWidth: 280
                elide: Text.ElideRight
            }
        }
    }

    footer: Rectangle {
        implicitHeight: 26
        color: benchUi.barBg
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            Label {
                text: "MotionBench  |  ClearLink  |  Motor M-" + (deviceService.selectedMotorInstance - 1)
                color: "#7d8fa3"
                font.pointSize: 8
            }
            Item { Layout.fillWidth: true }
            Label {
                text: deviceService.lastError.length > 0 ? ("Alert: " + deviceService.lastError) : ""
                color: "#f0c84a"
                font.pointSize: 8
                visible: deviceService.lastError.length > 0
            }
        }
    }

    ScrollView {
        id: mainScrollView
        anchors.fill: parent
        clip: true
        leftPadding: 12
        rightPadding: 12
        topPadding: 12
        bottomPadding: 12

        ColumnLayout {
            id: mainColumn
            width: Math.max(mainScrollView.availableWidth > 0 ? mainScrollView.availableWidth : root.width - 24, 400)
            spacing: 14

            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                PanelCard {
                    Layout.preferredWidth: 360
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignTop
                    cardTitle: "Device & I/O"

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 10

                        GridLayout {
                            columns: 2
                            columnSpacing: 14
                            rowSpacing: 8
                            Layout.fillWidth: true
                            Label { text: "Serial"; color: benchUi.labelMuted; Layout.minimumWidth: 92; Layout.maximumWidth: 92; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                            ValueCell { text: intValue(deviceService.monitorData.serialNumber, "---"); Layout.fillWidth: false; Layout.preferredWidth: 142 }
                            Label { text: "MAC"; color: benchUi.labelMuted; Layout.minimumWidth: 92; Layout.maximumWidth: 92; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                            ValueCell { text: deviceService.monitorData.macAddress || "---"; Layout.fillWidth: false; Layout.preferredWidth: 142 }
                            Label { text: "Board mode"; color: benchUi.labelMuted; Layout.minimumWidth: 92; Layout.maximumWidth: 92; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                            ValueCell { text: deviceService.monitorData.boardModeMConnector ? "M-connector" : "Step/Dir"; Layout.fillWidth: false; Layout.preferredWidth: 142 }
                            Label { text: "I/O owner"; color: benchUi.labelMuted; Layout.minimumWidth: 92; Layout.maximumWidth: 92; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                            ValueCell {
                                text: !!deviceService.monitorData.scannerConnected ? "External scanner" : "None"
                                Layout.fillWidth: false
                                Layout.preferredWidth: 142
                            }
                            Label { text: "DC bus (V)"; color: benchUi.labelMuted; Layout.minimumWidth: 92; Layout.maximumWidth: 92; Layout.alignment: Qt.AlignRight | Qt.AlignVCenter }
                            ValueCell { text: formatNumericOrDash(deviceService.supplyVoltage, 4); Layout.fillWidth: false; Layout.preferredWidth: 142 }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: benchUi.insetStroke
                        }

                        Label {
                            text: "Motor select"
                            font.bold: true
                            color: benchUi.sectionInk
                        }
                        RowLayout {
                            spacing: 12
                            Layout.fillWidth: true
                            Repeater {
                                model: 4
                                delegate: RadioButton {
                                    required property int index
                                    text: "M-" + index
                                    font.pointSize: 9
                                    checked: deviceService.selectedMotorInstance === (index + 1)
                                    onClicked: setMotorInstance(index + 1)
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: benchUi.insetStroke
                        }

                        Label {
                            text: "Discrete inputs"
                            font.bold: true
                            color: benchUi.sectionInk
                        }
                        GridLayout {
                            columns: 3
                            columnSpacing: 10
                            rowSpacing: 5
                            Layout.fillWidth: true
                            Repeater {
                                model: 13
                                delegate: RowLayout {
                                    required property int index
                                    spacing: 6
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 112
                                    Label {
                                        text: discreteInputLabels[index]
                                        Layout.preferredWidth: 42
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                        font.family: benchUi.monoFamily
                                        font.pointSize: 9
                                    }
                                    Rectangle {
                                        Layout.preferredWidth: 10
                                        Layout.preferredHeight: 10
                                        radius: 5
                                        color: indicatorColor(bitValue(deviceService.monitorData.discreteInputs, index))
                                        border.width: 1
                                        border.color: "#7a8698"
                                    }
                                    Label {
                                        text: bitValue(deviceService.monitorData.discreteInputs, index) ? "ON" : "off"
                                        Layout.preferredWidth: 26
                                        horizontalAlignment: Text.AlignLeft
                                        font.family: benchUi.monoFamily
                                        font.pointSize: 8
                                        color: bitValue(deviceService.monitorData.discreteInputs, index) ? benchUi.onColor : benchUi.labelMuted
                                    }
                                    Item { Layout.fillWidth: true }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: benchUi.insetStroke
                        }

                        Label {
                            text: "CCIO status"
                            font.bold: true
                            color: benchUi.sectionInk
                        }
                        RowLayout {
                            spacing: 0
                            Label { text: ""; Layout.preferredWidth: 56 }
                            Repeater {
                                model: 8
                                delegate: Label {
                                    required property int index
                                    text: index.toString()
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.preferredWidth: 22
                                    font.family: benchUi.monoFamily
                                    font.pointSize: 8
                                    color: benchUi.labelMuted
                                }
                            }
                        }
                        Repeater {
                            model: 8
                            delegate: RowLayout {
                                id: ccioStatusRow
                                required property int index
                                spacing: 0
                                Label {
                                    text: ccioRowLabels[ccioStatusRow.index]
                                    Layout.preferredWidth: 56
                                    font.family: benchUi.monoFamily
                                    font.pointSize: 8
                                    color: benchUi.labelMuted
                                }
                                Repeater {
                                    model: 8
                                    delegate: Item {
                                        id: ccioBitCell
                                        required property int index
                                        Layout.preferredWidth: 22
                                        implicitHeight: 14
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 10
                                            height: 10
                                            radius: 5
                                            color: indicatorColor(ccioBit(ccioStatusRow.index, ccioBitCell.index))
                                            border.width: 1
                                            border.color: "#7a8698"
                                        }
                                    }
                                }
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            Label { text: "CCIO"; color: benchUi.labelMuted }
                            ValueCell { text: deviceService.monitorData.ccioEnabled ? "Enabled" : "Disabled"; Layout.fillWidth: false; Layout.preferredWidth: 78 }
                            ToolBtn {
                                text: deviceService.monitorData.ccioEnabled ? "Disable" : "Enable"
                                enabled: deviceService.connected
                                onClicked: {
                                    if (deviceService.writeCcioEnabled(!deviceService.monitorData.ccioEnabled))
                                        deviceService.refreshOnce()
                                }
                            }
                            Label { text: "Boards"; color: benchUi.labelMuted }
                            ValueCell { text: intValue(deviceService.monitorData.ccioBoardCount, "0"); Layout.fillWidth: false; Layout.preferredWidth: 46 }
                            Item { Layout.fillWidth: true }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: benchUi.insetStroke
                        }

                        Label {
                            text: "Motor configuration"
                            font.bold: true
                            color: benchUi.sectionInk
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            Item { Layout.fillWidth: true }
                            ToolBtn {
                                Layout.fillWidth: false
                                Layout.preferredWidth: 170
                                text: "Edit motor config"
                                enabled: operatorWritesAllowed()
                                onClicked: openMotorConfigDialog()
                            }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                PanelCard {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignTop
                    Layout.minimumWidth: 520
                    visible: !isMConnectorModeActive()
                    cardTitle: "Motor state · motion · configuration"

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8

                        ScrollView {
                            id: registerScrollView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.minimumHeight: 240
                            clip: true
                            RowLayout {
                                id: registerRow
                                width: registerScrollView.availableWidth > 0
                                       ? registerScrollView.availableWidth
                                       : implicitWidth
                                spacing: 20
                                ColumnLayout {
                                    id: statusRegCol
                                    readonly property int statusLedLabelWidth: root.regLabelWidth
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: (registerRow.width - registerRow.spacing) / 2
                                    Layout.minimumWidth: (registerRow.width - registerRow.spacing) / 2
                                    Layout.alignment: Qt.AlignTop
                                    spacing: 1
                                    Label {
                                        text: "Status register"
                                        font.bold: true
                                        color: benchUi.sectionInk
                                        bottomPadding: 4
                                        Layout.preferredWidth: statusLedLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                    }
                                    Repeater {
                                        model: statusBitLabels.length
                                        delegate: LedRow {
                                            required property int index
                                            ledLabel: statusBitLabels[index]
                                            ledLabelWidth: statusRegCol.statusLedLabelWidth
                                            ledOn: bitValue(deviceService.monitorData.motorStatusBits, index)
                                        }
                                    }
                                }
                                ColumnLayout {
                                    id: alertRegCol
                                    readonly property int alertLedLabelWidth: 228
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: (registerRow.width - registerRow.spacing) / 2
                                    Layout.minimumWidth: (registerRow.width - registerRow.spacing) / 2
                                    Layout.alignment: Qt.AlignTop
                                    spacing: 1
                                    Label {
                                        text: "Alert register"
                                        font.bold: true
                                        color: benchUi.sectionInk
                                        bottomPadding: 4
                                        Layout.preferredWidth: alertLedLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                    }
                                    Repeater {
                                        model: alertBitLabels.length
                                        delegate: LedRow {
                                            required property int index
                                            ledLabel: alertBitLabels[index]
                                            ledLabelWidth: alertRegCol.alertLedLabelWidth
                                            ledOn: bitValue(deviceService.monitorData.motorAlertBits, index)
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: benchUi.insetStroke
                        }

                        Label {
                            text: "Motion data"
                            font.bold: true
                            color: benchUi.sectionInk
                        }
                        GridLayout {
                            columns: 4
                            columnSpacing: 10
                            rowSpacing: 4
                            Layout.fillWidth: true
                            Label { text: "Cmd position"; color: benchUi.labelMuted; Layout.minimumWidth: 108; horizontalAlignment: Text.AlignRight }
                            ValueCell { text: intValue(deviceService.monitorData.commandedPosition, "0") }
                            Label { text: "Cmd velocity"; color: benchUi.labelMuted; Layout.minimumWidth: 108; horizontalAlignment: Text.AlignRight }
                            ValueCell { text: intValue(deviceService.monitorData.commandedVelocity, "0") }
                            Label { text: "Target position"; color: benchUi.labelMuted; Layout.minimumWidth: 108; horizontalAlignment: Text.AlignRight }
                            ValueCell { text: intValue(deviceService.monitorData.targetPosition, "0") }
                            Label { text: "Target velocity"; color: benchUi.labelMuted; Layout.minimumWidth: 108; horizontalAlignment: Text.AlignRight }
                            ValueCell { text: intValue(deviceService.monitorData.targetVelocity, "0") }
                            Label { text: "Captured position"; color: benchUi.labelMuted; Layout.minimumWidth: 108; horizontalAlignment: Text.AlignRight }
                            ValueCell { text: intValue(deviceService.monitorData.capturedPosition, "0") }
                            Label { text: "Measured torque"; color: benchUi.labelMuted; Layout.minimumWidth: 108; horizontalAlignment: Text.AlignRight }
                            ValueCell { text: formatTorque(deviceService.monitorData.measuredTorque) }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: benchUi.insetStroke
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "Motor " + (deviceService.selectedMotorInstance - 1) + " configuration"
                                font.bold: true
                                color: benchUi.sectionInk
                            }
                        }

                        RowLayout {
                            spacing: 16
                            Layout.fillWidth: true

                            ColumnLayout {
                                id: cfgColLeft
                                Layout.fillWidth: true
                                spacing: 1
                                readonly property var cfgFlagLabels: [
                                    "Homing enable", "Home sensor active high", "Enable inversion",
                                    "HLFB inversion", "Position capture active high", "Soft limit enable"
                                ]
                                Repeater {
                                    model: cfgColLeft.cfgFlagLabels.length
                                    delegate: LedRow {
                                        required property int index
                                        ledLabel: cfgColLeft.cfgFlagLabels[index]
                                        ledOn: bitValue(deviceService.monitorData.cfgRegBits, index)
                                    }
                                }
                                RowLayout {
                                    Label {
                                        text: "Soft limit 1:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: intValue(deviceService.monitorData.cfgSoftLimitPos1, "0") }
                                }
                                RowLayout {
                                    Label {
                                        text: "Soft limit 2:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: intValue(deviceService.monitorData.cfgSoftLimitPos2, "0") }
                                }
                                RowLayout {
                                    Label {
                                        text: "+Limit input:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: deviceService.monitorData.cfgPosLimitInput || "NA" }
                                }
                                RowLayout {
                                    Label {
                                        text: "−Limit input:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: deviceService.monitorData.cfgNegLimitInput || "NA" }
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 1
                                RowLayout {
                                    Label {
                                        text: "Home sensor:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: deviceService.monitorData.cfgHomeSensor || "NA" }
                                }
                                RowLayout {
                                    Label {
                                        text: "Brake output:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: deviceService.monitorData.cfgBrakeOutput || "NA" }
                                }
                                RowLayout {
                                    Label {
                                        text: "Pos. capture:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: deviceService.monitorData.cfgPosCapture || "NA" }
                                }
                                RowLayout {
                                    Label {
                                        text: "Max decel rate:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: intValue(deviceService.monitorData.cfgMaxDecelRate, "0") }
                                }
                                RowLayout {
                                    Label {
                                        text: "Stop sensor:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: deviceService.monitorData.cfgStopSensor || "NA" }
                                }
                                RowLayout {
                                    Label {
                                        text: "Follow encoder:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: deviceService.monitorData.cfgFollowEncoder || "NA" }
                                }
                                RowLayout {
                                    Label {
                                        text: "Follow divisor:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: intValue(deviceService.monitorData.cfgFollowDivisor, "0") }
                                }
                                RowLayout {
                                    Label {
                                        text: "Follow multiplier:"
                                        Layout.preferredWidth: regLabelWidth
                                        horizontalAlignment: Text.AlignRight
                                        color: benchUi.labelMuted
                                    }
                                    ValueCell { text: intValue(deviceService.monitorData.cfgFollowMultiplier, "0") }
                                }
                            }
                        }
                    }
                }

                PanelCard {
                    id: operatorControlsCard
                    Layout.preferredWidth: 380
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignTop
                    visible: !isMConnectorModeActive()
                    cardTitle: "Operator controls"

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "Operator writes"
                                color: benchUi.sectionInk
                                font.bold: true
                            }
                            CheckBox {
                                checked: deviceService.operatorControlsEnabled
                                text: checked ? "ON" : "OFF"
                                enabled: deviceService.connected && !externalScannerOwnsIo()
                                onToggled: deviceService.operatorControlsEnabled = checked
                            }
                            Item { Layout.fillWidth: true }
                            Label {
                                text: !deviceService.connected
                                        ? "Disconnected"
                                        : (externalScannerOwnsIo()
                                            ? "Locked (external owner)"
                                            : (deviceService.operatorControlsEnabled ? "Unlocked" : "Locked"))
                                color: (!deviceService.connected || externalScannerOwnsIo() || !deviceService.operatorControlsEnabled)
                                        ? benchUi.warnColor
                                        : benchUi.onColor
                                font.bold: true
                            }
                        }

                        ColumnLayout {
                            id: operatorWriteSection
                            Layout.fillWidth: true
                            spacing: 8
                            enabled: operatorWritesAllowed()

                        Label {
                            text: "Control register · class 0x66 attr 6"
                            font.bold: true
                            color: benchUi.sectionInk
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        GridLayout {
                            columns: 2
                            columnSpacing: 12
                            rowSpacing: 2
                            Layout.fillWidth: true
                            CheckBox {
                                text: "Enable motor"
                                font.pointSize: 9
                                checked: bitValue(deviceService.monitorData.controlRegBits, 0)
                                enabled: deviceService.connected
                                onClicked: deviceService.writeControlBit(0, checked)
                            }
                            CheckBox {
                                text: "Absolute flag"
                                font.pointSize: 9
                                checked: bitValue(deviceService.monitorData.controlRegBits, 1)
                                enabled: deviceService.connected
                                onClicked: deviceService.writeControlBit(1, checked)
                            }
                            CheckBox {
                                text: "Homing move flag"
                                font.pointSize: 9
                                checked: bitValue(deviceService.monitorData.controlRegBits, 2)
                                enabled: deviceService.connected
                                onClicked: deviceService.writeControlBit(2, checked)
                            }
                            CheckBox {
                                text: "Load position move"
                                font.pointSize: 9
                                checked: bitValue(deviceService.monitorData.controlRegBits, 3)
                                enabled: deviceService.connected
                                onClicked: deviceService.writeControlBit(3, checked)
                            }
                            CheckBox {
                                text: "Load velocity move"
                                font.pointSize: 9
                                checked: bitValue(deviceService.monitorData.controlRegBits, 4)
                                enabled: deviceService.connected
                                onClicked: deviceService.writeControlBit(4, checked)
                            }
                            CheckBox {
                                text: "SW E-stop"
                                font.pointSize: 9
                                checked: bitValue(deviceService.monitorData.controlRegBits, 5)
                                enabled: deviceService.connected
                                onClicked: deviceService.writeControlBit(5, checked)
                            }
                            CheckBox {
                                text: "Clear alerts"
                                font.pointSize: 9
                                checked: bitValue(deviceService.monitorData.controlRegBits, 6)
                                enabled: deviceService.connected
                                onClicked: {
                                    if (checked)
                                        clearLoadMoveControlBits()
                                    deviceService.writeControlBit(6, checked)
                                }
                            }
                            CheckBox {
                                text: "Clear motor fault"
                                font.pointSize: 9
                                checked: bitValue(deviceService.monitorData.controlRegBits, 7)
                                enabled: deviceService.connected
                                onClicked: {
                                    if (checked)
                                        clearLoadMoveControlBits()
                                    deviceService.writeControlBit(7, checked)
                                }
                            }
                        }

                        ToolBtn {
                            Layout.fillWidth: true
                            text: "Zero position counter"
                            enabled: deviceService.connected
                            onClicked: deviceService.writeMoveDistance(-parseInt(intValue(deviceService.monitorData.commandedPosition, "0"), 10))
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: benchUi.insetStroke
                        }

                        Label {
                            text: "Velocity moves"
                            font.bold: true
                            color: benchUi.sectionInk
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Label { text: "Jog velocity"; color: benchUi.labelMuted }
                            TextField {
                                id: velocityField
                                text: "0"
                                Layout.fillWidth: true
                                implicitHeight: 28
                                font: benchUi.valueFont
                                selectionColor: "#3d5a80"
                                selectedTextColor: "#ffffff"
                            }
                            ToolBtn {
                                text: "Get"
                                enabled: deviceService.connected
                                onClicked: velocityField.text = intValue(deviceService.monitorData.velocity, "0")
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            ToolBtn {
                                text: "FWD"
                                Layout.fillWidth: true
                                enabled: deviceService.connected
                                onClicked: {
                                    const v = Math.abs(Number(velocityField.text))
                                    deviceService.writeMotorVelocity(v)
                                    deviceService.writeControlBit(4, true)
                                }
                            }
                            ToolBtn {
                                text: "REV"
                                Layout.fillWidth: true
                                enabled: deviceService.connected
                                onClicked: {
                                    const v = -Math.abs(Number(velocityField.text))
                                    deviceService.writeMotorVelocity(v)
                                    deviceService.writeControlBit(4, true)
                                }
                            }
                            ToolBtn {
                                text: "Stop"
                                Layout.fillWidth: true
                                enabled: deviceService.connected
                                onClicked: {
                                    deviceService.writeControlBit(4, false)
                                    deviceService.writeMotorVelocity(0)
                                }
                            }
                        }

                        Label {
                            text: "Common settings"
                            font.bold: true
                            color: benchUi.sectionInk
                        }
                        GridLayout {
                            columns: 4
                            columnSpacing: 8
                            rowSpacing: 6
                            Layout.fillWidth: true
                            Label { text: "Accel"; color: benchUi.labelMuted }
                            TextField {
                                id: accelField
                                text: "0"
                                implicitHeight: 28
                                font: benchUi.valueFont
                                Layout.fillWidth: true
                                selectionColor: "#3d5a80"
                                selectedTextColor: "#ffffff"
                            }
                            ToolBtn {
                                text: "Set"
                                enabled: deviceService.connected
                                onClicked: deviceService.writeAccel(Number(accelField.text))
                            }
                            ToolBtn {
                                text: "Get"
                                enabled: deviceService.connected
                                onClicked: accelField.text = intValue(deviceService.monitorData.accel, "0")
                            }
                            Label { text: "Decel"; color: benchUi.labelMuted }
                            TextField {
                                id: decelField
                                text: "0"
                                implicitHeight: 28
                                font: benchUi.valueFont
                                Layout.fillWidth: true
                                selectionColor: "#3d5a80"
                                selectedTextColor: "#ffffff"
                            }
                            ToolBtn {
                                text: "Set"
                                enabled: deviceService.connected
                                onClicked: deviceService.writeDecel(Number(decelField.text))
                            }
                            ToolBtn {
                                text: "Get"
                                enabled: deviceService.connected
                                onClicked: decelField.text = intValue(deviceService.monitorData.decel, "0")
                            }
                        }

                        Label {
                            text: "Positional moves"
                            font.bold: true
                            color: benchUi.sectionInk
                        }
                        GridLayout {
                            columns: 4
                            columnSpacing: 8
                            rowSpacing: 6
                            Layout.fillWidth: true
                            Label { text: "Move dist"; color: benchUi.labelMuted }
                            TextField {
                                id: distanceField
                                text: "0"
                                implicitHeight: 28
                                font: benchUi.valueFont
                                Layout.fillWidth: true
                                selectionColor: "#3d5a80"
                                selectedTextColor: "#ffffff"
                            }
                            ToolBtn {
                                text: "Move"
                                enabled: deviceService.connected
                                onClicked: deviceService.commandPositionalMove(Number(distanceField.text))
                            }
                            ToolBtn {
                                text: "Stop"
                                enabled: deviceService.connected
                                onClicked: deviceService.commandStopPositionalMove()
                            }
                            Label { text: "Move vel"; color: benchUi.labelMuted }
                            TextField {
                                id: moveVelocityField
                                text: "0"
                                implicitHeight: 28
                                font: benchUi.valueFont
                                Layout.fillWidth: true
                                selectionColor: "#3d5a80"
                                selectedTextColor: "#ffffff"
                            }
                            ToolBtn {
                                text: "Set"
                                enabled: deviceService.connected
                                onClicked: deviceService.writeMoveVelocity(Number(moveVelocityField.text))
                            }
                            ToolBtn {
                                text: "Get"
                                enabled: deviceService.connected
                                onClicked: moveVelocityField.text = intValue(deviceService.monitorData.moveVelocity, "0")
                            }
                        }
                        ToolBtn {
                            Layout.fillWidth: true
                            text: "Get move distance"
                            enabled: deviceService.connected
                            onClicked: distanceField.text = intValue(deviceService.monitorData.moveDistance, "0")
                        }

                        Label {
                            text: "Digital outputs"
                            font.bold: true
                            color: benchUi.sectionInk
                        }
                        GridLayout {
                            columns: 2
                            columnSpacing: 8
                            rowSpacing: 2
                            Layout.fillWidth: true
                            Repeater {
                                model: 6
                                delegate: CheckBox {
                                    required property int index
                                    text: "I/O-" + index
                                    font.pointSize: 9
                                    enabled: operatorWritesAllowed()
                                    onToggled: deviceService.writeDigitalOutput(index + 1, checked)
                                }
                            }
                        }

                        Label {
                            text: "CCIO outputs"
                            font.bold: true
                            color: benchUi.sectionInk
                        }
                        Label {
                            Layout.fillWidth: true
                            visible: !(deviceService.connected && deviceService.monitorData.ccioEnabled)
                            text: !deviceService.connected
                                  ? "Connect to enable CCIO outputs."
                                  : "Enable CCIO to control CCIO outputs."
                            color: benchUi.labelMuted
                            font.pointSize: 8
                            wrapMode: Text.WordWrap
                        }
                        GridLayout {
                            columns: 9
                            columnSpacing: 4
                            rowSpacing: 2
                            Layout.fillWidth: true
                            enabled: operatorWritesAllowed() && deviceService.monitorData.ccioEnabled
                            opacity: enabled ? 1.0 : 0.45

                            Label { text: ""; Layout.preferredWidth: 44 }
                            Repeater {
                                model: 8
                                delegate: Label {
                                    required property int index
                                    text: index.toString()
                                    font.family: benchUi.monoFamily
                                    font.pointSize: 8
                                    color: benchUi.labelMuted
                                    horizontalAlignment: Text.AlignHCenter
                                    Layout.preferredWidth: 20
                                }
                            }

                            Repeater {
                                model: detectedCcioBoardCount()
                                delegate: RowLayout {
                                    id: ccioOutputRow
                                    required property int index
                                    Layout.columnSpan: 9
                                    spacing: 4

                                    Label {
                                        text: "B-" + (index + 1)
                                        font.family: benchUi.monoFamily
                                        font.pointSize: 8
                                        color: benchUi.labelMuted
                                        Layout.preferredWidth: 44
                                    }

                                    Repeater {
                                        model: 8
                                        delegate: CheckBox {
                                            required property int index
                                            Layout.preferredWidth: 20
                                            enabled: operatorWritesAllowed()
                                                     && deviceService.monitorData.ccioEnabled
                                            onToggled: deviceService.writeCcioOutputBit(ccioOutputRow.index + 1, index, checked)
                                        }
                                    }
                                }
                            }
                        }
                        }
                    }
                }

                PanelCard {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignTop
                    visible: isMConnectorModeActive()
                    cardTitle: "MConnector Data"

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 3

                        Repeater {
                            model: mconnectorStatusLabels.length
                            delegate: LedRow {
                                required property int index
                                ledLabel: mconnectorStatusLabels[index]
                                ledLabelWidth: 220
                                ledOn: bitValue(deviceService.monitorData.mconnectorStatusBits, index)
                            }
                        }

                        Repeater {
                            model: mconnectorOutputLabels.length
                            delegate: LedRow {
                                required property int index
                                ledLabel: mconnectorOutputLabels[index]
                                ledLabelWidth: 220
                                ledOn: bitValue(deviceService.monitorData.mconnectorOutputBits, index)
                            }
                        }

                        RowLayout {
                            Label {
                                text: "HLFB Duty:"
                                Layout.preferredWidth: 220
                                horizontalAlignment: Text.AlignRight
                                color: benchUi.labelMuted
                            }
                            ValueCell {
                                Layout.preferredWidth: 120
                                text: formatTorque(deviceService.monitorData.mconnectorHlfbDuty)
                            }
                            Item { Layout.fillWidth: true }
                        }

                        RowLayout {
                            Label {
                                text: "A PWM:"
                                Layout.preferredWidth: 220
                                horizontalAlignment: Text.AlignRight
                                color: benchUi.labelMuted
                            }
                            ValueCell { Layout.preferredWidth: 120; text: intValue(deviceService.monitorData.mconnectorPwmA, "0") }
                            Item { Layout.fillWidth: true }
                        }

                        RowLayout {
                            Label {
                                text: "B PWM:"
                                Layout.preferredWidth: 220
                                horizontalAlignment: Text.AlignRight
                                color: benchUi.labelMuted
                            }
                            ValueCell { Layout.preferredWidth: 120; text: intValue(deviceService.monitorData.mconnectorPwmB, "0") }
                            Item { Layout.fillWidth: true }
                        }

                        RowLayout {
                            Label {
                                text: "Trigger Pulses:"
                                Layout.preferredWidth: 220
                                horizontalAlignment: Text.AlignRight
                                color: benchUi.labelMuted
                            }
                            ValueCell { Layout.preferredWidth: 120; text: intValue(deviceService.monitorData.mconnectorTriggerPulses, "0") }
                            Item { Layout.fillWidth: true }
                        }

                        RowLayout {
                            Label {
                                text: "Trigger Pulse Time:"
                                Layout.preferredWidth: 220
                                horizontalAlignment: Text.AlignRight
                                color: benchUi.labelMuted
                            }
                            ValueCell { Layout.preferredWidth: 120; text: intValue(deviceService.monitorData.mconnectorTriggerPulseTime, "0") }
                            Item { Layout.fillWidth: true }
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                PanelCard {
                    Layout.preferredWidth: 380
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignTop
                    visible: isMConnectorModeActive()
                    cardTitle: "MConnector operator controls"

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "Operator writes"
                                color: benchUi.sectionInk
                                font.bold: true
                            }
                            CheckBox {
                                checked: deviceService.operatorControlsEnabled
                                text: checked ? "ON" : "OFF"
                                enabled: deviceService.connected && !externalScannerOwnsIo()
                                onToggled: deviceService.operatorControlsEnabled = checked
                            }
                            Item { Layout.fillWidth: true }
                            Label {
                                text: !deviceService.connected
                                      ? "Disconnected"
                                      : (externalScannerOwnsIo()
                                          ? "Locked (external owner)"
                                          : (deviceService.operatorControlsEnabled ? "Unlocked" : "Locked"))
                                color: (!deviceService.connected || externalScannerOwnsIo() || !deviceService.operatorControlsEnabled)
                                       ? benchUi.warnColor
                                       : benchUi.onColor
                                font.bold: true
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            enabled: operatorWritesAllowed()

                            Label {
                                text: "Output register · class 0x67 attr 1"
                                font.bold: true
                                color: benchUi.sectionInk
                                Layout.fillWidth: true
                            }
                            GridLayout {
                                columns: 2
                                columnSpacing: 10
                                rowSpacing: 2
                                Layout.fillWidth: true

                                CheckBox {
                                    text: "Enabled"
                                    checked: bitValue(deviceService.monitorData.mconnectorOutputBits, 0)
                                    onClicked: deviceService.writeMConnectorOutputBit(0, checked)
                                }
                                CheckBox {
                                    text: "Output A"
                                    checked: bitValue(deviceService.monitorData.mconnectorOutputBits, 1)
                                    onClicked: deviceService.writeMConnectorOutputBit(1, checked)
                                }
                                CheckBox {
                                    text: "Output B"
                                    checked: bitValue(deviceService.monitorData.mconnectorOutputBits, 2)
                                    onClicked: deviceService.writeMConnectorOutputBit(2, checked)
                                }
                                CheckBox {
                                    text: "Disable Pulse"
                                    checked: bitValue(deviceService.monitorData.mconnectorOutputBits, 3)
                                    onClicked: deviceService.writeMConnectorOutputBit(3, checked)
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 1
                                color: benchUi.insetStroke
                            }

                            Label {
                                text: "Trigger pulses · class 0x67 attr 7"
                                font.bold: true
                                color: benchUi.sectionInk
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "Pulses"; color: benchUi.labelMuted }
                                TextField {
                                    id: mcTriggerPulsesField
                                    text: "0"
                                    Layout.fillWidth: true
                                    implicitHeight: 28
                                    font: benchUi.valueFont
                                    selectionColor: "#3d5a80"
                                    selectedTextColor: "#ffffff"
                                }
                                ToolBtn {
                                    text: "Set"
                                    enabled: deviceService.connected
                                    onClicked: deviceService.writeMConnectorTriggerPulses(parseCfgInt(mcTriggerPulsesField.text, 0))
                                }
                                ToolBtn {
                                    text: "Get"
                                    enabled: deviceService.connected
                                    onClicked: mcTriggerPulsesField.text = intValue(deviceService.monitorData.mconnectorTriggerPulses, "0")
                                }
                            }

                            Label {
                                text: "PWM outputs · class 0x67 attrs 8/9"
                                font.bold: true
                                color: benchUi.sectionInk
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "PWM A"; color: benchUi.labelMuted }
                                TextField {
                                    id: mcPwmAField
                                    text: "0"
                                    Layout.fillWidth: true
                                    implicitHeight: 28
                                    font: benchUi.valueFont
                                    selectionColor: "#3d5a80"
                                    selectedTextColor: "#ffffff"
                                }
                                ToolBtn {
                                    text: "Set"
                                    enabled: deviceService.connected
                                    onClicked: deviceService.writeMConnectorPwmA(parseCfgInt(mcPwmAField.text, 0))
                                }
                                ToolBtn {
                                    text: "Get"
                                    enabled: deviceService.connected
                                    onClicked: mcPwmAField.text = intValue(deviceService.monitorData.mconnectorPwmA, "0")
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "PWM B"; color: benchUi.labelMuted }
                                TextField {
                                    id: mcPwmBField
                                    text: "0"
                                    Layout.fillWidth: true
                                    implicitHeight: 28
                                    font: benchUi.valueFont
                                    selectionColor: "#3d5a80"
                                    selectedTextColor: "#ffffff"
                                }
                                ToolBtn {
                                    text: "Set"
                                    enabled: deviceService.connected
                                    onClicked: deviceService.writeMConnectorPwmB(parseCfgInt(mcPwmBField.text, 0))
                                }
                                ToolBtn {
                                    text: "Get"
                                    enabled: deviceService.connected
                                    onClicked: mcPwmBField.text = intValue(deviceService.monitorData.mconnectorPwmB, "0")
                                }
                            }

                            Label {
                                text: "Digital outputs"
                                font.bold: true
                                color: benchUi.sectionInk
                            }
                            GridLayout {
                                columns: 2
                                columnSpacing: 8
                                rowSpacing: 2
                                Layout.fillWidth: true
                                Repeater {
                                    model: 6
                                    delegate: CheckBox {
                                        required property int index
                                        text: "I/O-" + index
                                        font.pointSize: 9
                                        enabled: operatorWritesAllowed()
                                        onToggled: deviceService.writeDigitalOutput(index + 1, checked)
                                    }
                                }
                            }

                            Label {
                                text: "CCIO outputs"
                                font.bold: true
                                color: benchUi.sectionInk
                            }
                            Label {
                                Layout.fillWidth: true
                                visible: !(deviceService.connected && deviceService.monitorData.ccioEnabled)
                                text: !deviceService.connected
                                      ? "Connect to enable CCIO outputs."
                                      : "Enable CCIO to control CCIO outputs."
                                color: benchUi.labelMuted
                                font.pointSize: 8
                                wrapMode: Text.WordWrap
                            }
                            GridLayout {
                                columns: 9
                                columnSpacing: 4
                                rowSpacing: 2
                                Layout.fillWidth: true
                                enabled: operatorWritesAllowed() && deviceService.monitorData.ccioEnabled
                                opacity: enabled ? 1.0 : 0.45

                                Label { text: ""; Layout.preferredWidth: 44 }
                                Repeater {
                                    model: 8
                                    delegate: Label {
                                        required property int index
                                        text: index.toString()
                                        font.family: benchUi.monoFamily
                                        font.pointSize: 8
                                        color: benchUi.labelMuted
                                        horizontalAlignment: Text.AlignHCenter
                                        Layout.preferredWidth: 20
                                    }
                                }

                                Repeater {
                                    model: detectedCcioBoardCount()
                                    delegate: RowLayout {
                                        id: mcCcioOutputRow
                                        required property int index
                                        Layout.columnSpan: 9
                                        spacing: 4

                                        Label {
                                            text: "B-" + (index + 1)
                                            font.family: benchUi.monoFamily
                                            font.pointSize: 8
                                            color: benchUi.labelMuted
                                            Layout.preferredWidth: 44
                                        }

                                        Repeater {
                                            model: 8
                                            delegate: CheckBox {
                                                required property int index
                                                Layout.preferredWidth: 20
                                                enabled: operatorWritesAllowed()
                                                         && deviceService.monitorData.ccioEnabled
                                                onToggled: deviceService.writeCcioOutputBit(mcCcioOutputRow.index + 1, index, checked)
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }
                }
            }
        }
    }

    Dialog {
        id: motorConfigDialog
        modal: true
        width: 720
        height: 560
        title: (isMConnectorModeActive()
                ? ("Edit M-connector motor " + (deviceService.selectedMotorInstance - 1) + " configuration")
                : ("Edit motor " + (deviceService.selectedMotorInstance - 1) + " configuration"))
        standardButtons: Dialog.Save | Dialog.Cancel
        onAccepted: saveMotorConfigDialog()

        contentItem: ScrollView {
            clip: true
            ColumnLayout {
                width: parent.width
                spacing: 10

                Label {
                    visible: !isMConnectorModeActive()
                    text: "Config register flags"
                    font.bold: true
                    color: benchUi.sectionInk
                }
                GridLayout {
                    visible: !isMConnectorModeActive()
                    columns: 2
                    columnSpacing: 10
                    rowSpacing: 4
                    Layout.fillWidth: true

                    CheckBox { id: cfgEditHomingEnable; text: "Homing enable" }
                    CheckBox { id: cfgEditHomeSensorActiveHigh; text: "Home sensor active high" }
                    CheckBox { id: cfgEditEnableInversion; text: "Enable inversion" }
                    CheckBox { id: cfgEditHlfbInversion; text: "HLFB inversion" }
                    CheckBox { id: cfgEditPosCaptureActiveHigh; text: "Position capture active high" }
                    CheckBox { id: cfgEditSoftLimitEnable; text: "Soft limit enable" }
                }

                Rectangle {
                    visible: !isMConnectorModeActive()
                    Layout.fillWidth: true
                    height: 1
                    color: benchUi.insetStroke
                }

                Label {
                    visible: !isMConnectorModeActive()
                    text: "Numeric/config fields"
                    font.bold: true
                    color: benchUi.sectionInk
                }
                GridLayout {
                    visible: !isMConnectorModeActive()
                    columns: 2
                    columnSpacing: 14
                    rowSpacing: 6
                    Layout.fillWidth: true

                    Label { text: "Soft limit 1"; color: benchUi.labelMuted }
                    TextField { id: cfgEditSoftLimit1; font: benchUi.valueFont; Layout.fillWidth: true }
                    Label { text: "Soft limit 2"; color: benchUi.labelMuted }
                    TextField { id: cfgEditSoftLimit2; font: benchUi.valueFont; Layout.fillWidth: true }
                    Label { text: "Positive limit input"; color: benchUi.labelMuted }
                    TextField { id: cfgEditPosLimitInput; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "NA or -1" }
                    Label { text: "Negative limit input"; color: benchUi.labelMuted }
                    TextField { id: cfgEditNegLimitInput; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "NA or -1" }
                    Label { text: "Home sensor"; color: benchUi.labelMuted }
                    TextField { id: cfgEditHomeSensor; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "NA or -1" }
                    Label { text: "Brake output"; color: benchUi.labelMuted }
                    TextField { id: cfgEditBrakeOutput; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "NA or -1" }
                    Label { text: "Position capture"; color: benchUi.labelMuted }
                    TextField { id: cfgEditPosCapture; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "NA or -1" }
                    Label { text: "Max decel rate"; color: benchUi.labelMuted }
                    TextField { id: cfgEditMaxDecelRate; font: benchUi.valueFont; Layout.fillWidth: true }
                    Label { text: "Stop sensor"; color: benchUi.labelMuted }
                    TextField { id: cfgEditStopSensor; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "NA or -1" }
                    Label { text: "Follow encoder"; color: benchUi.labelMuted }
                    TextField { id: cfgEditFollowEncoder; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "NA or -1" }
                    Label { text: "Follow divisor"; color: benchUi.labelMuted }
                    TextField { id: cfgEditFollowDivisor; font: benchUi.valueFont; Layout.fillWidth: true }
                    Label { text: "Follow multiplier"; color: benchUi.labelMuted }
                    TextField { id: cfgEditFollowMultiplier; font: benchUi.valueFont; Layout.fillWidth: true }
                }

                Label {
                    visible: isMConnectorModeActive()
                    text: "M-connector routing/config fields"
                    font.bold: true
                    color: benchUi.sectionInk
                }
                GridLayout {
                    visible: isMConnectorModeActive()
                    columns: 2
                    columnSpacing: 14
                    rowSpacing: 6
                    Layout.fillWidth: true

                    Label { text: "Enable connector"; color: benchUi.labelMuted }
                    TextField { id: cfgEditMcEnableConnector; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "-1 for disabled" }
                    Label { text: "Input A connector"; color: benchUi.labelMuted }
                    TextField { id: cfgEditMcInputAConnector; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "-1 for disabled" }
                    Label { text: "Input B connector"; color: benchUi.labelMuted }
                    TextField { id: cfgEditMcInputBConnector; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "-1 for disabled" }
                    Label { text: "Trigger pulse time (ms)"; color: benchUi.labelMuted }
                    TextField { id: cfgEditMcTriggerPulseTime; font: benchUi.valueFont; Layout.fillWidth: true; placeholderText: "0..65535" }
                }
            }
        }
    }
}
