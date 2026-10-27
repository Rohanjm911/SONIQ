import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

Rectangle {
    id: root

    property string currentScreen: "home"
    signal navigationRequested(string screenName)

    width: Theme.sidebarWidth
    color: Colors.surface
    border.color: Colors.borderSubtle
    border.width: 1

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 24

        // App Branding
        Row {
            spacing: 12
            Rectangle {
                width: 36
                height: 36
                radius: 10
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Colors.accent }
                    GradientStop { position: 1.0; color: Qt.darker(Colors.accent, 1.4) }
                }
                Text {
                    anchors.centerIn: parent
                    text: "S"
                    font.family: Typography.fontFamily
                    font.pixelSize: 20
                    font.weight: Typography.weightBold
                    color: "#FFFFFF"
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 1
                Text {
                    text: "SONIQ"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeTitleMedium
                    font.weight: Typography.weightBold
                    color: Colors.textPrimary
                    letterSpacing: 1.5
                }
                Text {
                    text: "PURE SOUND"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                    letterSpacing: 1.0
                }
            }
        }

        // Section: LISTEN NOW
        Column {
            width: parent.width
            spacing: 4

            Text {
                text: "LISTEN NOW"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                color: Colors.textMuted
                leftPadding: 12
                bottomPadding: 4
            }

            Repeater {
                model: [
                    { id: "home", label: "Home", icon: "⌂" },
                    { id: "songs", label: "Songs", icon: "♫" },
                    { id: "nowplaying", label: "Now Playing", icon: "▶" }
                ]

                delegate: Rectangle {
                    width: parent.width
                    height: 38
                    radius: Theme.cornerRadiusSmall
                    color: root.currentScreen === modelData.id ? Colors.surfaceActive : (navMouse.containsMouse ? Colors.surfaceHover : "transparent")

                    Behavior on color {
                        ColorAnimation { duration: Motion.durationFast }
                    }

                    MouseArea {
                        id: navMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.navigationRequested(modelData.id)
                    }

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        spacing: 12

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.icon
                            font.pixelSize: 14
                            color: root.currentScreen === modelData.id ? Colors.accent : Colors.textSecondary
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.label
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            font.weight: root.currentScreen === modelData.id ? Typography.weightMedium : Typography.weightNormal
                            color: root.currentScreen === modelData.id ? Colors.textPrimary : Colors.textSecondary
                        }
                    }
                }
            }
        }

        // Section: AUDIO & DSP
        Column {
            width: parent.width
            spacing: 4

            Text {
                text: "AUDIO ENGINE"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                color: Colors.textMuted
                leftPadding: 12
                bottomPadding: 4
            }

            Repeater {
                model: [
                    { id: "equalizer", label: "Equalizer & DSP", icon: "🎚" },
                    { id: "inspector", label: "Audio Inspector", icon: "🔍" },
                    { id: "settings", label: "Settings", icon: "⚙" }
                ]

                delegate: Rectangle {
                    width: parent.width
                    height: 38
                    radius: Theme.cornerRadiusSmall
                    color: root.currentScreen === modelData.id ? Colors.surfaceActive : (navMouse2.containsMouse ? Colors.surfaceHover : "transparent")

                    Behavior on color {
                        ColorAnimation { duration: Motion.durationFast }
                    }

                    MouseArea {
                        id: navMouse2
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.navigationRequested(modelData.id)
                    }

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        spacing: 12

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.icon
                            font.pixelSize: 14
                            color: root.currentScreen === modelData.id ? Colors.accent : Colors.textSecondary
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.label
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            font.weight: root.currentScreen === modelData.id ? Typography.weightMedium : Typography.weightNormal
                            color: root.currentScreen === modelData.id ? Colors.textPrimary : Colors.textSecondary
                        }
                    }
                }
            }
        }

        Item {
            // Spacer to push audiophile status to bottom
            width: parent.width
            height: 1
        }
    }

    // Bottom Audiophile Hardware Status Card
    SoniqCard {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        height: 64

        Column {
            anchors.centerIn: parent
            spacing: 4
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 6
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    anchors.verticalCenter: parent.verticalCenter
                    color: (playbackController.bitPerfectStatus === "Verified") ? Colors.bitPerfectVerified : Colors.success
                }
                Text {
                    text: playbackController.bitPerfectStatus.toUpperCase()
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: (playbackController.bitPerfectStatus === "Verified") ? Colors.bitPerfectVerified : Colors.textPrimary
                }
            }
            Text {
                text: "WASAPI Core Audio"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeCaption
                color: Colors.textMuted
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
