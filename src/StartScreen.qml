import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: "#0e0e10"
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
                    text: "✎"
                    font.pixelSize: 32
                    color: theme.accentForeground
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "SplinkNote"
                font.pixelSize: 32
                font.bold: true
                color: "#f0f2f6"
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Minimalist note-taking and PDF annotation")
                font.pixelSize: 14
                color: "#8c909e"
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
                Layout.preferredHeight: 120
                radius: theme.radius > 0 ? Math.min(theme.radius, 14) : 12
                color: newArea.containsPress ? "#282a32" : newArea.containsMouse ? "#202228" : "#17181c"
                border.color: newArea.containsMouse ? theme.accent : "#2d3038"
                border.width: newArea.containsMouse ? 2 : 1

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "＋"
                        font.pixelSize: 28
                        font.bold: true
                        color: theme.accent
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("New Note")
                        font.pixelSize: 16
                        font.bold: true
                        color: "#ffffff"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Start on a blank page")
                        font.pixelSize: 12
                        color: "#888c99"
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
                Layout.preferredHeight: 120
                radius: theme.radius > 0 ? Math.min(theme.radius, 14) : 12
                color: openArea.containsPress ? "#282a32" : openArea.containsMouse ? "#202228" : "#17181c"
                border.color: openArea.containsMouse ? theme.accent : "#2d3038"
                border.width: openArea.containsMouse ? 2 : 1

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "📁"
                        font.pixelSize: 28
                        color: "#d0d3dc"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Open PDF")
                        font.pixelSize: 16
                        font.bold: true
                        color: "#ffffff"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Annotate or continue notes")
                        font.pixelSize: 12
                        color: "#888c99"
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
            font.pixelSize: 12
            color: "#5f6370"
        }
    }
}
