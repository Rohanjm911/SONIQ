import QtQuick
import QtQuick.Controls.Basic
import "../themes"

Item {
    id: root

    property bool isOpen: false
    property var trackData: null

    visible: opacity > 0
    opacity: isOpen ? 1.0 : 0.0

    anchors.fill: parent
    z: 9999

    Behavior on opacity {
        NumberAnimation { duration: Motion.durationNormal; easing.type: Motion.easingStandard }
    }

    // Modal background dimming
    Rectangle {
        anchors.fill: parent
        color: "#99000000"

        MouseArea {
            anchors.fill: parent
            onClicked: root.close()
        }
    }

    // Dialog card
    SoniqCard {
        id: card
        width: Math.min(520, parent.width - 32)
        height: 480
        anchors.centerIn: parent
        radius: Theme.cornerRadiusMedium

        MouseArea {
            anchors.fill: parent
            // Prevent clicks inside card from closing modal
        }

        Column {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16

            // Header Row
            Item {
                width: parent.width
                height: 28

                Row {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8

                    SoniqIcon {
                        name: "inspector"
                        size: 18
                        color: Colors.accent
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "Edit Metadata Tags"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeTitleMedium
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
                        anchors.verticalCenter: parent.verticalCenter
                    }
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

            // Form Fields
            Column {
                width: parent.width
                spacing: 12

                // Title Field
                Column {
                    width: parent.width
                    spacing: 4
                    Text {
                        text: "TRACK TITLE"
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: Colors.textMuted
                    }
                    Rectangle {
                        width: parent.width
                        height: 36
                        radius: Theme.cornerRadiusSmall
                        color: Colors.surfaceElevated
                        border.color: titleInput.activeFocus ? Colors.accent : Colors.borderSubtle
                        border.width: 1

                        TextInput {
                            id: titleInput
                            anchors.fill: parent
                            anchors.margins: 8
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            color: Colors.textPrimary
                            selectByMouse: true
                        }
                    }
                }

                // Artist & Album Row
                Row {
                    width: parent.width
                    spacing: 12

                    Column {
                        width: (parent.width - 12) / 2
                        spacing: 4
                        Text {
                            text: "ARTIST"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: Colors.textMuted
                        }
                        Rectangle {
                            width: parent.width
                            height: 36
                            radius: Theme.cornerRadiusSmall
                            color: Colors.surfaceElevated
                            border.color: artistInput.activeFocus ? Colors.accent : Colors.borderSubtle
                            border.width: 1

                            TextInput {
                                id: artistInput
                                anchors.fill: parent
                                anchors.margins: 8
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                color: Colors.textPrimary
                                selectByMouse: true
                            }
                        }
                    }

                    Column {
                        width: (parent.width - 12) / 2
                        spacing: 4
                        Text {
                            text: "ALBUM"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: Colors.textMuted
                        }
                        Rectangle {
                            width: parent.width
                            height: 36
                            radius: Theme.cornerRadiusSmall
                            color: Colors.surfaceElevated
                            border.color: albumInput.activeFocus ? Colors.accent : Colors.borderSubtle
                            border.width: 1

                            TextInput {
                                id: albumInput
                                anchors.fill: parent
                                anchors.margins: 8
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                color: Colors.textPrimary
                                selectByMouse: true
                            }
                        }
                    }
                }

                // Genre & Year Row
                Row {
                    width: parent.width
                    spacing: 12

                    Column {
                        width: (parent.width - 12) * 0.65
                        spacing: 4
                        Text {
                            text: "GENRE"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: Colors.textMuted
                        }
                        Rectangle {
                            width: parent.width
                            height: 36
                            radius: Theme.cornerRadiusSmall
                            color: Colors.surfaceElevated
                            border.color: genreInput.activeFocus ? Colors.accent : Colors.borderSubtle
                            border.width: 1

                            TextInput {
                                id: genreInput
                                anchors.fill: parent
                                anchors.margins: 8
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                color: Colors.textPrimary
                                selectByMouse: true
                            }
                        }
                    }

                    Column {
                        width: (parent.width - 12) * 0.35
                        spacing: 4
                        Text {
                            text: "YEAR"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: Colors.textMuted
                        }
                        Rectangle {
                            width: parent.width
                            height: 36
                            radius: Theme.cornerRadiusSmall
                            color: Colors.surfaceElevated
                            border.color: yearInput.activeFocus ? Colors.accent : Colors.borderSubtle
                            border.width: 1

                            TextInput {
                                id: yearInput
                                anchors.fill: parent
                                anchors.margins: 8
                                font.family: Typography.monospaceFamily
                                font.pixelSize: Typography.sizeBody
                                color: Colors.textPrimary
                                selectByMouse: true
                                inputMethodHints: Qt.ImhDigitsOnly
                            }
                        }
                    }
                }

                // File path info
                Column {
                    width: parent.width
                    spacing: 2

                    Text {
                        text: "FILE LOCATION"
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: Colors.textMuted
                    }
                    Text {
                        width: parent.width
                        text: root.trackData ? (root.trackData.filePath || "") : ""
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        color: Colors.textSecondary
                        elide: Text.ElideMiddle
                    }
                }
            }

            Item {
                // Spacer
                width: parent.width
                height: 1
            }

            // Bottom Buttons
            Row {
                anchors.right: parent.right
                spacing: 12

                SoniqButton {
                    variant: "secondary"
                    text: "Cancel"
                    onClicked: root.close()
                }

                SoniqButton {
                    variant: "primary"
                    text: "Save Tags"
                    iconText: "✓"
                    onClicked: {
                        if (root.trackData && root.trackData.id) {
                            var y = parseInt(yearInput.text) || 0;
                            libraryModel.updateTrackMetadata(
                                root.trackData.id,
                                titleInput.text.trim(),
                                artistInput.text.trim(),
                                albumInput.text.trim(),
                                genreInput.text.trim(),
                                y
                            );
                        }
                        root.close();
                    }
                }
            }
        }
    }

    function openWith(data) {
        root.trackData = data;
        titleInput.text = data.title || "";
        artistInput.text = data.artist || "";
        albumInput.text = data.album || "";
        genreInput.text = data.genre || "";
        yearInput.text = data.year ? data.year.toString() : "";
        root.isOpen = true;
    }

    function close() {
        root.isOpen = false;
        root.trackData = null;
    }
}
