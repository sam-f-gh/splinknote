import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    anchors.fill: parent
    color: "#cc0a0a0c"
    visible: false

    function toggle() {
        visible = !visible;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.visible = false
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 32, 460)
        height: contentCol.implicitHeight + 40
        radius: theme.radius > 0 ? Math.min(theme.radius, 16) : 14
        color: "#181a20"
        border.color: "#303440"
        border.width: 1

        ColumnLayout {
            id: contentCol
            anchors.centerIn: parent
            width: parent.width - 48
            spacing: 16

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: qsTr("Keyboard Shortcuts")
                    font.pixelSize: 18
                    font.bold: true
                    color: "#ffffff"
                    Layout.fillWidth: true
                }

                Text {
                    text: "✕"
                    font.pixelSize: 16
                    color: "#888c99"
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.visible = false
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#282b34"
            }

            GridLayout {
                columns: 2
                rowSpacing: 10
                columnSpacing: 24
                Layout.fillWidth: true

                Text { text: "P"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Pen tool"); color: "#d0d3dc" }

                Text { text: "H"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Highlighter tool"); color: "#d0d3dc" }

                Text { text: "E"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Eraser tool"); color: "#d0d3dc" }

                Text { text: "Ctrl + Z"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Undo stroke"); color: "#d0d3dc" }

                Text { text: "Ctrl + Y / Ctrl+Shift+Z"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Redo stroke"); color: "#d0d3dc" }

                Text { text: "Ctrl + S"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Save PDF"); color: "#d0d3dc" }

                Text { text: "Ctrl + O"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Open document"); color: "#d0d3dc" }

                Text { text: "N / PageDown"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Next page"); color: "#d0d3dc" }

                Text { text: "B / PageUp"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Previous page"); color: "#d0d3dc" }

                Text { text: "+"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Add new page"); color: "#d0d3dc" }

                Text { text: "Esc"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Close current note"); color: "#d0d3dc" }

                Text { text: "?"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Toggle this help overlay"); color: "#d0d3dc" }
            }
        }
    }
}
