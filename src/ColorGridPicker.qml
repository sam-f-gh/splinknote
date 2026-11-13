import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: pickerPopup
    parent: Overlay.overlay
    padding: 8
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside | Popup.CloseOnPressOutsideParent

    signal colorPicked(color chosenColor)

    background: Rectangle {
        color: theme.darkBackground
        radius: Math.min(theme.radius, 12)
        border.color: theme.lighterBackground
        border.width: 1

        Rectangle {
            z: -1
            anchors.fill: parent
            anchors.margins: -2
            radius: parent.radius + 2
            color: Qt.rgba(0, 0, 0, 0.4)
        }
    }

    contentItem: ColumnLayout {
        spacing: 6

        Text {
            text: qsTr("Palette")
            font.family: "JetBrainsMono Nerd Font"
            font.pixelSize: 11
            font.bold: true
            color: theme.secondaryForeground
            Layout.leftMargin: 2
        }

        // 8 Hue Columns x 5 Shades Grid
        Grid {
            columns: 8
            rows: 5
            spacing: 3
            flow: Grid.LeftToRight

            Repeater {
                model: [
                    // Row 0: Lightest / Pastel tints
                    "#FFFFFF", "#FEE2E2", "#FFEDD5", "#FEF9C3", "#DCFCE7", "#CFFAFE", "#DBEAFE", "#F3E8FF",
                    // Row 1: Light tones
                    "#D1D5DB", "#FCA5A5", "#FDBA74", "#FDE047", "#86EFAC", "#67E8F9", "#93C5FD", "#D8B4FE",
                    // Row 2: Standard Base Hues
                    "#9CA3AF", "#EF4444", "#F97316", "#EAB308", "#22C55E", "#06B6D4", "#3B82F6", "#A855F7",
                    // Row 3: Deep tones
                    "#4B5563", "#B91C1C", "#C2410C", "#A16207", "#15803D", "#0E7490", "#1D4ED8", "#7E22CE",
                    // Row 4: Dark shades
                    "#111827", "#7F1D1D", "#7C2D12", "#713F12", "#14532D", "#164E63", "#1E3A8A", "#581C87"
                ]
                delegate: Rectangle {
                    id: swatchCell
                    required property string modelData
                    width: 20
                    height: 20
                    radius: 3
                    color: swatchCell.modelData
                    border.color: cellMouse.containsMouse ? theme.foreground : Qt.rgba(0, 0, 0, 0.25)
                    border.width: cellMouse.containsMouse ? 1.5 : 0.5
                    scale: cellMouse.containsMouse ? 1.15 : 1.0
                    Behavior on scale { NumberAnimation { duration: 80 } }

                    MouseArea {
                        id: cellMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            pickerPopup.colorPicked(swatchCell.modelData)
                            pickerPopup.close()
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.lighterBackground
        }

        // Theme Accent Quick Pick
        RowLayout {
            spacing: 6
            Layout.alignment: Qt.AlignVCenter

            Rectangle {
                width: 18
                height: 18
                radius: 3
                color: theme.accent
                border.color: theme.foreground
                border.width: 1

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        pickerPopup.colorPicked(theme.accent)
                        pickerPopup.close()
                    }
                }
            }

            Text {
                text: qsTr("Theme Accent")
                font.family: "JetBrainsMono Nerd Font"
                font.pixelSize: 10
                color: theme.foreground
            }
        }
    }
}
