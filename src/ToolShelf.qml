import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: shelf
    width: 50
    height: contentColumn.implicitHeight + 24

    property var penColors: ["#18181c", theme.accent, "#f7768e", "#9ece6a", "#7dcfff"]
    property var highlighterColors: ["#FFD60A", "#70C0E8", "#F7768E", "#73DACA"]
    property int editingSwatchIndex: -1
    property int editingSwatchTool: -1

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
                    model: shelf.penColors
                    delegate: Rectangle {
                        id: penColorItem
                        required property var modelData
                        required property int index
                        width: 36
                        height: 36
                        radius: 18
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 22
                            height: 22
                            radius: 11
                            color: penColorItem.modelData
                            border.color: Qt.colorEqual(backend.penColor, penColorItem.modelData)
                                          ? theme.foreground
                                          : theme.lighterBackground
                            border.width: Qt.colorEqual(backend.penColor, penColorItem.modelData) ? 2.5 : 1

                            scale: swatchMouse.containsMouse ? 1.15 : 1.0
                            Behavior on scale { NumberAnimation { duration: 100 } }
                        }

                        MouseArea {
                            id: swatchMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.setPenColor(penColorItem.modelData)
                            onDoubleClicked: {
                                shelf.editingSwatchIndex = penColorItem.index
                                shelf.editingSwatchTool = 0
                                var globalPos = penColorItem.mapToItem(null, 0, 0)
                                colorPicker.x = globalPos.x + penColorItem.width + 12
                                colorPicker.y = Math.max(12, globalPos.y - 40)
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
                    model: shelf.highlighterColors
                    delegate: Rectangle {
                        id: hlColorItem
                        required property var modelData
                        required property int index
                        width: 36
                        height: 36
                        radius: 18
                        color: "transparent"

                        Rectangle {
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            radius: 12
                            color: hlColorItem.modelData
                            border.color: Qt.colorEqual(backend.highlighterColor, hlColorItem.modelData)
                                          ? theme.foreground
                                          : theme.lighterBackground
                            border.width: Qt.colorEqual(backend.highlighterColor, hlColorItem.modelData) ? 2.5 : 1

                            scale: hlMouse.containsMouse ? 1.15 : 1.0
                            Behavior on scale { NumberAnimation { duration: 100 } }
                        }

                        MouseArea {
                            id: hlMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.setHighlighterColor(hlColorItem.modelData)
                            onDoubleClicked: {
                                shelf.editingSwatchIndex = hlColorItem.index
                                shelf.editingSwatchTool = 1
                                var globalPos = hlColorItem.mapToItem(null, 0, 0)
                                colorPicker.x = globalPos.x + hlColorItem.width + 12
                                colorPicker.y = Math.max(12, globalPos.y - 40)
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
                        text: "\uf1f8"
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
        onClosed: {
            shelf.editingSwatchIndex = -1
            shelf.editingSwatchTool = -1
        }
        onColorPicked: function(chosenColor) {
            if (shelf.editingSwatchTool === 0 && shelf.editingSwatchIndex >= 0 && shelf.editingSwatchIndex < shelf.penColors.length) {
                var p = shelf.penColors.slice()
                p[shelf.editingSwatchIndex] = chosenColor
                shelf.penColors = p
                backend.setPenColor(chosenColor)
            } else if (shelf.editingSwatchTool === 1 && shelf.editingSwatchIndex >= 0 && shelf.editingSwatchIndex < shelf.highlighterColors.length) {
                var h = shelf.highlighterColors.slice()
                h[shelf.editingSwatchIndex] = chosenColor
                shelf.highlighterColors = h
                backend.setHighlighterColor(chosenColor)
            } else {
                if (backend.currentTool === 1) {
                    backend.setHighlighterColor(chosenColor)
                } else {
                    backend.setPenColor(chosenColor)
                }
            }
            shelf.editingSwatchIndex = -1
            shelf.editingSwatchTool = -1
        }
    }
}
