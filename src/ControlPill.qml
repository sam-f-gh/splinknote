import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: root

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

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 8

        // Left Section: Home/Close & Document Title
        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 8

            BarButton {
                iconText: "⌂"
                onClicked: {
                    if (backend.hasUnsavedChanges) {
                        confirmDialog.ask(qsTr("Close note and discard unsaved changes?"), function() {
                            backend.closeDocument()
                        })
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
                    Layout.maximumWidth: 160
                }
            }
        }

        // Center Spacer
        Item { Layout.fillWidth: true }

        // Center Section: The Core Tools
        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 4

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
        }

        // Right Spacer
        Item { Layout.fillWidth: true }

        // Right Section: Undo/Redo, Page Nav & Save
        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 4

            BarButton {
                iconText: "↶"
                enabled: backend.canUndo
                onClicked: backend.undo()
                ToolTip.visible: hovered && !Qt.isTouchScreen
                ToolTip.text: qsTr("Undo (Ctrl+Z)")
            }

            BarButton {
                iconText: "↷"
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
                onClicked: backend.previousPage()
            }

            Text {
                text: qsTr("%1 / %2").arg(backend.currentPageIndex + 1).arg(Math.max(1, backend.pageCount))
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
                onClicked: backend.nextPage()
            }

            BarButton {
                iconText: "＋"
                onClicked: backend.addPage()
                ToolTip.visible: hovered && !Qt.isTouchScreen
                ToolTip.text: qsTr("Add Page (+)")
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
    }
}
