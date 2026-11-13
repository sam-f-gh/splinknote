import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Rectangle {
    id: root

    property alias text: label.text
    property alias iconText: iconLabel.text
    property bool active: false
    property bool enabled: true
    signal clicked()

    implicitWidth: Math.max(44, contentRow.implicitWidth + 24)
    implicitHeight: 44
    radius: theme.radius > 0 ? Math.min(theme.radius, 10) : 8

    color: !enabled ? "transparent"
         : active ? theme.accent
         : mouseArea.containsPress ? "#383a42"
         : mouseArea.containsMouse ? "#282a30"
         : "transparent"

    border.color: active ? "transparent"
                : mouseArea.containsMouse ? "#454852"
                : "transparent"
    border.width: 1

    opacity: enabled ? 1.0 : 0.35

    Row {
        id: contentRow
        anchors.centerIn: parent
        spacing: 6

        Text {
            id: iconLabel
            visible: text.length > 0
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 18
            font.bold: true
            color: root.active ? theme.accentForeground : "#e2e4e9"
        }

        Text {
            id: label
            visible: text.length > 0
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 13
            font.bold: root.active
            color: root.active ? theme.accentForeground : "#d0d3dc"
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.clicked()
    }
}
