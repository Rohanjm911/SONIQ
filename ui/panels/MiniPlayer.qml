import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

Rectangle {
    id: root

    signal expandRequested()
    signal queueToggleRequested()
    signal fullscreenToggleRequested()

    height: Theme.miniPlayerHeight
    color: Colors.surface
    border.color: Colors.borderSubtle
    border.width: 1

    Item {
        anchors.fill: parent

        // 1. Left Group: Artwork Thumbnail & Track Details
        Row {
            id: leftGroup
            anchors.left: parent.left
            anchors.leftMargin: Theme.spacingM
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(280, Math.max(180, parent.width * 0.25))
            height: parent.height
            spacing: 12

            Rectangle {
                width: 48
                height: 48
                radius: Theme.cornerRadiusSmall
                color: Colors.surfaceElevated
                anchors.verticalCenter: parent.verticalCenter
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

                SoniqIcon {
                    anchors.centerIn: parent
                    name: "songs"
                    size: 22
                    color: Colors.textMuted
                    visible: !playbackController.trackArtworkPath || playbackController.trackArtworkPath.length === 0
                }

                // Hover overlay with expand hint
                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(0, 0, 0, 0.4)
                    opacity: artMouse.containsMouse ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: Motion.durationFast } }

                    SoniqIcon {
                        anchors.centerIn: parent
                        name: "fullscreen"
                        size: 14
                        color: "#FFFFFF"
                    }
                }

                MouseArea {
                    id: artMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.expandRequested()
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 64
                spacing: 2

                Text {
                    width: parent.width
                    text: playbackController.trackTitle
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeBody
                    font.weight: Typography.weightMedium
                    color: Colors.textPrimary
                    elide: Text.ElideRight
                }

                Row {
                    width: parent.width
                    spacing: 6

                    Text {
                        text: playbackController.trackArtist
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeCaption
                        color: Colors.textSecondary
                        elide: Text.ElideRight
                        width: Math.min(implicitWidth, parent.width - 48)
                    }

                    SoniqBadge {
                        visible: playbackController.trackFormat.length > 0 && playbackController.trackFormat !== "None"
                        badgeText: playbackController.trackFormat.slice(0, 4).toUpperCase()
                        variant: (playbackController.bitPerfectStatus === "Verified") ? "bitperfect" : "standard"
                    }
                }
            }
        }

        // 2. Center Group: Transport Controls & Timeline Scrubber (Centered in window)
        Column {
            id: centerGroup
            anchors.centerIn: parent
            width: Math.min(640, Math.max(260, parent.width - leftGroup.width - rightGroup.width - 32))
            spacing: 2

            // Transport buttons
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 12

                SoniqIconButton {
                    iconSymbol: "shuffle"
                    iconPixelSize: 14
                    isAccent: playbackController.isShuffle
                    tooltipText: playbackController.isShuffle ? "Shuffle On" : "Shuffle Off"
                    onClicked: playbackController.toggleShuffle()
                }

                SoniqIconButton {
                    iconSymbol: "prev"
                    iconPixelSize: 16
                    tooltipText: "Previous"
                    onClicked: playbackController.previous()
                }

                Rectangle {
                    width: 38
                    height: 38
                    radius: Theme.cornerRadiusPill
                    color: Colors.accent
                    anchors.verticalCenter: parent.verticalCenter

                    SoniqIcon {
                        anchors.centerIn: parent
                        name: playbackController.isPlaying ? "pause" : "play"
                        size: 15
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
                    iconPixelSize: 16
                    tooltipText: "Next"
                    onClicked: playbackController.next()
                }

                SoniqIconButton {
                    iconSymbol: playbackController.repeatMode === 1 ? "repeat_one" : "repeat"
                    iconPixelSize: 14
                    isAccent: playbackController.repeatMode > 0
                    tooltipText: playbackController.repeatMode === 1 ? "Repeat Track" : (playbackController.repeatMode === 2 ? "Repeat All" : "Repeat Off")
                    onClicked: {
                        var nextMode = (playbackController.repeatMode + 1) % 3;
                        playbackController.setRepeatMode(nextMode);
                    }
                }
            }

            // Timeline Scrubbing Row
            Row {
                width: parent.width
                spacing: 8
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    id: posText
                    text: playbackController.positionFormatted
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    color: Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }

                SoniqSlider {
                    width: Math.max(100, parent.width - posText.implicitWidth - durText.implicitWidth - 24)
                    anchors.verticalCenter: parent.verticalCenter
                    from: 0.0
                    to: Math.max(1.0, playbackController.duration)
                    value: playbackController.position
                    onMoved: playbackController.seek(value)
                }

                Text {
                    id: durText
                    text: playbackController.durationFormatted
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    color: Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // 3. Right Group: Spectrum, Volume, Queue & Window Controls
        Row {
            id: rightGroup
            anchors.right: parent.right
            anchors.rightMargin: Theme.spacingM
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            // Mini Audio Spectrum
            SoniqSpectrum {
                anchors.verticalCenter: parent.verticalCenter
                width: 50
                height: 22
                peakL: playbackController.peakLevelL
                peakR: playbackController.peakLevelR
                active: playbackController.isPlaying
            }

            // Volume Mute Button
            SoniqIconButton {
                anchors.verticalCenter: parent.verticalCenter
                iconSymbol: playbackController.isMuted ? "mute" : "volume"
                iconPixelSize: 15
                tooltipText: playbackController.isMuted ? "Unmute" : "Mute"
                onClicked: playbackController.setMuted(!playbackController.isMuted)
            }

            // Volume Slider
            SoniqSlider {
                width: 76
                anchors.verticalCenter: parent.verticalCenter
                from: 0.0
                to: 1.0
                value: playbackController.volume
                onMoved: playbackController.setVolume(value)
            }

            // Queue Toggle Button
            SoniqIconButton {
                anchors.verticalCenter: parent.verticalCenter
                iconSymbol: "queue"
                iconPixelSize: 15
                isAccent: window && window.queueOpen
                tooltipText: "Playing Queue"
                onClicked: root.queueToggleRequested()
            }

            // Expand to Now Playing
            SoniqIconButton {
                anchors.verticalCenter: parent.verticalCenter
                iconSymbol: "lyrics"
                iconPixelSize: 15
                isAccent: window && window.activeScreen === "nowplaying"
                tooltipText: "Now Playing & Lyrics"
                onClicked: root.expandRequested()
            }

            // Fullscreen Toggle Button
            SoniqIconButton {
                anchors.verticalCenter: parent.verticalCenter
                iconSymbol: (window && window.visibility === Window.FullScreen) ? "exit_fullscreen" : "fullscreen"
                iconPixelSize: 15
                tooltipText: (window && window.visibility === Window.FullScreen) ? "Exit Fullscreen (F11/Esc)" : "Fullscreen (F11)"
                onClicked: root.fullscreenToggleRequested()
            }
        }
    }
}
