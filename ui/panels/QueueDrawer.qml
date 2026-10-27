import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

Rectangle {
    id: root

    property bool isOpen: false
    signal closeRequested()

    width: 320
    color: Colors.surfaceElevated
    border.color: Colors.borderSubtle
    border.width: 1

    Behavior on x {
        NumberAnimation { duration: Motion.durationNormal; easing.type: Motion.easingStandard }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Drawer Header
        Item {
            width: parent.width
            height: 32

            Text {
                text: "PLAYING QUEUE"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeTitleSmall
                font.weight: Typography.weightBold
                color: Colors.textPrimary
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }

            SoniqIconButton {
                id: closeBtn
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                iconSymbol: "✕"
                iconPixelSize: 12
                onClicked: root.closeRequested()
            }
        }

        // Current Track Header
        SoniqCard {
            width: parent.width
            height: 64

            Row {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Rectangle {
                    width: 44
                    height: 44
                    radius: Theme.cornerRadiusSmall
                    color: Colors.surfaceActive
                    Text {
                        anchors.centerIn: parent
                        text: "♫"
                        font.pixelSize: 18
                        color: Colors.accent
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

        // Section: UP NEXT
        Row {
            width: parent.width
            Text {
                text: "UP NEXT"
                font.family: Typography.fontFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                color: Colors.textMuted
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        ListView {
            id: queueList
            width: parent.width
            height: parent.height - 180
            clip: true
            model: libraryModel
            spacing: 6

            delegate: Rectangle {
                required property int index
                required property var model
                width: queueList.width
                height: 44
                radius: Theme.cornerRadiusSmall
                color: qItemMouse.containsMouse ? Colors.surfaceHover : Colors.surface

                MouseArea {
                    id: qItemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onDoubleClicked: libraryModel.getTrackAt(index)
                }

                Row {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: (index + 1).toString()
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeCaption
                        color: Colors.textMuted
                        width: 20
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 70
                        spacing: 1

                        Text {
                            width: parent.width
                            text: model.title
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            color: Colors.textPrimary
                            elide: Text.ElideRight
                        }

                        Text {
                            width: parent.width
                            text: model.artist
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeMicro
                            color: Colors.textSecondary
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.durationFormatted
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeCaption
                        color: Colors.textMuted
                    }
                }
            }
        }
    }
}
