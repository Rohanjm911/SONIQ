import QtQuick
import QtQuick.Controls.Basic
import "../themes"

Item {
    id: root

    property bool isOpen: false
    signal playlistCreated(string name)

    visible: opacity > 0
    opacity: isOpen ? 1.0 : 0.0

    anchors.fill: parent
    z: 9999

    Behavior on opacity {
        NumberAnimation { duration: Motion.durationNormal; easing.type: Motion.easingStandard }
    }

    Rectangle {
        anchors.fill: parent
        color: "#99000000"

        MouseArea {
            anchors.fill: parent
            onClicked: root.close()
        }
    }

    SoniqCard {
        width: Math.min(420, parent.width - 32)
        height: 220
        anchors.centerIn: parent
        radius: Theme.cornerRadiusMedium

        MouseArea {
            anchors.fill: parent
        }

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            Item {
                width: parent.width
                height: 28

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Create New Playlist"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeTitleMedium
                    font.weight: Typography.weightBold
                    color: Colors.textPrimary
                }

                SoniqIconButton {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    iconSymbol: "✕"
                    iconPixelSize: 13
                    onClicked: root.close()
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: Colors.borderSubtle
            }

            Column {
                width: parent.width
                spacing: 4

                Text {
                    text: "PLAYLIST NAME"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                }

                Rectangle {
                    width: parent.width
                    height: 38
                    radius: Theme.cornerRadiusSmall
                    color: Colors.surfaceElevated
                    border.color: nameInput.activeFocus ? Colors.accent : Colors.borderSubtle
                    border.width: 1

                    TextInput {
                        id: nameInput
                        anchors.fill: parent
                        anchors.margins: 10
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        color: Colors.textPrimary
                        selectByMouse: true
                        onAccepted: submitBtn.clicked()
                    }
                }
            }

            Row {
                anchors.right: parent.right
                spacing: 10

                SoniqButton {
                    variant: "secondary"
                    text: "Cancel"
                    onClicked: root.close()
                }

                SoniqButton {
                    id: submitBtn
                    variant: "primary"
                    text: "Create Playlist"
                    iconText: "＋"
                    enabled: nameInput.text.trim().length > 0
                    onClicked: {
                        var name = nameInput.text.trim();
                        if (name.length > 0) {
                            var plId = libraryModel.createPlaylist(name);
                            root.playlistCreated(name);
                            root.close();
                        }
                    }
                }
            }
        }
    }

    function open() {
        nameInput.text = "";
        root.isOpen = true;
        nameInput.forceActiveFocus();
    }

    function close() {
        root.isOpen = false;
    }
}
