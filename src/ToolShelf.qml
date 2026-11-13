import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: shelf
    width: collapsed ? 24 : 52
    height: contentColumn.implicitHeight + 28

    property bool collapsed: false

    Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }

    Rectangle {
        anchors.fill: parent
        color: theme.darkBackground
        radius: Math.min(theme.radius, 16)
        border.color: theme.lighterBackground
        border.width: 1
        opacity: 0.95

        // Subtle shadow
        Rectangle {
            z: -1
            anchors.fill: parent
            anchors.margins: -1
            radius: parent.radius + 1
            color: Qt.rgba(0, 0, 0, 0.25)
        }

        // Toggle expand/collapse button
        Rectangle {
            id: toggleBtn
            anchors.top: parent.top
            anchors.topMargin: 4
            anchors.horizontalCenter: parent.horizontalCenter
            width: 32
            height: 20
            radius: 4
            color: toggleArea.containsMouse ? theme.lighterBackground : "transparent"

            Text {
                anchors.centerIn: parent
                text: shelf.collapsed ? "›" : "‹"
                color: theme.foreground
                font.family: "JetBrainsMono Nerd Font"
                font.pixelSize: 13
                font.bold: true
            }

            MouseArea {
                id: toggleArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: shelf.collapsed = !shelf.collapsed
            }
        }

        // Main shelf options column
        ColumnLayout {
            id: contentColumn
            anchors.top: toggleBtn.bottom
            anchors.topMargin: 6
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8
            visible: !shelf.collapsed

            // 1. PEN TOOL SHELF
            ColumnLayout {
                spacing: 8
                visible: backend.currentTool === 0

                // Color Swatches
                Repeater {
                    model: [
                        { name: "accent", color: theme.accent },
                        { name: "dark", color: theme.light ? "#18181c" : "#c0caf5" },
                        { name: "red", color: "#f7768e" },
                        { name: "green", color: "#9ece6a" },
                        { name: "cyan", color: "#7dcfff" }
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

            // 3. ERASER TOOL SHELF
            ColumnLayout {
                spacing: 8
                visible: backend.currentTool === 2

                // Eraser Radius Presets (Fine 10, Medium 18, Broad 32)
                Repeater {
                    model: [
                        { radius: 10.0, ring: 12, label: "S" },
                        { radius: 18.0, ring: 18, label: "M" },
                        { radius: 32.0, ring: 24, label: "L" }
                    ]
                    delegate: Rectangle {
                        id: eraserSizeItem
                        required property var modelData
                        width: 36
                        height: 36
                        radius: 8
                        color: Math.abs(backend.eraserRadius - eraserSizeItem.modelData.radius) < 1.0
                               ? theme.lighterBackground
                               : (eraserSizeMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.05) : "transparent")

                        Rectangle {
                            anchors.centerIn: parent
                            width: eraserSizeItem.modelData.ring
                            height: eraserSizeItem.modelData.ring
                            radius: eraserSizeItem.modelData.ring / 2
                            color: "transparent"
                            border.color: theme.foreground
                            border.width: 1.5
                        }

                        MouseArea {
                            id: eraserSizeMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.setEraserRadius(eraserSizeItem.modelData.radius)
                        }
                    }
                }

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 24
                    height: 1
                    color: theme.lighterBackground
                }

                // Quick Clear Page Button
                Rectangle {
                    width: 36
                    height: 36
                    radius: 8
                    color: clearMouse.containsMouse ? Qt.rgba(0.9, 0.2, 0.2, 0.2) : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "🗑"
                        font.pixelSize: 14
                    }

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
}
