import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

Rectangle {
    id: root

    property bool isOpen: false
    signal closeRequested()

    width: 340
    color: Colors.surfaceElevated
    border.color: Colors.borderSubtle
    border.width: 1

    Behavior on x {
        NumberAnimation { duration: Motion.durationNormal; easing.type: Motion.easingStandard }
    }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.spacingM
        spacing: 14

        // 1. Drawer Header
        Item {
            width: parent.width
            height: 32

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                SoniqIcon {
                    name: "queue"
                    size: 16
                    color: Colors.accent
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "PLAYING QUEUE"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeTitleSmall
                    font.weight: Typography.weightBold
                    font.letterSpacing: Typography.letterSpacingCaps
                    color: Colors.textPrimary
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "(" + playbackController.queueCount + ")"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeCaption
                    color: Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            SoniqIconButton {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                iconSymbol: "close"
                iconPixelSize: 13
                tooltipText: "Close Queue"
                onClicked: root.closeRequested()
            }
        }

        // 2. Currently Playing Track Card
        SoniqCard {
            width: parent.width
            height: 64
            color: Colors.surfaceCard

            Row {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Rectangle {
                    width: 44
                    height: 44
                    radius: Theme.cornerRadiusSmall
                    color: Colors.surfaceActive
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
                        size: 18
                        color: Colors.accent
                        visible: !playbackController.trackArtworkPath || playbackController.trackArtworkPath.length === 0
                    }
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 64
                    spacing: 2

                    Row {
                        spacing: 6
                        Rectangle {
                            width: 6
                            height: 6
                            radius: 3
                            color: playbackController.isPlaying ? Colors.accent : Colors.textMuted
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: playbackController.isPlaying ? "NOW PLAYING" : "PAUSED"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: playbackController.isPlaying ? Colors.accent : Colors.textMuted
                        }
                    }

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
        }

        // 3. Queue Action Bar (Up Next, Shuffle, Clear)
        Item {
            width: parent.width
            height: 28

            Text {
                text: "UP NEXT"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                font.letterSpacing: Typography.letterSpacingCaps
                color: Colors.textMuted
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                SoniqIconButton {
                    width: 28
                    height: 28
                    iconSymbol: "shuffle"
                    iconPixelSize: 13
                    tooltipText: "Shuffle Queue"
                    onClicked: playbackController.toggleShuffle()
                }

                SoniqIconButton {
                    width: 28
                    height: 28
                    iconSymbol: "trash"
                    iconPixelSize: 13
                    tooltipText: "Clear Queue"
                    visible: playbackController.queueCount > 0
                    onClicked: clearConfirmDialog.open()
                }
            }
        }

        // 4. Queue List View
        ListView {
            id: queueList
            width: parent.width
            height: parent.height - 180
            clip: true
            model: playbackController.queueItems
            spacing: 4

            delegate: Rectangle {
                required property int index
                required property var modelData
                width: queueList.width
                height: 48
                radius: Theme.cornerRadiusSmall
                property bool isCurrentTrack: modelData.isCurrent

                color: isCurrentTrack ? Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.12)
                                      : (qItemMouse.containsMouse ? Colors.surfaceHover : Colors.surfaceCard)

                Behavior on color { ColorAnimation { duration: Motion.durationFast } }

                MouseArea {
                    id: qItemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: playbackController.playQueueIndex(index)
                }

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 8
                    spacing: 8

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: (index + 1).toString()
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeCaption
                        color: isCurrentTrack ? Colors.accent : Colors.textMuted
                        width: 22
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 110
                        spacing: 2

                        Text {
                            width: parent.width
                            text: modelData.title
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            font.weight: isCurrentTrack ? Typography.weightBold : Typography.weightRegular
                            color: isCurrentTrack ? Colors.accent : Colors.textPrimary
                            elide: Text.ElideRight
                        }

                        Text {
                            width: parent.width
                            text: modelData.artist
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeMicro
                            color: Colors.textSecondary
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.durationFormatted
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        color: Colors.textMuted
                        visible: !qItemMouse.containsMouse
                    }

                    // Hover Actions: Move Up, Move Down, Remove
                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        visible: qItemMouse.containsMouse
                        spacing: 2

                        SoniqIconButton {
                            width: 24
                            height: 24
                            iconSymbol: "▲"
                            iconPixelSize: 9
                            tooltipText: "Move Up"
                            visible: index > 0
                            onClicked: playbackController.moveQueueItem(index, index - 1)
                        }

                        SoniqIconButton {
                            width: 24
                            height: 24
                            iconSymbol: "▼"
                            iconPixelSize: 9
                            tooltipText: "Move Down"
                            visible: index < playbackController.queueCount - 1
                            onClicked: playbackController.moveQueueItem(index, index + 1)
                        }

                        SoniqIconButton {
                            width: 24
                            height: 24
                            iconSymbol: "close"
                            iconPixelSize: 11
                            tooltipText: "Remove from Queue"
                            onClicked: playbackController.removeQueueItem(index)
                        }
                    }
                }
            }

            // Polished Empty State
            Item {
                anchors.centerIn: parent
                width: parent.width - 40
                height: 140
                visible: playbackController.queueCount === 0

                Column {
                    anchors.centerIn: parent
                    spacing: 10

                    SoniqIcon {
                        anchors.horizontalCenter: parent.horizontalCenter
                        name: "queue"
                        size: 36
                        color: Colors.textMuted
                    }

                    Text {
                        text: "Queue is Empty"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeTitleSmall
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Play a song or select tracks to queue them for upcoming playback."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeCaption
                        color: Colors.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        width: parent.parent.width
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }
    }

    // Confirmation Dialog for Clearing Queue
    Dialog {
        id: clearConfirmDialog
        title: "Clear Queue"
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        Text {
            text: "Are you sure you want to clear all upcoming tracks from the queue?"
            font.family: Typography.fontFamily
            font.pixelSize: Typography.sizeBody
            color: Colors.textPrimary
            wrapMode: Text.WordWrap
            width: 260
        }

        onAccepted: playbackController.clearQueue()
    }
}
