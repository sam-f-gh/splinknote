import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import SplinkNote 1.0

ApplicationWindow {
    id: win
    width: 960
    height: 680
    minimumWidth: 360
    minimumHeight: 400
    visible: true

    readonly property var appBackend: backend

    title: backend.hasDocument
           ? (backend.currentFilePath.length > 0
              ? backend.currentFilePath.substring(backend.currentFilePath.lastIndexOf('/') + 1)
              : qsTr("Untitled Note")) + (backend.hasUnsavedChanges ? " ●" : "") + " — SplinkNote"
           : "SplinkNote"
    color: theme.background

    Material.theme: theme.light ? Material.Light : Material.Dark
    Material.accent: theme.accent
    Material.roundedScale: theme.radius

    // Safe close handler
    onClosing: function(close) {
        if (backend.hasUnsavedChanges) {
            close.accepted = false
            confirmDialog.ask(qsTr("You have unsaved notes. Exit without saving?"), function() {
                Qt.quit()
            })
        }
    }

    // Keyboard Shortcuts
    Shortcut {
        sequence: "P"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: backend.setCurrentTool(0)
    }

    Shortcut {
        sequence: "H"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: backend.setCurrentTool(1)
    }

    Shortcut {
        sequence: "E"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: backend.setCurrentTool(2)
    }

    Shortcut {
        sequences: [StandardKey.Undo]
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument && backend.canUndo
        onActivated: backend.undo()
    }

    // Redo: explicit Ctrl+Y alongside StandardKey.Redo for robust Linux parity
    Shortcut {
        sequences: [StandardKey.Redo, "Ctrl+Y"]
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument && backend.canRedo
        onActivated: backend.redo()
    }

    Shortcut {
        sequences: [StandardKey.Save]
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: backend.saveDocument()
    }

    Shortcut {
        sequences: [StandardKey.Open]
        context: Qt.ApplicationShortcut
        onActivated: backend.openDocument()
    }

    Shortcut {
        sequences: [StandardKey.New]
        context: Qt.ApplicationShortcut
        onActivated: {
            if (backend.hasUnsavedChanges) {
                confirmDialog.ask(qsTr("Start a new note and discard unsaved changes?"), function() {
                    backend.newDocument()
                })
            } else {
                backend.newDocument()
            }
        }
    }

    Shortcut {
        sequence: "N"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: {
            backend.nextPage()
            canvas.scrollToPage(backend.currentPageIndex)
        }
    }

    Shortcut {
        sequence: "PageDown"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: {
            backend.nextPage()
            canvas.scrollToPage(backend.currentPageIndex)
        }
    }

    Shortcut {
        sequence: "B"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: {
            backend.previousPage()
            canvas.scrollToPage(backend.currentPageIndex)
        }
    }

    Shortcut {
        sequence: "PageUp"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: {
            backend.previousPage()
            canvas.scrollToPage(backend.currentPageIndex)
        }
    }

    Shortcut {
        sequence: "+"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: backend.addPage()
    }

    Shortcut {
        sequence: "="
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: backend.addPage()
    }

    Shortcut {
        sequence: "Escape"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: {
            if (backend.hasUnsavedChanges) {
                confirmDialog.ask(qsTr("Close note and discard unsaved changes?"), function() {
                    backend.closeDocument()
                })
            } else {
                backend.closeDocument()
            }
        }
    }

    Shortcut {
        sequence: "?"
        context: Qt.ApplicationShortcut
        onActivated: helpOverlay.toggle()
    }

    Shortcut {
        sequence: "Ctrl+Q"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (backend.hasUnsavedChanges) {
                confirmDialog.ask(qsTr("Exit and discard unsaved changes?"), function() {
                    Qt.quit()
                })
            } else {
                Qt.quit()
            }
        }
    }

    // Main View
    Item {
        anchors.fill: parent
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        // 1. Clean Slate Start Screen (when no document is open)
        StartScreen {
            anchors.fill: parent
            visible: !backend.hasDocument
        }

        // 2. Inking Canvas View (when document is active)
        Item {
            anchors.fill: parent
            visible: backend.hasDocument

            // Integrated Header Bar
            ControlPill {
                id: pill
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 52
                z: 10
                canvasItem: canvas
            }

            // High-DPI Continuous Canvas Item anchored cleanly below the header bar
            CanvasItem {
                id: canvas
                anchors.top: pill.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                backend: win.appBackend
                backgroundColor: theme.background
            }

            // Minimalist Contextual Shelf (docked to left edge)
            ToolShelf {
                id: toolShelf
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.top: pill.bottom
                anchors.topMargin: 16
                z: 20
                visible: backend.hasDocument
            }

            // Status message at bottom-left corner
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12
                text: backend.status
                color: theme.secondaryForeground
                font.family: "JetBrainsMono Nerd Font"
                font.pixelSize: 11
                opacity: text.length > 0 ? 0.9 : 0.0
                Behavior on opacity { NumberAnimation { duration: 200 } }
            }

            // Touch-Friendly Zoom Controls cluster at bottom-right
            Rectangle {
                id: zoomPill
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 12
                width: zoomRow.implicitWidth + 12
                height: 36
                radius: Math.min(theme.radius, 10)
                color: theme.darkBackground
                border.color: theme.lighterBackground
                border.width: 1
                opacity: 0.95
                z: 15

                RowLayout {
                    id: zoomRow
                    anchors.centerIn: parent
                    spacing: 2

                    BarButton {
                        text: "－"
                        implicitWidth: 32
                        implicitHeight: 30
                        onClicked: canvas.zoomOut()
                    }

                    Rectangle {
                        implicitWidth: 54
                        implicitHeight: 30
                        radius: 4
                        color: fitArea.containsMouse ? theme.lighterBackground : "transparent"

                        Text {
                            anchors.centerIn: parent
                            text: Math.round(canvas.zoom * 100) + "%"
                            color: theme.foreground
                            font.family: "JetBrainsMono Nerd Font"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        MouseArea {
                            id: fitArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (Math.abs(canvas.zoom - 1.0) < 0.05) {
                                    canvas.fitToWidth(canvas.width, canvas.height)
                                } else {
                                    canvas.resetView()
                                }
                            }
                        }
                    }

                    BarButton {
                        text: "＋"
                        implicitWidth: 32
                        implicitHeight: 30
                        onClicked: canvas.zoomIn()
                    }
                }
            }
        }

        // Overlays
        Confirm {
            id: confirmDialog
        }

        HelpOverlay {
            id: helpOverlay
        }
    }
}
