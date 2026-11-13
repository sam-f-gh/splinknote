import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: shelf
    width: 50
    height: contentColumn.implicitHeight + 24

    Rectangle {
        anchors.fill: parent
        color: theme.darkBackground
        radius: Math.min(theme.radius, 14)
        border.color: theme.lighterBackground
        border.width: 1
        opacity: 0.95

        // Subtle drop shadow
        Rectangle {
            z: -1
            anchors.fill: parent
            anchors.margins: -1
            radius: parent.radius + 1
            color: Qt.rgba(0, 0, 0, 0.25)
        }

        // Main shelf options column
        ColumnLayout {
            id: contentColumn
            anchors.centerIn: parent
            spacing: 8

            // 1. PEN TOOL SHELF
            ColumnLayout {
                spacing: 8
                visible: backend.currentTool === 0

                // Color Swatches (Black is the default primary color)
                Repeater {
                    model: [
                        { color: "#18181c" }, // True ink black default
                        { color: theme.accent },
                        { color: "#f7768e" },
                        { color: "#9ece6a" },
                        { color: "#7dcfff" }
                    ]
                    delegate: Rectangle {
                        id: penColorItem
                        required property var modelData
                        width: 36
                        height: 36
                        radius: 18
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 22
                            height: 22
                            radius: 11
                            color: penColorItem.modelData.color
                            border.color: Qt.colorEqual(backend.penColor, penColorItem.modelData.color)
                                          ? theme.foreground
                                          : theme.lighterBackground
                            border.width: Qt.colorEqual(backend.penColor, penColorItem.modelData.color) ? 2.5 : 1

                            scale: swatchMouse.containsMouse ? 1.15 : 1.0
                            Behavior on scale { NumberAnimation { duration: 100 } }
                        }

                        MouseArea {
                            id: swatchMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.setPenColor(penColorItem.modelData.color)
                            onDoubleClicked: {
                                colorPicker.x = shelf.width + 6
                                colorPicker.y = Math.max(0, penColorItem.mapToItem(shelf, 0, 0).y - 40)
                                colorPicker.open()
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 24
                    height: 1
                    color: theme.lighterBackground
                }

                // Pen Width Presets (Fine 1.5, Medium 2.5, Bold 5.0)
                Repeater {
                    model: [
                        { size: 1.5, dot: 4, label: "F" },
                        { size: 2.5, dot: 7, label: "M" },
                        { size: 5.0, dot: 11, label: "B" }
                    ]
                    delegate: Rectangle {
                        id: penSizeItem
                        required property var modelData
                        width: 36
                        height: 36
                        radius: 8
                        color: Math.abs(backend.penWidth - penSizeItem.modelData.size) < 0.2
                               ? theme.lighterBackground
                               : (penWidthMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.05) : "transparent")

                        Rectangle {
                            anchors.centerIn: parent
                            width: penSizeItem.modelData.dot
                            height: penSizeItem.modelData.dot
                            radius: penSizeItem.modelData.dot / 2
                            color: backend.penColor
                        }

                        MouseArea {
                            id: penWidthMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.setPenWidth(penSizeItem.modelData.size)
                        }
                    }
                }
            }

            // 2. HIGHLIGHTER TOOL SHELF
            ColumnLayout {
                spacing: 8
                visible: backend.currentTool === 1

                // Highlighter Colors
                Repeater {
                    model: [
                        { color: "#FFD60A" }, // Yellow
                        { color: "#70C0E8" }, // Cyan
                        { color: "#F7768E" }, // Rose Pink
                        { color: "#73DACA" }  // Mint Green
                    ]
                    delegate: Rectangle {
                        id: hlColorItem
                        required property var modelData
                        width: 36
                        height: 36
                        radius: 18
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            radius: 12
                            color: hlColorItem.modelData.color
                            border.color: Qt.colorEqual(backend.highlighterColor, hlColorItem.modelData.color)
                                          ? theme.foreground
                                          : theme.lighterBackground
                            border.width: Qt.colorEqual(backend.highlighterColor, hlColorItem.modelData.color) ? 2.5 : 1

                            scale: hlMouse.containsMouse ? 1.15 : 1.0
                            Behavior on scale { NumberAnimation { duration: 100 } }
                        }

                        MouseArea {
                            id: hlMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.setHighlighterColor(hlColorItem.modelData.color)
                            onDoubleClicked: {
                                colorPicker.x = shelf.width + 6
                                colorPicker.y = Math.max(0, hlColorItem.mapToItem(shelf, 0, 0).y - 40)
                                colorPicker.open()
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 24
                    height: 1
                    color: theme.lighterBackground
                }

                // Highlighter Width Presets (14pt, 22pt, 32pt)
                Repeater {
                    model: [
                        { size: 14.0, h: 4, label: "M" },
                        { size: 22.0, h: 7, label: "W" },
                        { size: 32.0, h: 11, label: "X" }
                    ]
                    delegate: Rectangle {
                        id: hlSizeItem
                        required property var modelData
                        width: 36
                        height: 36
                        radius: 8
                        color: Math.abs(backend.highlighterWidth - hlSizeItem.modelData.size) < 1.0
                               ? theme.lighterBackground
                               : (hlWidthMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.05) : "transparent")

                        Rectangle {
                            anchors.centerIn: parent
                            width: 18
                            height: hlSizeItem.modelData.h
                            radius: 2
                            color: backend.highlighterColor
                            opacity: 0.8
                        }

                        MouseArea {
                            id: hlWidthMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.setHighlighterWidth(hlSizeItem.modelData.size)
                        }
                    }
                }
            }

            // 3. ERASER TOOL SHELF (Clear Page Action, no redundant size dot)
            ColumnLayout {
                spacing: 8
                visible: backend.currentTool === 2

                Rectangle {
                    width: 36
                    height: 36
                    radius: 8
                    color: clearMouse.containsMouse ? Qt.rgba(0.9, 0.2, 0.2, 0.2) : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "󰇚"
                        font.family: "JetBrainsMono Nerd Font"
                        font.pixelSize: 18
                        color: clearMouse.containsMouse ? "#f7768e" : theme.foreground
                    }

                    ToolTip.visible: clearMouse.containsMouse && !Qt.isTouchScreen
                    ToolTip.text: qsTr("Clear Page")

                    MouseArea {
                        id: clearMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            confirmDialog.ask(qsTr("Clear all strokes on this page?"), function() {
                                backend.clearCurrentPage()
                            })
                        }
                    }
                }
            }
        }
    }

    // Floating Word-Style Grid Color Picker Popup
    ColorGridPicker {
        id: colorPicker
        onColorPicked: function(chosenColor) {
            if (backend.currentTool === 1) {
                backend.setHighlighterColor(chosenColor)
            } else {
                backend.setPenColor(chosenColor)
            }
        }
    }
}
