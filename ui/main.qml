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
    width: 1200
    height: 800
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    color: Colors.background

    property string activeScreen: "home"
    property bool queueOpen: false

    Item {
        anchors.fill: parent

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
            anchors.right: parent.right
            x: window.queueOpen ? parent.width - width : parent.width
            isOpen: window.queueOpen
            onCloseRequested: {
                window.queueOpen = false;
            }
        }
    }
}
