import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "../themes"
import "../components"

Rectangle {
    id: root

    property string currentScreen: "home"
    signal navigationRequested(string screenName)

    width: Theme.sidebarWidth
    color: Colors.surface
    border.color: Colors.borderSubtle
    border.width: 1

    // App Branding (Pinned to top)
    Row {
        id: brandingRow
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 16
        height: 40
        spacing: 12

        Rectangle {
            width: 38
            height: 38
            radius: 10
            color: Colors.surfaceActive
            border.color: Colors.borderSubtle
            border.width: 1
            clip: true

            Image {
                anchors.centerIn: parent
                width: 32
                height: 32
                source: "qrc:/ui/assets/logo_icon.png"
                fillMode: Image.PreserveAspectFit
                smooth: false
                mipmap: true
            }
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 1
            Text {
                text: "SONIQ"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeTitleMedium
                font.weight: Typography.weightBold
                font.letterSpacing: 1.5
                color: Colors.textPrimary
            }
            Text {
                text: "PURE SOUND"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                font.letterSpacing: 1.0
                color: Colors.textMuted
            }
        }
    }

    // Scrollable Navigation Items
    Flickable {
        id: navFlickable
        anchors.top: brandingRow.bottom
        anchors.topMargin: 12
        anchors.bottom: statusCard.top
        anchors.bottomMargin: 8
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        contentHeight: navColumn.height
        clip: true

        Column {
            id: navColumn
            width: parent.width
            spacing: 18

            // Section: LISTEN NOW
            Column {
                width: parent.width
                spacing: 3

                Text {
                    text: "LISTEN NOW"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                    leftPadding: 12
                    bottomPadding: 4
                }

                Repeater {
                    model: [
                        { id: "home", label: "Home", icon: "⌂" },
                        { id: "songs", label: "Songs", icon: "♫" },
                        { id: "nowplaying", label: "Now Playing", icon: "▶" }
                    ]

                    delegate: Rectangle {
                        width: parent.width
                        height: 36
                        radius: Theme.cornerRadiusSmall
                        color: (root.currentScreen === modelData.id && libraryModel.activePlaylistId <= 0) ? Colors.surfaceActive : (navMouse.containsMouse ? Colors.surfaceHover : "transparent")

                        Behavior on color {
                            ColorAnimation { duration: Motion.durationFast }
                        }

                        MouseArea {
                            id: navMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (modelData.id === "songs") {
                                    libraryModel.clearPlaylistFilter();
                                }
                                root.navigationRequested(modelData.id);
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            spacing: 12

                            SoniqIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                name: modelData.icon
                                size: 16
                                color: (root.currentScreen === modelData.id && libraryModel.activePlaylistId <= 0) ? Colors.accent : Colors.textSecondary
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.label
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: (root.currentScreen === modelData.id && libraryModel.activePlaylistId <= 0) ? Typography.weightMedium : Typography.weightNormal
                                color: (root.currentScreen === modelData.id && libraryModel.activePlaylistId <= 0) ? Colors.textPrimary : Colors.textSecondary
                            }
                        }
                    }
                }
            }

            // Section: PLAYLISTS
            Column {
                width: parent.width
                spacing: 3

                Item {
                    width: parent.width
                    height: 24

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: "PLAYLISTS"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: Colors.textMuted
                    }

                    SoniqIconButton {
                        anchors.right: parent.right
                        anchors.rightMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        iconSymbol: "＋"
                        iconPixelSize: 12
                        onClicked: createPlaylistDialog.open()
                    }
                }

                Repeater {
                    model: libraryModel.playlists

                    delegate: Rectangle {
                        width: parent.width
                        height: 34
                        radius: Theme.cornerRadiusSmall
                        property bool isCurrentPl: (root.currentScreen === "songs" && libraryModel.activePlaylistId === modelData.id)
                        color: isCurrentPl ? Colors.surfaceActive : (plMouse.containsMouse ? Colors.surfaceHover : "transparent")

                        Behavior on color {
                            ColorAnimation { duration: Motion.durationFast }
                        }

                        MouseArea {
                            id: plMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                libraryModel.filterByPlaylist(modelData.id, modelData.name);
                                root.navigationRequested("songs");
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 10
                            spacing: 8

                            Text {
                                text: "☰"
                                font.pixelSize: 12
                                color: isCurrentPl ? Colors.accent : Colors.textMuted
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.name
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeCaption
                                font.weight: isCurrentPl ? Typography.weightMedium : Typography.weightNormal
                                color: isCurrentPl ? Colors.textPrimary : Colors.textSecondary
                                elide: Text.ElideRight
                                width: Math.max(50, parent.width - 70)
                            }

                            Item { width: 1; height: 1; Layout.fillWidth: true }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.trackCount ? modelData.trackCount.toString() : "0"
                                font.family: Typography.monospaceFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }
                    }
                }

                Text {
                    visible: libraryModel.playlists.length === 0
                    text: "No playlists yet (click ＋)"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeMicro
                    color: Colors.textMuted
                    leftPadding: 14
                    topPadding: 2
                }
            }

            // Section: AUDIO ENGINE
            Column {
                width: parent.width
                spacing: 3

                Text {
                    text: "AUDIO ENGINE"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                    leftPadding: 12
                    bottomPadding: 4
                }

                Repeater {
                    model: [
                        { id: "equalizer", label: "Equalizer & DSP", icon: "equalizer" },
                        { id: "inspector", label: "Audio Inspector", icon: "inspector" },
                        { id: "settings", label: "Settings", icon: "⚙" }
                    ]

                    delegate: Rectangle {
                        width: parent.width
                        height: 36
                        radius: Theme.cornerRadiusSmall
                        color: root.currentScreen === modelData.id ? Colors.surfaceActive : (navMouse2.containsMouse ? Colors.surfaceHover : "transparent")

                        Behavior on color {
                            ColorAnimation { duration: Motion.durationFast }
                        }

                        MouseArea {
                            id: navMouse2
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.navigationRequested(modelData.id)
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            spacing: 12

                            SoniqIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                name: modelData.icon
                                size: 16
                                color: root.currentScreen === modelData.id ? Colors.accent : Colors.textSecondary
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.label
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: root.currentScreen === modelData.id ? Typography.weightMedium : Typography.weightNormal
                                color: root.currentScreen === modelData.id ? Colors.textPrimary : Colors.textSecondary
                            }
                        }
                    }
                }
            }
        }
    }

    // Bottom Audiophile Hardware Status Card
    SoniqCard {
        id: statusCard
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        height: 60

        Column {
            anchors.centerIn: parent
            spacing: 3
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 6
                Rectangle {
                    width: 7
                    height: 7
                    radius: 3.5
                    anchors.verticalCenter: parent.verticalCenter
                    color: (playbackController.bitPerfectStatus === "Verified") ? Colors.bitPerfectVerified : Colors.success
                }
                Text {
                    text: playbackController.bitPerfectStatus.toUpperCase()
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: (playbackController.bitPerfectStatus === "Verified") ? Colors.bitPerfectVerified : Colors.textPrimary
                }
            }
            Text {
                text: "WASAPI Core Audio"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeMicro
                color: Colors.textMuted
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
