import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: root
    anchors.fill: parent
    color: "#b3000000"
    visible: false

    property string title: qsTr("Confirm")
    property string message: qsTr("You have unsaved changes. Discard them?")
    property string confirmButtonText: qsTr("Confirm")
    property var confirmCallback: null
    signal confirmed()
    signal cancelled()

    function ask(msg, onConfirm, dlgTitle, btnText) {
        if (msg) message = msg;
        title = dlgTitle ? dlgTitle : qsTr("Confirm");
        confirmButtonText = btnText ? btnText : qsTr("Confirm");
        confirmCallback = onConfirm;
        root.visible = true;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.visible = false;
            root.confirmCallback = null;
            root.cancelled();
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 380)
        height: dialogCol.implicitHeight + 40
        radius: theme.radius > 0 ? Math.min(theme.radius, 14) : 12
        color: theme.darkBackground
        border.color: theme.lighterBackground
        border.width: 1

        ColumnLayout {
            id: dialogCol
            anchors.centerIn: parent
            width: parent.width - 40
            spacing: 20

            Text {
                text: root.title
                font.family: "JetBrainsMono Nerd Font"
                font.pixelSize: 16
                font.bold: true
                color: theme.foreground
            }

            Text {
                text: root.message
                font.family: "JetBrainsMono Nerd Font"
                font.pixelSize: 13
                color: theme.secondaryForeground
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
                        root.confirmCallback = null;
                        root.cancelled();
                    }
                }

                BarButton {
                    text: root.confirmButtonText
                    active: true
                    onClicked: {
                        root.visible = false;
                        var cb = root.confirmCallback;
                        root.confirmCallback = null;
                        root.confirmed();
                        if (cb) cb();
                    }
                }
            }
        }
    }
}
