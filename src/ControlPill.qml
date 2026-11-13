import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: root

    property var canvasItem: null
    implicitHeight: 52
    color: theme.darkBackground

    // 1px subtle divider border at bottom
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: theme.lighterBackground
    }

    // Left Section: Home/Close & Document Title
    RowLayout {
        id: leftSection
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        BarButton {
            iconText: "󰋜"
            onClicked: {
                if (backend.hasUnsavedChanges) {
                    confirmDialog.ask(qsTr("Close note and discard unsaved changes?"), function() {
                        backend.closeDocument()
                    }, qsTr("Close Note"), qsTr("Discard"))
                } else {
                    backend.closeDocument()
                }
            }
            ToolTip.visible: hovered && !Qt.isTouchScreen
            ToolTip.text: qsTr("Close note")
        }

        Rectangle {
            width: 1
            height: 24
            color: theme.lighterBackground
        }

        ColumnLayout {
            spacing: 2
            Layout.alignment: Qt.AlignVCenter

            RowLayout {
                spacing: 6
                Text {
                    text: "SplinkNote"
                    font.family: "JetBrainsMono Nerd Font"
                    font.pixelSize: 13
                    font.bold: true
                    color: theme.accent
                }

                Rectangle {
                    visible: backend.hasUnsavedChanges
                    width: 6
                    height: 6
                    radius: 3
                    color: theme.accent
                }
            }

            Text {
                text: backend.currentFilePath.length > 0
                      ? backend.currentFilePath.substring(backend.currentFilePath.lastIndexOf('/') + 1)
                      : qsTr("Untitled Note")
                font.family: "JetBrainsMono Nerd Font"
                font.pixelSize: 11
                color: theme.secondaryForeground
                elide: Text.ElideRight
                Layout.maximumWidth: 150
            }
        }
    }

    // Right Section: Undo/Redo, Page Nav & Save
    RowLayout {
        id: rightSection
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        spacing: 4

        BarButton {
            iconText: "󰕌"
            enabled: backend.canUndo
            onClicked: backend.undo()
            ToolTip.visible: hovered && !Qt.isTouchScreen
            ToolTip.text: qsTr("Undo (Ctrl+Z)")
        }

        BarButton {
            iconText: "󰑎"
            enabled: backend.canRedo
            onClicked: backend.redo()
            ToolTip.visible: hovered && !Qt.isTouchScreen
            ToolTip.text: qsTr("Redo (Ctrl+Y)")
        }

        Rectangle {
            width: 1
            height: 24
            color: theme.lighterBackground
            Layout.leftMargin: 4
            Layout.rightMargin: 4
        }

        BarButton {
            iconText: "‹"
            enabled: backend.currentPageIndex > 0
            onClicked: {
                backend.previousPage()
                if (root.canvasItem) root.canvasItem.scrollToPage(backend.currentPageIndex)
            }
        }

        Text {
            text: qsTr("%1 / %2").arg(backend.currentPageIndex + 1).arg(Math.max(backend.currentPageIndex + 1, backend.effectivePageCount))
            font.family: "JetBrainsMono Nerd Font"
            font.pixelSize: 12
            font.bold: true
            color: theme.foreground
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 4
            Layout.rightMargin: 4
        }

        BarButton {
            iconText: "›"
            enabled: backend.currentPageIndex < backend.pageCount - 1
            onClicked: {
                backend.nextPage()
                if (root.canvasItem) root.canvasItem.scrollToPage(backend.currentPageIndex)
            }
        }

        BarButton {
            iconText: "\U000f0752"
            onClicked: backend.addPage()
            ToolTip.visible: hovered && !Qt.isTouchScreen
            ToolTip.text: qsTr("Add Page (+)")
        }

        BarButton {
            iconText: "\U000f1aa1"
            enabled: backend.pageCount > 1
            ToolTip.visible: hovered && !Qt.isTouchScreen
            ToolTip.text: qsTr("Delete Page")
            onClicked: {
                confirmDialog.ask(qsTr("Delete page %1?").arg(backend.currentPageIndex + 1), function() {
                    backend.deleteCurrentPage()
                    if (root.canvasItem) root.canvasItem.scrollToPage(backend.currentPageIndex)
                }, qsTr("Delete Page"), qsTr("Delete"))
            }
        }

        Rectangle {
            width: 1
            height: 24
            color: theme.lighterBackground
            Layout.leftMargin: 4
            Layout.rightMargin: 4
        }

        BarButton {
            text: qsTr("Save")
            active: backend.hasUnsavedChanges
            onClicked: backend.saveDocument()
            ToolTip.visible: hovered && !Qt.isTouchScreen
            ToolTip.text: qsTr("Save PDF (Ctrl+S)")
        }
    }

    // Center Section: Exact Window Midline Alignment with Responsive Clamping
    RowLayout {
        id: centerSection
        anchors.verticalCenter: parent.verticalCenter
        spacing: 4

        x: {
            var idealX = (root.width - width) / 2
            var minX = leftSection.x + leftSection.width + 10
            var maxX = rightSection.x - width - 10
            if (maxX < minX) return minX
            return Math.max(minX, Math.min(maxX, idealX))
        }

        BarButton {
            iconText: "󰏫"
            text: qsTr("Pen")
            active: backend.currentTool === 0
            onClicked: backend.setCurrentTool(0)
        }

        BarButton {
            iconText: "\uee5a"
            text: qsTr("Highlighter")
            active: backend.currentTool === 1
            onClicked: backend.setCurrentTool(1)
        }

        BarButton {
            iconText: "\uf12d"
            text: qsTr("Eraser")
            active: backend.currentTool === 2
            onClicked: backend.setCurrentTool(2)
        }
    }
}
