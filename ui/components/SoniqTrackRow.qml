import QtQuick
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

    implicitHeight: 48
    width: parent ? parent.width : 600

    radius: Theme.cornerRadiusSmall
    color: {
        if (root.isPlaying) return Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.12);
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
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeBody
                color: Colors.textMuted
            }

            Text {
                visible: root.isPlaying
                anchors.centerIn: parent
                text: "▶"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeBody
                color: Colors.accent
            }

            SoniqIconButton {
                visible: mouseArea.containsMouse && !root.isPlaying
                anchors.centerIn: parent
                iconSymbol: "▶"
                iconPixelSize: 12
                onClicked: root.playClicked()
            }
        }

        // Mini Artwork Thumbnail
        Rectangle {
            width: 32
            height: 32
            radius: Theme.cornerRadiusSmall
            color: Colors.surfaceElevated
            anchors.verticalCenter: parent.verticalCenter
            visible: root.artworkPath.length > 0
            clip: true

            Image {
                anchors.fill: parent
                source: root.artworkPath.length > 0 ? ("file:///" + root.artworkPath) : ""
                fillMode: Image.PreserveAspectCrop
                smooth: true
            }
        }

        // Title and Artist Column
        Column {
            width: parent.width - 28 - (root.artworkPath.length > 0 ? 44 : 0)
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            Text {
                width: parent.width
                text: root.title
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeBody
                font.weight: Typography.weightMedium
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

    // 3. Right Group: Format Badge + Favorite + Duration (Pinned to right edge)
    Row {
        id: rightGroup
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 12

        // Format Badge
        Item {
            width: 58
            height: 24
            anchors.verticalCenter: parent.verticalCenter

            SoniqBadge {
                anchors.centerIn: parent
                badgeText: root.format.toUpperCase()
                variant: (root.format.toUpperCase() === "FLAC" || root.format.toUpperCase() === "WAV") ? "hires" : "standard"
            }
        }

        // Favorite Button
        Item {
            width: 28
            height: 28
            anchors.verticalCenter: parent.verticalCenter

            SoniqIconButton {
                anchors.centerIn: parent
                iconSymbol: root.isFavorite ? "♥" : "♡"
                iconPixelSize: 14
                iconColor: root.isFavorite ? Colors.accent : Colors.textMuted
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
            font.pixelSize: Typography.sizeBody
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
                onClicked: optionsPopup.open()
            }

            Popup {
                id: optionsPopup
                y: 30
                x: -140
                width: 176
                padding: 4
                closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                background: Rectangle {
                    radius: Theme.cornerRadiusSmall
                    color: Colors.surfaceElevated
                    border.color: Colors.borderSubtle
                    border.width: 1
                }
                contentItem: Column {
                    spacing: 2

                    // Option: Add to Playlist
                    Rectangle {
                        width: parent.width
                        height: 30
                        radius: 4
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
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8
                            Text { text: "＋"; font.pixelSize: 12; color: Colors.accent; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Add to Playlist..."; font.family: Typography.fontFamily; font.pixelSize: Typography.sizeCaption; color: Colors.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }

                    // Option: Edit Tags
                    Rectangle {
                        width: parent.width
                        height: 30
                        radius: 4
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
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8
                            Text { text: "✎"; font.pixelSize: 12; color: Colors.accent; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Edit Metadata Tags..."; font.family: Typography.fontFamily; font.pixelSize: Typography.sizeCaption; color: Colors.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }

                    // Option: Remove from Playlist (if currently in a playlist view)
                    Rectangle {
                        visible: root.inPlaylist
                        width: parent.width
                        height: 30
                        radius: 4
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
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8
                            Text { text: "✕"; font.pixelSize: 11; color: "#EF4444"; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Remove from Playlist"; font.family: Typography.fontFamily; font.pixelSize: Typography.sizeCaption; color: "#EF4444"; anchors.verticalCenter: parent.verticalCenter }
                        }
                    }
                }
            }
        }
    }
}
