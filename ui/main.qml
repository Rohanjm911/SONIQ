import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "themes"
import "components"
import "panels"
import "screens"

ApplicationWindow {
    id: window

    title: "SONIQ — Pure Sound. Infinite Possibilities."
    width: Math.min(1240, Math.max(minimumWidth, Screen.desktopAvailableWidth ? Screen.desktopAvailableWidth * 0.84 : 1100))
    height: Math.min(760, Math.max(minimumHeight, Screen.desktopAvailableHeight ? Screen.desktopAvailableHeight * 0.84 : 700))
    minimumWidth: 860
    minimumHeight: 540
    visible: true
    color: Colors.background

    property string activeScreen: "home"
    property string activeLibraryTab: "songs"
    property bool queueOpen: false

    function showToast(msg, icon) {
        toast.show(msg, icon || "check");
    }

    Component.onCompleted: {
        if (settingsService.accentColor && settingsService.accentColor.length > 0) {
            Colors.accent = settingsService.accentColor;
        }
        if (settingsService.themeMode && settingsService.themeMode.length > 0) {
            Colors.isDark = (settingsService.themeMode !== "Light");
        }
        Motion.reducedMotion = settingsService.reducedMotion;

        if (Screen.desktopAvailableWidth > 0 && Screen.desktopAvailableHeight > 0) {
            if (window.width > Screen.desktopAvailableWidth) {
                window.width = Screen.desktopAvailableWidth * 0.9;
            }
            if (window.height > Screen.desktopAvailableHeight) {
                window.height = Screen.desktopAvailableHeight * 0.85;
            }
            window.x = Screen.virtualX + (Screen.desktopAvailableWidth - window.width) / 2;
            window.y = Screen.virtualY + (Screen.desktopAvailableHeight - window.height) / 2;
        }
    }

    Item {
        anchors.fill: parent
        clip: true

        // Main Horizontal Split: Sidebar + Content Area
        Row {
            anchors.top: parent.top
            anchors.bottom: miniPlayer.top
            anchors.left: parent.left
            anchors.right: parent.right

            // Left Navigation Sidebar
            NavigationSidebar {
                id: sidebar
                height: parent.height
                currentScreen: window.activeScreen
                onNavigationRequested: (screenName) => {
                    window.activeScreen = screenName;
                }
            }

            // Central Content View Container
            Item {
                width: parent.width - sidebar.width
                height: parent.height
                clip: true

                // Top Notification / Library Scanner Banner
                Rectangle {
                    id: scannerBanner
                    visible: libraryScanner.isScanning
                    width: parent.width
                    height: 38
                    color: Colors.accentMuted
                    border.color: Colors.accent
                    border.width: 1
                    z: 10

                    Row {
                        anchors.centerIn: parent
                        spacing: 8

                        SoniqIcon {
                            name: "folder"
                            size: 15
                            color: Colors.accent
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: libraryScanner.totalCount > 0 ?
                                  ("Scanning music library... (" + libraryScanner.scannedCount + " / " + libraryScanner.totalCount + ")") :
                                  "Scanning music library..."
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            font.weight: Typography.weightMedium
                            color: Colors.textPrimary
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                // Screen Views with subtle fade transition
                Loader {
                    id: screenLoader
                    anchors.fill: parent
                    anchors.topMargin: scannerBanner.visible ? 38 : 0
                    sourceComponent: {
                        if (window.activeScreen === "home") return homeComponent;
                        if (window.activeScreen === "songs") return songsComponent;
                        if (window.activeScreen === "nowplaying") return nowPlayingComponent;
                        if (window.activeScreen === "equalizer") return equalizerComponent;
                        if (window.activeScreen === "inspector") return inspectorComponent;
                        if (window.activeScreen === "settings") return settingsComponent;
                        return homeComponent;
                    }
                }

                Component {
                    id: homeComponent
                    HomeScreen {
                        onScanFolderRequested: (path) => {
                            libraryScanner.scanDirectory(path);
                        }
                    }
                }

                Component {
                    id: songsComponent
                    SongsScreen {
                        onScanFolderRequested: (path) => {
                            libraryScanner.scanDirectory(path);
                        }
                    }
                }

                Component {
                    id: nowPlayingComponent
                    NowPlayingScreen {}
                }

                Component {
                    id: equalizerComponent
                    EqualizerScreen {}
                }

                Component {
                    id: inspectorComponent
                    AudioInspectorScreen {}
                }

                Component {
                    id: settingsComponent
                    SettingsScreen {}
                }
            }
        }

        // Persistent Mini-Player at bottom
        MiniPlayer {
            id: miniPlayer
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            onExpandRequested: {
                window.activeScreen = "nowplaying";
            }
            onQueueToggleRequested: {
                window.queueOpen = !window.queueOpen;
            }
            onFullscreenToggleRequested: {
                if (window.visibility === Window.FullScreen) {
                    window.visibility = Window.Windowed;
                } else {
                    window.visibility = Window.FullScreen;
                }
            }
        }

        // Keyboard Shortcuts
        Shortcut {
            sequence: "Space"
            onActivated: playbackController.togglePlayPause()
        }

        Shortcut {
            sequence: "Left"
            onActivated: playbackController.seek(Math.max(0.0, playbackController.position - 5.0))
        }

        Shortcut {
            sequence: "Right"
            onActivated: playbackController.seek(Math.min(playbackController.duration, playbackController.position + 5.0))
        }

        Shortcut {
            sequence: "Up"
            onActivated: playbackController.setVolume(Math.min(1.0, playbackController.volume + 0.05))
        }

        Shortcut {
            sequence: "Down"
            onActivated: playbackController.setVolume(Math.max(0.0, playbackController.volume - 0.05))
        }

        Shortcut {
            sequence: "M"
            onActivated: playbackController.setMuted(!playbackController.isMuted)
        }

        Shortcut {
            sequence: "Ctrl+Q"
            onActivated: window.queueOpen = !window.queueOpen
        }

        Shortcut {
            sequence: "F11"
            onActivated: {
                if (window.visibility === Window.FullScreen) {
                    window.visibility = Window.Windowed;
                } else {
                    window.visibility = Window.FullScreen;
                }
            }
        }

        Shortcut {
            sequence: "Escape"
            onActivated: {
                if (window.queueOpen) {
                    window.queueOpen = false;
                } else if (window.visibility === Window.FullScreen) {
                    window.visibility = Window.Windowed;
                }
            }
        }

        // Slide-in Queue Drawer from right
        QueueDrawer {
            id: queueDrawer
            anchors.top: parent.top
            anchors.bottom: miniPlayer.top
            x: window.queueOpen ? parent.width - width : parent.width
            visible: window.queueOpen || x < parent.width
            isOpen: window.queueOpen
            z: 20
            onCloseRequested: {
                window.queueOpen = false;
            }
        }

        // Toast Micro-Notification Popup
        SoniqToast {
            id: toast
            anchors.top: parent.top
            anchors.topMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            z: 100
        }

        // Global Modal Dialogs
        CreatePlaylistDialog {
            id: createPlaylistDialog
        }

        TagEditorDialog {
            id: tagEditorDialog
        }

        AddToPlaylistDialog {
            id: addToPlaylistDialog
        }

        // Animated Splash Screen
        SplashOverlay {
            id: splashOverlay
        }
    }
}
