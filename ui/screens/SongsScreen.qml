import QtQuick
import QtQuick.Dialogs
import "../themes"
import "../components"

Item {
    id: root

    signal scanFolderRequested(string path)

    FolderDialog {
        id: folderDialog
        title: "Select Music Folder"
        onAccepted: {
            root.scanFolderRequested(folderDialog.selectedFolder.toString());
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        // Header and Search Bar
        Item {
            width: parent.width
            height: 38

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 16

                Text {
                    text: "Songs"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeTitleLarge
                    font.weight: Typography.weightBold
                    color: Colors.textPrimary
                    anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                    width: 280
                    height: 36
                    radius: Theme.cornerRadiusSmall
                    color: Colors.surfaceElevated
                    border.color: searchInput.activeFocus ? Colors.borderFocus : Colors.borderSubtle
                    anchors.verticalCenter: parent.verticalCenter

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Text {
                            text: "🔍"
                            font.pixelSize: 12
                            color: Colors.textMuted
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        TextInput {
                            id: searchInput
                            width: parent.width - 30
                            anchors.verticalCenter: parent.verticalCenter
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            color: Colors.textPrimary
                            selectByMouse: true
                            onTextChanged: libraryModel.search(text)

                            Text {
                                visible: parent.text.length === 0
                                text: "Search songs, artists, albums..."
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                color: Colors.textMuted
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }

            SoniqButton {
                id: addFolderBtn
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: "Add Folder"
                iconText: "＋"
                onClicked: folderDialog.open()
            }
        }

        // Table Column Headers
        Rectangle {
            width: parent.width
            height: 32
            color: "transparent"
            border.color: Colors.borderSubtle
            border.width: 0

            Row {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 12

                Text {
                    width: 28
                    text: "#"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    width: parent.width * 0.42
                    text: "TITLE"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    width: parent.width * 0.30
                    text: "ALBUM"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    width: 60
                    text: "FORMAT"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    width: 32
                    text: "FAV"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "TIME"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Track List View
        ListView {
            id: trackListView
            width: parent.width
            height: parent.height - 110
            clip: true
            model: libraryModel
            spacing: 2

            delegate: SoniqTrackRow {
                required property int index
                required property var model
                trackNumber: index + 1
                title: model.title
                artist: model.artist
                album: model.album
                duration: model.durationFormatted
                format: model.format
                artworkPath: model.artworkPath ? model.artworkPath : ""
                isFavorite: model.isFavorite
                isPlaying: playbackController.trackFilePath === model.filePath && playbackController.isPlaying

                onPlayClicked: playbackController.playFile(model.filePath)
                onFavoriteClicked: libraryModel.toggleFavorite(index)
            }

            // Empty State
            Item {
                anchors.centerIn: parent
                visible: trackListView.count === 0
                width: 320
                height: 180

                Column {
                    anchors.centerIn: parent
                    spacing: 12
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "♫"
                        font.pixelSize: 48
                        color: Colors.textMuted
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "No Tracks in Library"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeTitleMedium
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Click 'Add Folder' above to scan audio files."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        color: Colors.textSecondary
                    }
                }
            }
        }
    }
}
