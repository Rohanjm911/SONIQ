import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

Item {
    id: root

    Column {
        anchors.centerIn: parent
        width: Math.min(780, parent.width - 48)
        spacing: root.height > 700 ? 28 : 18

        // Artwork & Dynamic Ambilight Aura Container
        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(420, Math.min(parent.width - 64, Math.max(240, root.height * 0.44)))
            height: width

            // Atmospheric Ambilight Aura Glow
            Rectangle {
                id: ambilightGlow
                anchors.centerIn: parent
                width: parent.width * 1.15
                height: parent.height * 1.15
                radius: parent.width * 0.25
                color: Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.35)
                opacity: playbackController.isPlaying ? 0.8 : 0.0

                Behavior on opacity {
                    NumberAnimation { duration: 600 }
                }

                SequentialAnimation on scale {
                    loops: Animation.Infinite
                    running: playbackController.isPlaying
                    NumberAnimation { to: 1.05; duration: 2500; easing.type: Easing.InOutQuad }
                    NumberAnimation { to: 0.98; duration: 2500; easing.type: Easing.InOutQuad }
                }

                Image {
                    anchors.fill: parent
                    anchors.margins: -10
                    source: (playbackController.trackArtworkPath && playbackController.trackArtworkPath.length > 0)
                            ? ("file:///" + playbackController.trackArtworkPath)
                            : ""
                    fillMode: Image.PreserveAspectCrop
                    opacity: 0.35
                    visible: status === Image.Ready
                }
            }

            // Crisp Foreground Album Artwork Card
            Rectangle {
                anchors.fill: parent
                radius: Theme.cornerRadiusLarge
                color: Colors.surfaceElevated
                border.color: Colors.borderSubtle
                border.width: 1
                clip: true

                Image {
                    anchors.fill: parent
                    source: (playbackController.trackArtworkPath && playbackController.trackArtworkPath.length > 0)
                            ? ("file:///" + playbackController.trackArtworkPath)
                            : ""
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    visible: status === Image.Ready
                }

                Text {
                    anchors.centerIn: parent
                    text: "\u266B"
                    font.pixelSize: 80
                    color: Colors.textMuted
                    visible: !playbackController.trackArtworkPath || playbackController.trackArtworkPath.length === 0
                }
            }
        }

        // Track Information & Audiophile Badges
        Column {
            width: parent.width
            spacing: 6
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                width: parent.width
                text: playbackController.trackTitle
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeTitleLarge
                font.weight: Typography.weightBold
                color: Colors.textPrimary
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: playbackController.trackArtist + " \u2014 " + playbackController.trackAlbum
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeBody
                color: Colors.textSecondary
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 8
                topPadding: 4

                SoniqBadge {
                    badgeText: playbackController.trackFormat.length > 0 ? playbackController.trackFormat : "PCM"
                    variant: "hires"
                }

                SoniqBadge {
                    badgeText: playbackController.bitPerfectStatus.toUpperCase()
                    variant: playbackController.bitPerfectStatus === "Verified" ? "bitperfect" : "standard"
                }
            }
        }

        // Timeline Progress Scrubber
        Column {
            width: parent.width
            spacing: 4

            SoniqSlider {
                width: parent.width
                from: 0.0
                to: Math.max(1.0, playbackController.duration)
                value: playbackController.position
                onMoved: playbackController.seek(value)
            }

            Item {
                width: parent.width
                height: 16

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: playbackController.positionFormatted
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeCaption
                    color: Colors.textMuted
                }

                Text {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: playbackController.durationFormatted
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeCaption
                    color: Colors.textMuted
                }
            }
        }

        // Transport Controls
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 24

            SoniqIconButton {
                iconSymbol: "shuffle"
                iconPixelSize: 18
                isAccent: playbackController.isShuffle
                onClicked: playbackController.toggleShuffle()
            }

            SoniqIconButton {
                iconSymbol: "prev"
                iconPixelSize: 22
                onClicked: playbackController.previous()
            }

            Rectangle {
                width: 56
                height: 56
                radius: Theme.cornerRadiusPill
                color: Colors.accent
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    anchors.centerIn: parent
                    text: playbackController.isPlaying ? "\u23F8" : "\u25B6"
                    font.pixelSize: 20
                    color: Colors.textInverse
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: playbackController.togglePlayPause()
                }
            }

            SoniqIconButton {
                iconSymbol: "next"
                iconPixelSize: 22
                onClicked: playbackController.next()
            }

            SoniqIconButton {
                iconSymbol: "repeat"
                iconPixelSize: 18
                isAccent: playbackController.repeatMode > 0
                onClicked: {
                    var nextMode = (playbackController.repeatMode + 1) % 3;
                    playbackController.setRepeatMode(nextMode);
                }
            }
        }

        // Real-time Audio Spectrum Visualization
        SoniqSpectrum {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 240
            height: 36
            peakL: playbackController.peakLevelL
            peakR: playbackController.peakLevelR
            active: playbackController.isPlaying
        }
    }
}
