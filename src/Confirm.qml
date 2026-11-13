import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: root
    anchors.fill: parent
    color: "#b3000000"
    visible: false

    property string message: qsTr("You have unsaved changes. Discard them?")
    signal confirmed()
    signal cancelled()

    function ask(msg, onConfirm) {
        if (msg) message = msg;
        root.confirmed.connect(onConfirm);
        root.visible = true;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.visible = false;
            root.cancelled();
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 380)
        height: dialogCol.implicitHeight + 40
        radius: theme.radius > 0 ? Math.min(theme.radius, 14) : 12
        color: "#1c1e26"
        border.color: "#323644"
        border.width: 1

        ColumnLayout {
            id: dialogCol
            anchors.centerIn: parent
            width: parent.width - 40
            spacing: 20

            Text {
                text: qsTr("Unsaved Changes")
                font.pixelSize: 18
                font.bold: true
                color: "#ffffff"
            }

            Text {
                text: root.message
                font.pixelSize: 14
                color: "#c0c3ce"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Item { Layout.fillWidth: true }

                BarButton {
                    text: qsTr("Cancel")
                    onClicked: {
                        root.visible = false;
                        root.cancelled();
                    }
                }

                BarButton {
                    text: qsTr("Discard")
                    active: true
                    onClicked: {
                        root.visible = false;
                        root.confirmed();
                    }
                }
            }
        }
    }
}
