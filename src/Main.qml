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

    Shortcut {
        sequences: [StandardKey.Redo]
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
        onActivated: backend.nextPage()
    }

    Shortcut {
        sequence: "PageDown"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: backend.nextPage()
    }

    Shortcut {
        sequence: "B"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: backend.previousPage()
    }

    Shortcut {
        sequence: "PageUp"
        context: Qt.ApplicationShortcut
        enabled: backend.hasDocument
        onActivated: backend.previousPage()
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
            }

            // High-DPI Canvas Item anchored cleanly below the header bar
            CanvasItem {
                id: canvas
                anchors.top: pill.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                backend: win.appBackend
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
