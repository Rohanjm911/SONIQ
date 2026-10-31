import QtQuick
import QtQuick.Controls.Basic
import "../themes"

Rectangle {
    id: root

    property int trackNumber: 1
    property int trackId: 0
    property string title: "Track Title"
    property string artist: "Artist Name"
    property string album: "Album Name"
    property string genre: ""
    property int year: 0
    property string filePath: ""
    property string duration: "3:45"
    property string format: "FLAC"
    property string artworkPath: ""
    property bool isPlaying: false
    property bool isFavorite: false
    property bool inPlaylist: false

    signal playClicked()
    signal favoriteClicked()
    signal editTagsClicked()
    signal addToPlaylistClicked()
    signal removeFromPlaylistClicked()
    signal toastRequested(string msg)

    implicitHeight: 52
    width: parent ? parent.width : 600

    radius: Theme.cornerRadiusSmall
    color: {
        if (root.isPlaying) return Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.14);
        if (mouseArea.containsMouse) return Colors.surfaceHover;
        return "transparent";
    }

    Behavior on color {
        ColorAnimation { duration: Motion.durationFast; easing.type: Motion.easingStandard }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.playClicked()
        onDoubleClicked: root.playClicked()
    }

    // 1. Left Group: Number + Thumbnail + Title/Artist
    Row {
        id: leftGroup
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        width: Math.max(180, (parent.width - rightGroup.width - 80) * 0.50)
        spacing: 12

        // Number / Play indicator
        Item {
            width: 28
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter

            Text {
                visible: !mouseArea.containsMouse && !root.isPlaying
                anchors.centerIn: parent
                text: root.trackNumber > 0 ? root.trackNumber.toString() : "-"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeCaption
                color: Colors.textMuted
            }

            SoniqIcon {
                visible: root.isPlaying
                anchors.centerIn: parent
                name: "play"
                size: 14
                color: Colors.accent
            }

            SoniqIconButton {
                visible: mouseArea.containsMouse && !root.isPlaying
                anchors.centerIn: parent
                iconSymbol: "play"
                iconPixelSize: 12
                onClicked: root.playClicked()
            }
        }

        // Mini Artwork Thumbnail
        Rectangle {
            width: 36
            height: 36
            radius: Theme.cornerRadiusSmall
            color: Colors.surfaceElevated
            anchors.verticalCenter: parent.verticalCenter
            clip: true

            Image {
                anchors.fill: parent
                source: root.artworkPath.length > 0 ? ("file:///" + root.artworkPath) : ""
                fillMode: Image.PreserveAspectCrop
                smooth: true
                visible: status === Image.Ready
            }

            SoniqIcon {
                anchors.centerIn: parent
                name: "songs"
                size: 16
                color: Colors.textMuted
                visible: !root.artworkPath || root.artworkPath.length === 0
            }
        }

        // Title and Artist Column
        Column {
            width: parent.width - 80
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            Text {
                width: parent.width
                text: root.title
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeBody
                font.weight: root.isPlaying ? Typography.weightBold : Typography.weightMedium
                color: root.isPlaying ? Colors.accent : Colors.textPrimary
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: root.artist
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeCaption
                color: Colors.textSecondary
                elide: Text.ElideRight
            }
        }
    }

    // 2. Center: Flexible Album Column
    Text {
        id: albumText
        anchors.left: leftGroup.right
        anchors.leftMargin: 16
        anchors.right: rightGroup.left
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        text: root.album
        font.family: Typography.fontFamily
        font.pixelSize: Typography.sizeBody
        color: Colors.textSecondary
        elide: Text.ElideRight
        visible: width > 40
    }

    // 3. Right Group: Format Badge + Favorite + Duration + More Actions
    Row {
        id: rightGroup
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        // Format Badge
        Item {
            width: 62
            height: 24
            anchors.verticalCenter: parent.verticalCenter

            SoniqBadge {
                anchors.centerIn: parent
                badgeText: root.format.toUpperCase()
                variant: {
                    var f = root.format.toUpperCase();
                    if (f === "DSD" || f === "DSF" || f === "DFF") return "dsd";
                    if (f === "FLAC" || f === "WAV" || f === "AIFF" || f === "ALAC") return "hires";
                    return "standard";
                }
            }
        }

        // Favorite Button
        Item {
            width: 28
            height: 28
            anchors.verticalCenter: parent.verticalCenter

            SoniqIconButton {
                anchors.centerIn: parent
                iconSymbol: root.isFavorite ? "heart_filled" : "heart"
                iconPixelSize: 15
                iconColor: root.isFavorite ? Colors.accent : Colors.textMuted
                tooltipText: root.isFavorite ? "Remove from Favorites" : "Add to Favorites"
                onClicked: root.favoriteClicked()
            }
        }

        // Duration
        Text {
            width: 44
            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignRight
            text: root.duration
            font.family: Typography.monospaceFamily
            font.pixelSize: Typography.sizeCaption
            color: Colors.textMuted
        }

        // More Options Button (...)
        Item {
            width: 28
            height: 28
            anchors.verticalCenter: parent.verticalCenter

            SoniqIconButton {
                id: moreOptionsBtn
                anchors.centerIn: parent
                iconSymbol: "•••"
                iconPixelSize: 11
                iconColor: optionsPopup.visible ? Colors.accent : Colors.textMuted
                tooltipText: "Options"
                onClicked: optionsPopup.open()
            }

            Popup {
                id: optionsPopup
                y: 32
                x: -160
                width: 190
                padding: 6
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                background: Rectangle {
                    radius: Theme.cornerRadiusMedium
                    color: Colors.surfaceElevated
                    border.color: Colors.borderMedium
                    border.width: 1
                }
                contentItem: Column {
                    spacing: 2

                    // Play Next
                    Rectangle {
                        width: parent.width
                        height: 32
                        radius: Theme.cornerRadiusSmall
                        color: playNextMouse.containsMouse ? Colors.surfaceHover : "transparent"
                        MouseArea {
                            id: playNextMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                optionsPopup.close();
                                playbackController.enqueueTrack(root.filePath, true);
                            }
                        }
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            spacing: 10
                            SoniqIcon { name: "play"; size: 12; color: Colors.accent; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Play Next"; font.family: Typography.fontFamily; font.pixelSize: Typography.sizeCaption; color: Colors.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }

                    // Add to Queue
                    Rectangle {
                        width: parent.width
                        height: 32
                        radius: Theme.cornerRadiusSmall
                        color: queueMouse.containsMouse ? Colors.surfaceHover : "transparent"
                        MouseArea {
                            id: queueMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                optionsPopup.close();
                                playbackController.enqueueTrack(root.filePath, false);
                            }
                        }
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            spacing: 10
                            SoniqIcon { name: "queue"; size: 12; color: Colors.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Add to Queue"; font.family: Typography.fontFamily; font.pixelSize: Typography.sizeCaption; color: Colors.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }

                    // Add to Playlist
                    Rectangle {
                        width: parent.width
                        height: 32
                        radius: Theme.cornerRadiusSmall
                        color: addPlMouse.containsMouse ? Colors.surfaceHover : "transparent"
                        MouseArea {
                            id: addPlMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                optionsPopup.close();
                                root.addToPlaylistClicked();
                            }
                        }
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            spacing: 10
                            SoniqIcon { name: "add"; size: 12; color: Colors.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Add to Playlist..."; font.family: Typography.fontFamily; font.pixelSize: Typography.sizeCaption; color: Colors.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }

                    // Edit Tags
                    Rectangle {
                        width: parent.width
                        height: 32
                        radius: Theme.cornerRadiusSmall
                        color: editMouse.containsMouse ? Colors.surfaceHover : "transparent"
                        MouseArea {
                            id: editMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                optionsPopup.close();
                                root.editTagsClicked();
                            }
                        }
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            spacing: 10
                            SoniqIcon { name: "edit"; size: 12; color: Colors.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Edit Metadata Tags..."; font.family: Typography.fontFamily; font.pixelSize: Typography.sizeCaption; color: Colors.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }

                    // Inspect Audio
                    Rectangle {
                        width: parent.width
                        height: 32
                        radius: Theme.cornerRadiusSmall
                        color: inspMouse.containsMouse ? Colors.surfaceHover : "transparent"
                        MouseArea {
                            id: inspMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                optionsPopup.close();
                                audioInspector.inspectFile(root.filePath);
                                if (window) window.activeScreen = "inspector";
                            }
                        }
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            spacing: 10
                            SoniqIcon { name: "inspector"; size: 12; color: Colors.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Inspect Audio File"; font.family: Typography.fontFamily; font.pixelSize: Typography.sizeCaption; color: Colors.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }

                    // Remove from Playlist (if in playlist view)
                    Rectangle {
                        visible: root.inPlaylist
                        width: parent.width
                        height: 32
                        radius: Theme.cornerRadiusSmall
                        color: remMouse.containsMouse ? Qt.rgba(1, 0, 0, 0.15) : "transparent"
                        MouseArea {
                            id: remMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                optionsPopup.close();
                                root.removeFromPlaylistClicked();
                            }
                        }
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            spacing: 10
                            SoniqIcon { name: "trash"; size: 12; color: Colors.error; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Remove from Playlist"; font.family: Typography.fontFamily; font.pixelSize: Typography.sizeCaption; color: Colors.error; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }
                }
            }
        }
    }
}
