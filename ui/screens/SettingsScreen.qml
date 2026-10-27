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
                height: 140

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    // Theme selector
                    Item {
                        width: parent.width
                        height: 38

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

                    // Accent Colors
                    Item {
                        width: parent.width
                        height: 36

                        Text {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Accent Color"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            font.weight: Typography.weightMedium
                            color: Colors.textPrimary
                        }

                        Row {
                            id: accentRow
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 10

                            Repeater {
                                model: ["#FA2D48", "#3B82F6", "#10B981", "#8B5CF6", "#F59E0B"]

                                Rectangle {
                                    width: 26
                                    height: 26
                                    radius: 13
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
                height: 156

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

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

                    // Hardware Buffer Size
                    Item {
                        width: parent.width
                        height: 40

                        Column {
                            anchors.left: parent.left
                            anchors.right: bufCombo.left
                            anchors.rightMargin: 16
                            anchors.verticalCenter: parent.verticalCenter
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

                        ComboBox {
                            id: bufCombo
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
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
                    spacing: 6

                    Text {
                        text: "SONIQ — Pure Sound. Infinite Possibilities."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
                    }

                    Text {
                        text: "Version 1.0.0 • Built with C++20, Qt 6.8.2, and Windows WASAPI"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeCaption
                        color: Colors.textSecondary
                    }

                    Text {
                        text: "Offline-first desktop architecture with zero cloud tracking and bit-perfect playback."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeMicro
                        color: Colors.textMuted
                    }
                }
            }
        }
    }
}
