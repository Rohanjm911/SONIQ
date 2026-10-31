import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

Item {
    id: root

    // View sub-modes: "artwork", "lyrics", "visualizer", "specs"
    property string activeMode: "artwork"
    property string visualizerMode: "bars"

    Column {
        anchors.fill: parent
        anchors.margins: Theme.spacingL
        spacing: 14

        // 1. Top Mode Selector Bar (Cover | Lyrics | Visualizer | Stream Specs)
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 36
            width: Math.min(380, parent.width - 40)
            radius: Theme.cornerRadiusSmall
            color: Colors.surfaceElevated
            border.color: Colors.borderSubtle
            border.width: 1

            Row {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                Repeater {
                    model: [
                        { id: "artwork", label: "Cover", icon: "albums" },
                        { id: "lyrics", label: "Lyrics", icon: "lyrics" },
                        { id: "visualizer", label: "Visualizer", icon: "visualizer" },
                        { id: "specs", label: "Audio Specs", icon: "inspector" }
                    ]

                    delegate: Rectangle {
                        width: (parent.width - 6) / 4
                        height: parent.height
                        radius: Theme.cornerRadiusSmall - 1
                        property bool isCurrent: (root.activeMode === modelData.id)
                        color: isCurrent ? Colors.surfaceCard : (modeMouse.containsMouse ? Colors.surfaceHover : "transparent")

                        Behavior on color { ColorAnimation { duration: Motion.durationFast } }

                        MouseArea {
                            id: modeMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.activeMode = modelData.id
                        }

                        Row {
                            anchors.centerIn: parent
                            spacing: 6

                            SoniqIcon {
                                name: modelData.icon
                                size: 12
                                color: isCurrent ? Colors.accent : Colors.textMuted
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: modelData.label
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeCaption
                                font.weight: isCurrent ? Typography.weightMedium : Typography.weightRegular
                                color: isCurrent ? Colors.textPrimary : Colors.textSecondary
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }
        }

        // 2. Center Content Area (Dynamic based on activeMode)
        Item {
            width: parent.width
            height: parent.height - 180
            clip: true

            // === 2A. COVER ARTWORK VIEW ===
            Item {
                anchors.fill: parent
                visible: root.activeMode === "artwork"

                Column {
                    anchors.centerIn: parent
                    spacing: 16
                    width: Math.min(540, parent.width - 32)

                    // Large Album Artwork Container
                    Item {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Math.min(320, Math.min(parent.width - 40, parent.height * 0.58))
                        height: width

                        // Subtle Ambient Halo Glow (Preserving contrast, low GPU cost)
                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width * 1.08
                            height: parent.height * 1.08
                            radius: Theme.cornerRadiusLarge * 1.5
                            color: Colors.accentMuted
                            opacity: playbackController.isPlaying ? 0.6 : 0.0
                            Behavior on opacity { NumberAnimation { duration: Motion.durationSlow } }
                        }

                        // Crisp Album Artwork Card
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

                            SoniqIcon {
                                anchors.centerIn: parent
                                name: "songs"
                                size: 64
                                color: Colors.textMuted
                                visible: !playbackController.trackArtworkPath || playbackController.trackArtworkPath.length === 0
                            }
                        }
                    }

                    // Track Information & Audiophile Badges
                    Column {
                        width: parent.width
                        spacing: 4
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
                            text: playbackController.trackArtist + (playbackController.trackAlbum.length > 0 ? (" — " + playbackController.trackAlbum) : "")
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
                                badgeText: playbackController.trackFormat.length > 0 && playbackController.trackFormat !== "None"
                                           ? playbackController.trackFormat.slice(0, 10).toUpperCase()
                                           : "PCM"
                                variant: (playbackController.trackFormat.toUpperCase().indexOf("DSD") !== -1) ? "dsd" : "hires"
                            }

                            SoniqBadge {
                                badgeText: playbackController.bitPerfectStatus.toUpperCase()
                                variant: playbackController.bitPerfectStatus === "Verified" ? "bitperfect" : "standard"
                            }
                        }
                    }
                }
            }

            // === 2B. LYRICS VIEW (Apple Music Style Synced Lyrics) ===
            Item {
                anchors.fill: parent
                visible: root.activeMode === "lyrics"

                // Synced Lyrics List
                ListView {
                    id: lyricsList
                    anchors.fill: parent
                    anchors.margins: 20
                    clip: true
                    visible: playbackController.hasLyrics && playbackController.isSyncedLyrics
                    model: playbackController.syncedLyrics
                    spacing: 16

                    property int currentIndex: {
                        var pos = playbackController.position;
                        var items = playbackController.syncedLyrics;
                        if (!items || items.length === 0) return -1;
                        var activeIdx = 0;
                        for (var i = 0; i < items.length; i++) {
                            if (items[i].time <= pos) {
                                activeIdx = i;
                            } else {
                                break;
                            }
                        }
                        return activeIdx;
                    }

                    onCurrentIndexChanged: {
                        if (currentIndex >= 0 && !lyricsList.moving) {
                            lyricsList.positionViewAtIndex(currentIndex, ListView.Center);
                        }
                    }

                    delegate: Item {
                        required property int index
                        required property var modelData
                        width: lyricsList.width
                        height: lyricLineText.implicitHeight + 12

                        property bool isCurrentLine: (lyricsList.currentIndex === index)

                        Text {
                            id: lyricLineText
                            width: parent.width
                            text: modelData.text
                            font.family: Typography.fontFamily
                            font.pixelSize: isCurrentLine ? Typography.sizeTitleMedium : Typography.sizeBody
                            font.weight: isCurrentLine ? Typography.weightBold : Typography.weightRegular
                            color: isCurrentLine ? Colors.textPrimary : Colors.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap

                            Behavior on font.pixelSize { NumberAnimation { duration: Motion.durationNormal } }
                            Behavior on color { ColorAnimation { duration: Motion.durationFast } }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: playbackController.seek(modelData.time)
                        }
                    }
                }

                // Plain Text Lyrics Fallback
                Flickable {
                    id: plainLyricsFlick
                    anchors.fill: parent
                    anchors.margins: 24
                    contentWidth: width
                    contentHeight: plainLyricsText.implicitHeight
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    visible: playbackController.hasLyrics && !playbackController.isSyncedLyrics

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    Text {
                        id: plainLyricsText
                        width: plainLyricsFlick.width
                        text: playbackController.lyricsText
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        color: Colors.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        lineHeight: 1.5
                    }
                }

                // Empty State when no lyrics found
                Item {
                    anchors.centerIn: parent
                    width: parent.width - 40
                    visible: !playbackController.hasLyrics

                    Column {
                        anchors.centerIn: parent
                        spacing: 12

                        SoniqIcon {
                            anchors.horizontalCenter: parent.horizontalCenter
                            name: "lyrics"
                            size: 40
                            color: Colors.textMuted
                        }

                        Text {
                            text: "No Lyrics Available"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleSmall
                            font.weight: Typography.weightBold
                            color: Colors.textPrimary
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "SONIQ automatically displays synchronized lyrics when a .lrc or .txt file is placed in the album folder."
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            color: Colors.textSecondary
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            width: Math.min(360, parent.parent.width)
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }

            // === 2C. VISUALIZER VIEW ===
            Item {
                anchors.fill: parent
                visible: root.activeMode === "visualizer"

                Column {
                    anchors.fill: parent
                    spacing: 14

                    // Visualizer Mode Selector (Bars | Waveform | Circular | Off)
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 8

                        Repeater {
                            model: [
                                { id: "bars", label: "Spectrum Bars" },
                                { id: "waveform", label: "Oscilloscope Waveform" },
                                { id: "circular", label: "Circular Radar" },
                                { id: "off", label: "Off" }
                            ]

                            delegate: SoniqButton {
                                variant: (root.visualizerMode === modelData.id) ? "primary" : "secondary"
                                text: modelData.label
                                customRadius: Theme.cornerRadiusPill
                                implicitHeight: 30
                                onClicked: root.visualizerMode = modelData.id
                            }
                        }
                    }

                    // Main Visualizer Canvas
                    SoniqCard {
                        width: parent.width
                        height: parent.height - 48
                        color: Colors.surfaceCard

                        SoniqVisualizer {
                            anchors.fill: parent
                            anchors.margins: 20
                            mode: root.visualizerMode
                            active: playbackController.isPlaying
                            peakL: playbackController.peakLevelL
                            peakR: playbackController.peakLevelR
                            waveformData: playbackController.waveform
                            visualizerColor: Colors.accent
                        }
                    }
                }
            }

            // === 2D. AUDIO SPECS TELEMETRY VIEW ===
            Item {
                anchors.fill: parent
                visible: root.activeMode === "specs"

                Flickable {
                    id: specsFlick
                    anchors.fill: parent
                    contentWidth: width
                    contentHeight: specsCol.implicitHeight + 32
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    Column {
                        id: specsCol
                        width: Math.min(680, Math.max(420, specsFlick.width - 32))
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 16

                        SoniqCard {
                            width: parent.width
                            height: 200
                            color: Colors.surfaceCard

                            Column {
                                anchors.fill: parent
                                anchors.margins: 18
                                spacing: 14

                                Text {
                                    text: "REAL-TIME STREAM TELEMETRY"
                                    font.family: Typography.monospaceFamily
                                    font.pixelSize: Typography.sizeMicro
                                    font.weight: Typography.weightBold
                                    font.letterSpacing: Typography.letterSpacingCaps
                                    color: Colors.textMuted
                                }

                                Item {
                                    width: parent.width
                                    height: 84

                                    // Left Column: Format, Output Sink, DSP Status
                                    Column {
                                        anchors.left: parent.left
                                        anchors.right: parent.horizontalCenter
                                        anchors.rightMargin: 12
                                        spacing: 8

                                        Row {
                                            width: parent.width
                                            spacing: 6
                                            Text {
                                                text: "Format:"
                                                font.family: Typography.fontFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: Colors.textMuted
                                                width: 76
                                            }
                                            Text {
                                                text: playbackController.trackFormat.length > 0 ? playbackController.trackFormat : "None"
                                                font.family: Typography.monospaceFamily
                                                font.pixelSize: Typography.sizeCaption
                                                font.weight: Typography.weightMedium
                                                color: Colors.textPrimary
                                                elide: Text.ElideRight
                                                width: parent.width - 82
                                            }
                                        }

                                        Row {
                                            width: parent.width
                                            spacing: 6
                                            Text {
                                                text: "Output Sink:"
                                                font.family: Typography.fontFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: Colors.textMuted
                                                width: 76
                                            }
                                            Text {
                                                text: "WASAPI Core Audio (Windows)"
                                                font.family: Typography.monospaceFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: Colors.textPrimary
                                                elide: Text.ElideRight
                                                width: parent.width - 82
                                            }
                                        }

                                        Row {
                                            width: parent.width
                                            spacing: 6
                                            Text {
                                                text: "DSP Status:"
                                                font.family: Typography.fontFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: Colors.textMuted
                                                width: 76
                                            }
                                            Text {
                                                text: dspService.enabled ? "Active" : "Bypass (Bit-Perfect)"
                                                font.family: Typography.monospaceFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: dspService.enabled ? Colors.warning : Colors.success
                                                elide: Text.ElideRight
                                                width: parent.width - 82
                                            }
                                        }
                                    }

                                    // Right Column: Bit-Perfect, Gapless, File Path
                                    Column {
                                        anchors.left: parent.horizontalCenter
                                        anchors.leftMargin: 12
                                        anchors.right: parent.right
                                        spacing: 8

                                        Row {
                                            width: parent.width
                                            spacing: 6
                                            Text {
                                                text: "Bit-Perfect:"
                                                font.family: Typography.fontFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: Colors.textMuted
                                                width: 76
                                            }
                                            Text {
                                                text: playbackController.bitPerfectStatus
                                                font.family: Typography.monospaceFamily
                                                font.pixelSize: Typography.sizeCaption
                                                font.weight: Typography.weightBold
                                                color: playbackController.bitPerfectStatus === "Verified" ? Colors.bitPerfectVerified : Colors.textPrimary
                                                elide: Text.ElideRight
                                                width: parent.width - 82
                                            }
                                        }

                                        Row {
                                            width: parent.width
                                            spacing: 6
                                            Text {
                                                text: "Gapless:"
                                                font.family: Typography.fontFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: Colors.textMuted
                                                width: 76
                                            }
                                            Text {
                                                text: "Enabled (Lookahead Ready)"
                                                font.family: Typography.monospaceFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: Colors.textPrimary
                                                elide: Text.ElideRight
                                                width: parent.width - 82
                                            }
                                        }

                                        Row {
                                            width: parent.width
                                            spacing: 6
                                            Text {
                                                text: "File Path:"
                                                font.family: Typography.fontFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: Colors.textMuted
                                                width: 76
                                            }
                                            Text {
                                                text: playbackController.trackFilePath.length > 0 ? playbackController.trackFilePath : "No track loaded"
                                                font.family: Typography.monospaceFamily
                                                font.pixelSize: Typography.sizeCaption
                                                color: Colors.textSecondary
                                                elide: Text.ElideMiddle
                                                width: parent.width - 82
                                            }
                                        }
                                    }
                                }

                                SoniqButton {
                                    text: "Open Complete Audio Inspector →"
                                    iconText: "inspector"
                                    variant: "secondary"
                                    onClicked: {
                                        audioInspector.inspectFile(playbackController.trackFilePath);
                                        if (window) window.activeScreen = "inspector";
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // 3. Bottom Transport Controls & Timeline Scrubber
        Column {
            width: Math.min(620, parent.width - 32)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

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

            // Transport Control Buttons (Shuffle, Prev, Play/Pause, Next, Repeat)
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 22

                SoniqIconButton {
                    iconSymbol: "shuffle"
                    iconPixelSize: 17
                    isAccent: playbackController.isShuffle
                    tooltipText: playbackController.isShuffle ? "Shuffle On" : "Shuffle Off"
                    onClicked: playbackController.toggleShuffle()
                }

                SoniqIconButton {
                    iconSymbol: "prev"
                    iconPixelSize: 20
                    tooltipText: "Previous Track"
                    onClicked: playbackController.previous()
                }

                Rectangle {
                    width: 52
                    height: 52
                    radius: Theme.cornerRadiusPill
                    color: Colors.accent
                    anchors.verticalCenter: parent.verticalCenter

                    SoniqIcon {
                        anchors.centerIn: parent
                        name: playbackController.isPlaying ? "pause" : "play"
                        size: 20
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
                    iconPixelSize: 20
                    tooltipText: "Next Track"
                    onClicked: playbackController.next()
                }

                SoniqIconButton {
                    iconSymbol: playbackController.repeatMode === 1 ? "repeat_one" : "repeat"
                    iconPixelSize: 17
                    isAccent: playbackController.repeatMode > 0
                    tooltipText: playbackController.repeatMode === 1 ? "Repeat Track" : (playbackController.repeatMode === 2 ? "Repeat All" : "Repeat Off")
                    onClicked: {
                        var nextMode = (playbackController.repeatMode + 1) % 3;
                        playbackController.setRepeatMode(nextMode);
                    }
                }
            }
        }
    }
}
