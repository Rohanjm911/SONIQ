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
        anchors.margins: Theme.spacingM
        height: 40
        spacing: 12

        Rectangle {
            width: 38
            height: 38
            radius: Theme.cornerRadiusMedium
            color: Colors.surfaceActive
            border.color: Colors.borderSubtle
            border.width: 1
            clip: true

            Image {
                anchors.centerIn: parent
                width: 30
                height: 30
                source: "qrc:/ui/assets/logo_icon.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
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
                font.letterSpacing: Typography.letterSpacingWide
                color: Colors.textPrimary
            }
            Text {
                text: "PURE SOUND"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                font.letterSpacing: Typography.letterSpacingCaps
                color: Colors.textMuted
            }
        }
    }

    // Scrollable Navigation Items
    Flickable {
        id: navFlickable
        anchors.top: brandingRow.bottom
        anchors.topMargin: Theme.spacingXS
        anchors.bottom: statusCard.top
        anchors.bottomMargin: Theme.spacingXS
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Theme.spacingS
        anchors.rightMargin: Theme.spacingS
        contentHeight: navColumn.height
        clip: true

        Column {
            id: navColumn
            width: parent.width
            spacing: 16

            // Section: LISTEN NOW
            Column {
                width: parent.width
                spacing: 2

                Text {
                    text: "LISTEN NOW"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    font.letterSpacing: Typography.letterSpacingCaps
                    color: Colors.textMuted
                    leftPadding: 10
                    bottomPadding: 4
                }

                Repeater {
                    model: [
                        { id: "home", label: "Home", icon: "home" },
                        { id: "nowplaying", label: "Now Playing", icon: "play" }
                    ]

                    delegate: Rectangle {
                        width: parent.width
                        height: 34
                        radius: Theme.cornerRadiusSmall
                        property bool isSelected: (root.currentScreen === modelData.id && libraryModel.activePlaylistId <= 0 && libraryModel.activeFilterType === "")
                        color: isSelected ? Colors.surfaceActive : (navMouse.containsMouse ? Colors.surfaceHover : "transparent")

                        Behavior on color {
                            ColorAnimation { duration: Motion.durationFast }
                        }

                        MouseArea {
                            id: navMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (modelData.id === "home") {
                                    libraryModel.clearFilter();
                                }
                                root.navigationRequested(modelData.id);
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            spacing: 10

                            SoniqIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                name: modelData.icon
                                size: 15
                                color: isSelected ? Colors.accent : Colors.textSecondary
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.label
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: isSelected ? Typography.weightMedium : Typography.weightRegular
                                color: isSelected ? Colors.textPrimary : Colors.textSecondary
                            }
                        }
                    }
                }
            }

            // Section: LIBRARY
            Column {
                width: parent.width
                spacing: 2

                Text {
                    text: "LIBRARY"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    font.letterSpacing: Typography.letterSpacingCaps
                    color: Colors.textMuted
                    leftPadding: 10
                    bottomPadding: 4
                }

                Repeater {
                    model: [
                        { id: "songs", filterType: "", label: "Songs", icon: "songs" },
                        { id: "songs", filterType: "albums_view", label: "Albums", icon: "albums" },
                        { id: "songs", filterType: "artists_view", label: "Artists", icon: "artists" },
                        { id: "songs", filterType: "genres_view", label: "Genres", icon: "genres" },
                        { id: "songs", filterType: "favorites", label: "Favorites", icon: "favorites" },
                        { id: "songs", filterType: "recent_played", label: "Recently Played", icon: "recent" }
                    ]

                    delegate: Rectangle {
                        width: parent.width
                        height: 34
                        radius: Theme.cornerRadiusSmall
                        property bool isSelected: (root.currentScreen === "songs" && libraryModel.activePlaylistId <= 0 &&
                            ((modelData.filterType === "" && libraryModel.activeFilterType === "") ||
                             (modelData.filterType === libraryModel.activeFilterType)))

                        color: isSelected ? Colors.surfaceActive : (libMouse.containsMouse ? Colors.surfaceHover : "transparent")

                        Behavior on color {
                            ColorAnimation { duration: Motion.durationFast }
                        }

                        MouseArea {
                            id: libMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (modelData.filterType === "") {
                                    libraryModel.clearFilter();
                                } else if (modelData.filterType === "favorites") {
                                    libraryModel.filterFavorites();
                                } else if (modelData.filterType === "recent_played") {
                                    libraryModel.filterRecentlyPlayed();
                                } else if (modelData.filterType === "albums_view") {
                                    window.activeLibraryTab = "albums";
                                    libraryModel.clearFilter();
                                } else if (modelData.filterType === "artists_view") {
                                    window.activeLibraryTab = "artists";
                                    libraryModel.clearFilter();
                                } else if (modelData.filterType === "genres_view") {
                                    window.activeLibraryTab = "genres";
                                    libraryModel.clearFilter();
                                }
                                if (modelData.filterType !== "albums_view" && modelData.filterType !== "artists_view" && modelData.filterType !== "genres_view") {
                                    window.activeLibraryTab = "songs";
                                }
                                root.navigationRequested("songs");
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            spacing: 10

                            SoniqIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                name: modelData.icon
                                size: 15
                                color: isSelected ? Colors.accent : Colors.textSecondary
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.label
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: isSelected ? Typography.weightMedium : Typography.weightRegular
                                color: isSelected ? Colors.textPrimary : Colors.textSecondary
                            }
                        }
                    }
                }
            }

            // Section: PLAYLISTS
            Column {
                width: parent.width
                spacing: 2

                Item {
                    width: parent.width
                    height: 24

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: "PLAYLISTS"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        font.letterSpacing: Typography.letterSpacingCaps
                        color: Colors.textMuted
                    }

                    SoniqIconButton {
                        anchors.right: parent.right
                        anchors.rightMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        iconSymbol: "add"
                        iconPixelSize: 13
                        tooltipText: "Create Playlist"
                        onClicked: createPlaylistDialog.open()
                    }
                }

                Repeater {
                    model: libraryModel.playlists

                    delegate: Rectangle {
                        width: parent.width
                        height: 32
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
                                window.activeLibraryTab = "songs";
                                libraryModel.filterByPlaylist(modelData.id, modelData.name);
                                root.navigationRequested("songs");
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 8
                            spacing: 8

                            SoniqIcon {
                                name: "playlists"
                                size: 13
                                color: isCurrentPl ? Colors.accent : Colors.textMuted
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.name
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeCaption
                                font.weight: isCurrentPl ? Typography.weightMedium : Typography.weightRegular
                                color: isCurrentPl ? Colors.textPrimary : Colors.textSecondary
                                elide: Text.ElideRight
                                width: Math.max(50, parent.width - 66)
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
                    text: "No playlists yet"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeMicro
                    color: Colors.textMuted
                    leftPadding: 12
                    topPadding: 2
                }
            }

            // Section: AUDIO ENGINE
            Column {
                width: parent.width
                spacing: 2

                Text {
                    text: "AUDIO ENGINE"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    font.letterSpacing: Typography.letterSpacingCaps
                    color: Colors.textMuted
                    leftPadding: 10
                    bottomPadding: 4
                }

                Repeater {
                    model: [
                        { id: "equalizer", label: "Equalizer & DSP", icon: "equalizer" },
                        { id: "inspector", label: "Audio Inspector", icon: "inspector" },
                        { id: "settings", label: "Settings", icon: "settings" }
                    ]

                    delegate: Rectangle {
                        width: parent.width
                        height: 34
                        radius: Theme.cornerRadiusSmall
                        property bool isSelected: (root.currentScreen === modelData.id)
                        color: isSelected ? Colors.surfaceActive : (navMouse2.containsMouse ? Colors.surfaceHover : "transparent")

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
                            anchors.leftMargin: 10
                            spacing: 10

                            SoniqIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                name: modelData.icon
                                size: 15
                                color: isSelected ? Colors.accent : Colors.textSecondary
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.label
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: isSelected ? Typography.weightMedium : Typography.weightRegular
                                color: isSelected ? Colors.textPrimary : Colors.textSecondary
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
        anchors.margins: Theme.spacingS
        height: 58

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
