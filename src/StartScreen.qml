import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: theme.background
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 36
        width: Math.min(parent.width - 48, 520)

        // Logo & Title
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                width: 64
                height: 64
                radius: theme.radius > 0 ? Math.min(theme.radius, 16) : 14
                color: theme.accent

                Text {
                    anchors.centerIn: parent
                    text: "󰏫"
                    font.family: "JetBrainsMono Nerd Font"
                    font.pixelSize: 32
                    color: theme.accentForeground
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "SplinkNote"
                font.family: "JetBrainsMono Nerd Font"
                font.pixelSize: 30
                font.bold: true
                color: theme.foreground
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Minimalist note-taking and PDF annotation")
                font.family: "JetBrainsMono Nerd Font"
                font.pixelSize: 13
                color: theme.secondaryForeground
            }
        }

        // Primary Action Cards (Generous touch targets for tablets)
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            spacing: 20

            // New Blank Note Card
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 124
                radius: theme.radius > 0 ? Math.min(theme.radius, 14) : 12
                color: newArea.containsPress ? theme.darkBackground : newArea.containsMouse ? theme.lighterBackground : theme.darkBackground
                border.color: newArea.containsMouse ? theme.accent : theme.lighterBackground
                border.width: newArea.containsMouse ? 2 : 1

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "＋"
                        font.family: "JetBrainsMono Nerd Font"
                        font.pixelSize: 28
                        font.bold: true
                        color: theme.accent
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("New Note")
                        font.family: "JetBrainsMono Nerd Font"
                        font.pixelSize: 15
                        font.bold: true
                        color: theme.foreground
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Start on a blank page")
                        font.family: "JetBrainsMono Nerd Font"
                        font.pixelSize: 12
                        color: theme.secondaryForeground
                    }
                }

                MouseArea {
                    id: newArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: backend.newDocument()
                }
            }

            // Open Document Card
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 124
                radius: theme.radius > 0 ? Math.min(theme.radius, 14) : 12
                color: openArea.containsPress ? theme.darkBackground : openArea.containsMouse ? theme.lighterBackground : theme.darkBackground
                border.color: openArea.containsMouse ? theme.accent : theme.lighterBackground
                border.width: openArea.containsMouse ? 2 : 1

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "󰉋"
                        font.family: "JetBrainsMono Nerd Font"
                        font.pixelSize: 26
                        color: theme.foreground
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Open PDF")
                        font.family: "JetBrainsMono Nerd Font"
                        font.pixelSize: 15
                        font.bold: true
                        color: theme.foreground
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Annotate or continue notes")
                        font.family: "JetBrainsMono Nerd Font"
                        font.pixelSize: 12
                        color: theme.secondaryForeground
                    }
                }

                MouseArea {
                    id: openArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: backend.openDocument()
                }
            }
        }

        // Quick tip
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Press '?' anytime for keyboard shortcuts")
            font.family: "JetBrainsMono Nerd Font"
            font.pixelSize: 11
            color: theme.secondaryForeground
        }
    }
}
