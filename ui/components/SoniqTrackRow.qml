import QtQuick
import "../themes"

Rectangle {
    id: root

    property int trackNumber: 1
    property string title: "Track Title"
    property string artist: "Artist Name"
    property string album: "Album Name"
    property string duration: "3:45"
    property string format: "FLAC"
    property string artworkPath: ""
    property bool isPlaying: false
    property bool isFavorite: false

    signal playClicked()
    signal favoriteClicked()

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
    }
}
