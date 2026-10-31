import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

Item {
    id: root

    Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: width
        contentHeight: contentCol.implicitHeight + 64
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Column {
            id: contentCol
            width: Math.min(880, Math.max(480, flickable.width - 48))
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 24
            spacing: 20

        // Header
        Column {
            spacing: 4
            Text {
                text: "Settings"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeTitleLarge
                font.weight: Typography.weightBold
                color: Colors.textPrimary
            }
            Text {
                text: "Configure audio output, theme, playback engine, DSP, and local library preferences."
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeBody
                color: Colors.textSecondary
            }
        }

        // Section 1: APPEARANCE
        Column {
            width: parent.width
            spacing: 8

            Text {
                text: "APPEARANCE"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                font.letterSpacing: Typography.letterSpacingCaps
                color: Colors.textMuted
                leftPadding: 4
            }

            SoniqCard {
                width: parent.width
                height: 190
                color: Colors.surfaceCard

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    // Theme Mode Toggle (Dark / Light)
                    Item {
                        width: parent.width
                        height: 36

                        Column {
                            anchors.left: parent.left
                            anchors.right: themeRow.left
                            anchors.rightMargin: 16
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "Theme Mode"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                            Text {
                                text: "Select dark or light neutral interface foundation"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }

                        Row {
                            id: themeRow
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            SoniqButton {
                                variant: Colors.isDark ? "primary" : "secondary"
                                text: "Dark"
                                onClicked: {
                                    Colors.isDark = true;
                                    settingsService.themeMode = "Dark";
                                }
                            }

                            SoniqButton {
                                variant: !Colors.isDark ? "primary" : "secondary"
                                text: "Light"
                                onClicked: {
                                    Colors.isDark = false;
                                    settingsService.themeMode = "Light";
                                }
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Colors.borderSubtle }

                    // Accent Colors Presets
                    Item {
                        width: parent.width
                        height: 36

                        Column {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "Accent Color"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                            Text {
                                text: "Vibrant Apple Music-inspired tint palettes"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }

                        Row {
                            id: accentRow
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            Repeater {
                                model: ["#FA2D48", "#0A84FF", "#30D158", "#FF9F0A", "#AF52DE", "#00F0FF"]

                                Rectangle {
                                    width: 26
                                    height: 26
                                    radius: 13
                                    color: modelData
                                    border.color: Colors.accent.toString().toUpperCase() === modelData.toUpperCase() ? Colors.textPrimary : "transparent"
                                    border.width: 2

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            Colors.accent = modelData;
                                            settingsService.accentColor = modelData;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Colors.borderSubtle }

                    // Reduced Motion Toggle
                    Item {
                        width: parent.width
                        height: 36

                        Column {
                            anchors.left: parent.left
                            anchors.right: motionSwitch.left
                            anchors.rightMargin: 16
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "Reduce Motion"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                            Text {
                                text: "Minimize UI transitions and animations for accessibility or performance"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }

                        Switch {
                            id: motionSwitch
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            checked: settingsService.reducedMotion
                            onToggled: {
                                settingsService.reducedMotion = checked;
                                Motion.reducedMotion = checked;
                            }
                        }
                    }
                }
            }
        }

        // Section 2: AUDIO OUTPUT & PLAYBACK
        Column {
            width: parent.width
            spacing: 8

            Text {
                text: "AUDIO OUTPUT & PLAYBACK"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                font.letterSpacing: Typography.letterSpacingCaps
                color: Colors.textMuted
                leftPadding: 4
            }

            SoniqCard {
                width: parent.width
                height: 210
                color: Colors.surfaceCard

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    // WASAPI Exclusive Mode
                    Item {
                        width: parent.width
                        height: 40

                        Column {
                            anchors.left: parent.left
                            anchors.right: exclSwitch.left
                            anchors.rightMargin: 16
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "WASAPI Exclusive Mode"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                            Text {
                                text: "Bypass Windows Audio Engine mixer for bit-perfect hardware output"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }

                        Switch {
                            id: exclSwitch
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            checked: settingsService.exclusiveMode
                            onToggled: settingsService.exclusiveMode = checked
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Colors.borderSubtle }

                    // ReplayGain Loudness Normalization
                    Item {
                        width: parent.width
                        height: 40

                        Column {
                            anchors.left: parent.left
                            anchors.right: replaySwitch.left
                            anchors.rightMargin: 16
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "ReplayGain Loudness Normalization"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                            Text {
                                text: "Match perceived volume across tracks without compressing dynamic range"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }

                        Switch {
                            id: replaySwitch
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            checked: settingsService.replayGainEnabled
                            onToggled: settingsService.replayGainEnabled = checked
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Colors.borderSubtle }

                    // Gapless & Crossfading
                    Item {
                        width: parent.width
                        height: 40

                        Column {
                            anchors.left: parent.left
                            anchors.right: crossSwitch.left
                            anchors.rightMargin: 16
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "Crossfade Playback (" + settingsService.crossfadeSeconds.toFixed(1) + "s)"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                            Text {
                                text: "Equal-power sine/cosine transition between consecutive tracks"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }

                        Switch {
                            id: crossSwitch
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            checked: settingsService.crossfadeEnabled
                            onToggled: settingsService.crossfadeEnabled = checked
                        }
                    }
                }
            }
        }

        // Section 3: KEYBOARD SHORTCUTS
        Column {
            width: parent.width
            spacing: 8

            Text {
                text: "KEYBOARD SHORTCUTS"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                font.letterSpacing: Typography.letterSpacingCaps
                color: Colors.textMuted
                leftPadding: 4
            }

            SoniqCard {
                width: parent.width
                height: 150
                color: Colors.surfaceCard

                Item {
                    anchors.fill: parent
                    anchors.margins: 16

                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 12
                        spacing: 10

                        Repeater {
                            model: [
                                { key: "Space", desc: "Toggle Play / Pause" },
                                { key: "Left / Right", desc: "Seek Backward / Forward 5s" },
                                { key: "Up / Down", desc: "Volume Up / Down" }
                            ]

                            Row {
                                width: parent.width
                                spacing: 12

                                Rectangle {
                                    width: 88
                                    height: 24
                                    radius: 4
                                    color: Colors.surfaceElevated
                                    border.color: Colors.borderSubtle
                                    border.width: 1

                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.key
                                        font.family: Typography.monospaceFamily
                                        font.pixelSize: Typography.sizeCaption
                                        font.weight: Typography.weightBold
                                        color: Colors.textPrimary
                                    }
                                }

                                Text {
                                    text: modelData.desc
                                    font.family: Typography.fontFamily
                                    font.pixelSize: Typography.sizeCaption
                                    color: Colors.textSecondary
                                    anchors.verticalCenter: parent.verticalCenter
                                    elide: Text.ElideRight
                                    width: parent.width - 100
                                }
                            }
                        }
                    }

                    Column {
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 12
                        anchors.right: parent.right
                        spacing: 10

                        Repeater {
                            model: [
                                { key: "F11", desc: "Toggle Fullscreen Mode" },
                                { key: "Escape", desc: "Exit Fullscreen / Overlays" },
                                { key: "M", desc: "Toggle Mute" }
                            ]

                            Row {
                                width: parent.width
                                spacing: 12

                                Rectangle {
                                    width: 88
                                    height: 24
                                    radius: 4
                                    color: Colors.surfaceElevated
                                    border.color: Colors.borderSubtle
                                    border.width: 1

                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.key
                                        font.family: Typography.monospaceFamily
                                        font.pixelSize: Typography.sizeCaption
                                        font.weight: Typography.weightBold
                                        color: Colors.textPrimary
                                    }
                                }

                                Text {
                                    text: modelData.desc
                                    font.family: Typography.fontFamily
                                    font.pixelSize: Typography.sizeCaption
                                    color: Colors.textSecondary
                                    anchors.verticalCenter: parent.verticalCenter
                                    elide: Text.ElideRight
                                    width: parent.width - 100
                                }
                            }
                        }
                    }
                }
            }
        }

        // Section 4: ABOUT SONIQ
        Column {
            width: parent.width
            spacing: 8

            Text {
                text: "ABOUT SONIQ"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                font.letterSpacing: Typography.letterSpacingCaps
                color: Colors.textMuted
                leftPadding: 4
            }

            SoniqCard {
                width: parent.width
                height: 110
                color: Colors.surfaceCard

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 6

                    Row {
                        spacing: 8
                        SoniqIcon { name: "songs"; size: 18; color: Colors.accent; anchors.verticalCenter: parent.verticalCenter }
                        Text {
                            text: "SONIQ — Pure Sound. Infinite Possibilities."
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            font.weight: Typography.weightBold
                            color: Colors.textPrimary
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Text {
                        text: "Version 1.0.0 • Built with C++20, Qt 6.8.2, and Windows WASAPI Core Audio"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeCaption
                        color: Colors.textSecondary
                    }

                    Text {
                        text: "Offline-first desktop audiophile architecture with zero cloud telemetry and verified bit-perfect DAC hardware output."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeMicro
                        color: Colors.textMuted
                    }
                }
            }
        }
    }
}
}
