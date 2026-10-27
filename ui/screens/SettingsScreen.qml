import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

ScrollView {
    id: root

    clip: true

    Column {
        width: Math.min(840, parent.width - 48)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 24
        topPadding: 24
        bottomPadding: 32

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
                text: "Configure audio output, theme, DSP processing, and local library preferences."
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeBody
                color: Colors.textSecondary
            }
        }

        // Section: APPEARANCE
        Column {
            width: parent.width
            spacing: 12

            Text {
                text: "APPEARANCE"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                color: Colors.textMuted
                leftPadding: 4
            }

            SoniqCard {
                width: parent.width
                height: 120

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    // Theme selector
                    Row {
                        width: parent.width
                        Column {
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

                        Item { width: 1; height: 1; anchors.right: themeRow.left }

                        Row {
                            id: themeRow
                            anchors.right: parent.right
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

                    // Accent Colors
                    Row {
                        width: parent.width
                        Text {
                            text: "Accent Color"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            font.weight: Typography.weightMedium
                            color: Colors.textPrimary
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Item { width: 1; height: 1; anchors.right: accentRow.left }

                        Row {
                            id: accentRow
                            anchors.right: parent.right
                            spacing: 10
                            anchors.verticalCenter: parent.verticalCenter

                            Repeater {
                                model: ["#FA2D48", "#3B82F6", "#10B981", "#8B5CF6", "#F59E0B"]

                                Rectangle {
                                    width: 24
                                    height: 24
                                    radius: 12
                                    color: modelData
                                    border.color: Colors.accent === modelData ? "#FFFFFF" : "transparent"
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
                }
            }
        }

        // Section: AUDIO OUTPUT
        Column {
            width: parent.width
            spacing: 12

            Text {
                text: "AUDIO OUTPUT & BIT-PERFECT"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                color: Colors.textMuted
                leftPadding: 4
            }

            SoniqCard {
                width: parent.width
                height: 140

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    Row {
                        width: parent.width
                        Column {
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

                        Item { width: 1; height: 1; anchors.right: exclSwitch.left }

                        Switch {
                            id: exclSwitch
                            anchors.right: parent.right
                            checked: settingsService.exclusiveMode
                            onToggled: settingsService.exclusiveMode = checked
                        }
                    }

                    Row {
                        width: parent.width
                        Column {
                            spacing: 2
                            Text {
                                text: "Hardware Buffer Size"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                            Text {
                                text: "Lower values reduce latency; higher values protect against buffer underruns"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }

                        Item { width: 1; height: 1; anchors.right: bufCombo.left }

                        ComboBox {
                            id: bufCombo
                            anchors.right: parent.right
                            model: ["512 frames (~11 ms)", "1024 frames (~23 ms)", "2048 frames (~46 ms)"]
                            currentIndex: 1
                        }
                    }
                }
            }
        }

        // Section: ABOUT SONIQ
        Column {
            width: parent.width
            spacing: 12

            Text {
                text: "ABOUT SONIQ"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                color: Colors.textMuted
                leftPadding: 4
            }

            SoniqCard {
                width: parent.width
                height: 100

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 4

                    Text {
                        text: "SONIQ — Pure Sound. Infinite Possibilities."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeTitleSmall
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
                    }

                    Text {
                        text: "Version 1.0.0 (Phase 1 Foundation) • Built with C++20, Qt 6.8.2, and Windows WASAPI"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        color: Colors.textSecondary
                    }

                    Text {
                        text: "Offline-first desktop architecture with zero cloud tracking and bit-perfect playback."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeCaption
                        color: Colors.textMuted
                    }
                }
            }
        }
    }
}
