import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: root

    implicitWidth: mainRow.implicitWidth + 16
    implicitHeight: 52
    radius: theme.radius > 0 ? Math.min(theme.radius, 14) : 12

    color: "#e61b1d24"
    border.color: "#323540"
    border.width: 1

    // Drop shadow
    Rectangle {
        z: -1
        anchors.fill: parent
        anchors.margins: -1
        radius: root.radius + 1
        color: "#33000000"
    }

    RowLayout {
        id: mainRow
        anchors.centerIn: parent
        spacing: 4

        // 1. Home / Close Document
        BarButton {
            iconText: "⌂"
            onClicked: backend.closeDocument()
            ToolTip.visible: (hovered || active) && !Qt.isTouchScreen
            ToolTip.text: qsTr("Close note")
        }

        Rectangle {
            width: 1
            height: 24
            color: "#2f323c"
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 2
            Layout.rightMargin: 2
        }

        // 2. Tools
        BarButton {
            iconText: "✎"
            text: qsTr("Pen")
            active: backend.currentTool === 0
            onClicked: backend.setCurrentTool(0)
        }

        BarButton {
            iconText: "🖍"
            text: qsTr("Highlighter")
            active: backend.currentTool === 1
            onClicked: backend.setCurrentTool(1)
        }

        BarButton {
            iconText: "⌫"
            text: qsTr("Eraser")
            active: backend.currentTool === 2
            onClicked: backend.setCurrentTool(2)
        }

        Rectangle {
            width: 1
            height: 24
            color: "#2f323c"
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 2
            Layout.rightMargin: 2
        }

        // 3. Undo / Redo
        BarButton {
            iconText: "↶"
            enabled: backend.canUndo
            onClicked: backend.undo()
        }

        BarButton {
            iconText: "↷"
            enabled: backend.canRedo
            onClicked: backend.redo()
        }

        Rectangle {
            width: 1
            height: 24
            color: "#2f323c"
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 2
            Layout.rightMargin: 2
        }

        // 4. Page Nav & Add
        BarButton {
            iconText: "‹"
            enabled: backend.currentPageIndex > 0
            onClicked: backend.previousPage()
        }

        Item {
            implicitWidth: pageText.implicitWidth + 12
            implicitHeight: 36
            Layout.alignment: Qt.AlignVCenter

            Text {
                id: pageText
                anchors.centerIn: parent
                text: qsTr("%1 / %2").arg(backend.currentPageIndex + 1).arg(Math.max(1, backend.pageCount))
                font.pixelSize: 13
                font.bold: true
                color: "#e2e4e9"
            }
        }

        BarButton {
            iconText: "›"
            enabled: backend.currentPageIndex < backend.pageCount - 1
            onClicked: backend.nextPage()
        }

        BarButton {
            iconText: "＋"
            onClicked: backend.addPage()
        }

        Rectangle {
            width: 1
            height: 24
            color: "#2f323c"
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 2
            Layout.rightMargin: 2
        }

        // 5. Save Button with Unsaved Indicator
        BarButton {
            text: qsTr("Save")
            onClicked: backend.saveDocument()

            Rectangle {
                visible: backend.hasUnsavedChanges
                width: 7
                height: 7
                radius: 3.5
                color: theme.accent
                anchors.top: parent.top
                anchors.topMargin: 6
                anchors.right: parent.right
                anchors.rightMargin: 6
            }
        }
    }
}
