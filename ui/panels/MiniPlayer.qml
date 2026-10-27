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
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            width: 260
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

                Text {
                    anchors.centerIn: parent
                    text: "\u266B"
                    font.pixelSize: 22
                    color: Colors.textMuted
                    visible: !playbackController.trackArtworkPath || playbackController.trackArtworkPath.length === 0
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.expandRequested()
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 60
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

                Text {
                    width: parent.width
                    text: playbackController.trackArtist
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeCaption
                    color: Colors.textSecondary
                    elide: Text.ElideRight
                }
            }
        }

        // 2. Center Group: Transport Controls & Timeline Scrubber (Centered in window)
        Column {
            id: centerGroup
            anchors.centerIn: parent
            width: Math.min(680, Math.max(280, parent.width - leftGroup.width - rightGroup.width - 48))
            spacing: 2

            // Transport buttons
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 12

                SoniqIconButton {
                    iconSymbol: "shuffle"
                    iconPixelSize: 14
                    isAccent: playbackController.isShuffle
                    tooltipText: "Shuffle"
                    onClicked: playbackController.toggleShuffle()
                }

                SoniqIconButton {
                    iconSymbol: "prev"
                    iconPixelSize: 16
                    tooltipText: "Previous"
                    onClicked: playbackController.previous()
                }

                Rectangle {
                    width: 36
                    height: 36
                    radius: Theme.cornerRadiusPill
                    color: Colors.accent
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.centerIn: parent
                        text: playbackController.isPlaying ? "\u23F8" : "\u25B6"
                        font.pixelSize: 14
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
                    iconSymbol: "repeat"
                    iconPixelSize: 14
                    isAccent: playbackController.repeatMode > 0
                    tooltipText: playbackController.repeatMode === 1 ? "Repeat Track (Mode 1)" : (playbackController.repeatMode === 2 ? "Repeat All (Mode 2)" : "Repeat Off")
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

        // 3. Right Group: Audio Telemetry, Volume, Queue & Fullscreen Controls
        Row {
            id: rightGroup
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            // Real-time Spectrum Visualizer
            SoniqSpectrum {
                anchors.verticalCenter: parent.verticalCenter
                width: 60
                height: 24
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
                width: 80
                anchors.verticalCenter: parent.verticalCenter
                from: 0.0
                to: 1.0
                value: playbackController.volume
                onMoved: playbackController.setVolume(value)
            }

            // Queue Drawer Toggle
            SoniqIconButton {
                anchors.verticalCenter: parent.verticalCenter
                iconSymbol: "☰"
                iconPixelSize: 16
                tooltipText: "Playback Queue"
                onClicked: root.queueToggleRequested()
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
