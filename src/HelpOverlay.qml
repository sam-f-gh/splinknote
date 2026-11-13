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
        color: theme.darkBackground
        border.color: theme.lighterBackground
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
                    font.family: "JetBrainsMono Nerd Font"
                    font.pixelSize: 16
                    font.bold: true
                    color: theme.foreground
                    Layout.fillWidth: true
                }

                Text {
                    text: "✕"
                    font.family: "JetBrainsMono Nerd Font"
                    font.pixelSize: 16
                    color: theme.secondaryForeground
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
                color: theme.lighterBackground
            }

            GridLayout {
                columns: 2
                rowSpacing: 10
                columnSpacing: 24
                Layout.fillWidth: true

                Text { text: "P"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Pen tool"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "H"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Highlighter tool"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "E"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Eraser tool"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "Ctrl + Z"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Undo stroke"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "Ctrl + Y / Ctrl+Shift+Z"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Redo stroke"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "Ctrl + S"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Save PDF"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "Ctrl + O"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Open document"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "N / PageDown"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Next page"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "B / PageUp"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Previous page"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "+"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Add new page"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "Esc"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Close current note"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }

                Text { text: "?"; font.family: "JetBrainsMono Nerd Font"; font.bold: true; color: theme.accent }
                Text { text: qsTr("Toggle this help overlay"); font.family: "JetBrainsMono Nerd Font"; color: theme.foreground }
            }
        }
    }
}
