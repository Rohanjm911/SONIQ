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
    width: Math.min(1200, Math.max(minimumWidth, Screen.desktopAvailableWidth ? Screen.desktopAvailableWidth * 0.82 : 1100))
    height: Math.min(720, Math.max(minimumHeight, Screen.desktopAvailableHeight ? Screen.desktopAvailableHeight * 0.82 : 680))
    minimumWidth: 840
    minimumHeight: 520
    visible: true
    color: Colors.background

    property string activeScreen: "home"
    property bool queueOpen: false

    Component.onCompleted: {
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
                    height: 36
                    color: Colors.accentMuted
                    border.color: Colors.accent
                    border.width: 1
                    z: 10

                    Row {
                        anchors.centerIn: parent
                        spacing: 8
                        Text {
                            text: "Scanning music library..."
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            color: Colors.textPrimary
                        }
                    }
                }

                // Screen Views
                Loader {
                    id: screenLoader
                    anchors.fill: parent
                    anchors.topMargin: scannerBanner.visible ? 36 : 0
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
    }
}
