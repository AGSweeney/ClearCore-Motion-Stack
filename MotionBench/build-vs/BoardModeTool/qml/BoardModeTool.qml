import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 560
    height: 220
    minimumWidth: 520
    minimumHeight: 200
    visible: true
    title: "Board Mode Tool"
    color: benchUi.panelBgOuter
    font.family: "Segoe UI"
    font.pointSize: 9

    QtObject {
        id: benchUi
        readonly property color panelBgOuter: "#d8dce2"
        readonly property color panelBg: "#f4f6f8"
        readonly property color panelStroke: "#8b96a8"
        readonly property color insetBg: "#e8ecf0"
        readonly property color insetStroke: "#bdc6d2"
        readonly property color labelMuted: "#5a6b7f"
        readonly property color valueInk: "#0f1419"
        readonly property color sectionInk: "#2d3f56"
        readonly property color onColor: "#0d9d61"
        readonly property color warnColor: "#c47f00"
        readonly property color errorColor: "#b44a4a"
    }

    // Remove system accent (purple on Windows) and align with MotionBench styling.
    palette.highlight: "#3d5a80"
    palette.highlightedText: "#ffffff"
    palette.base: "#ffffff"
    palette.text: benchUi.valueInk
    palette.button: "#f4f6f8"
    palette.buttonText: benchUi.valueInk

    Connections {
        target: boardModeTool
        function onTargetIpChanged() {
            if (targetIpCombo.editText !== boardModeTool.targetIp) {
                targetIpCombo.editText = boardModeTool.targetIp
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 4
            color: benchUi.panelBg
            border.color: benchUi.panelStroke
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: "Target IP"
                        color: benchUi.sectionInk
                        font.bold: true
                    }

                    ComboBox {
                        id: targetIpCombo
                        Layout.fillWidth: true
                        editable: true
                        model: boardModeTool.discoveredIps
                        Component.onCompleted: editText = boardModeTool.targetIp
                        onAccepted: boardModeTool.targetIp = editText
                        onActivated: {
                            if (currentIndex >= 0) {
                                const ip = model[currentIndex]
                                editText = ip
                                boardModeTool.targetIp = ip
                            }
                        }
                    }

                    Button {
                        text: "Discover"
                        onClicked: boardModeTool.discover()
                    }

                    Button {
                        text: boardModeTool.connected ? "Disconnect" : "Connect"
                        onClicked: {
                            if (boardModeTool.connected) {
                                boardModeTool.disconnectDevice()
                            } else {
                                boardModeTool.targetIp = targetIpCombo.editText
                                boardModeTool.connectDevice()
                            }
                        }
                    }

                    Button {
                        text: "Refresh"
                        enabled: boardModeTool.connected
                        onClicked: boardModeTool.refreshBoardMode()
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: benchUi.insetStroke
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Label {
                        text: "M-connector board mode"
                        font.pointSize: 11
                        color: benchUi.sectionInk
                        font.bold: true
                    }

                    Switch {
                        id: boardModeSwitch
                        enabled: boardModeTool.connected
                        checked: boardModeTool.boardModeMConnector
                        onToggled: {
                            if (checked !== boardModeTool.boardModeMConnector) {
                                if (!boardModeTool.setBoardModeMConnector(checked)) {
                                    boardModeTool.refreshBoardMode()
                                }
                            }
                        }
                    }

                    Label {
                        text: boardModeTool.connected ? "Connected" : "Disconnected"
                        color: boardModeTool.connected ? benchUi.onColor : benchUi.warnColor
                    }

                    Item { Layout.fillWidth: true }
                }

                Label {
                    Layout.fillWidth: true
                    visible: boardModeTool.lastError.length > 0
                    color: benchUi.errorColor
                    wrapMode: Text.WordWrap
                    text: boardModeTool.lastError
                }

                Item { Layout.fillHeight: true }
            }
        }
    }
}
