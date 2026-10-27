import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import "../themes"
import "../components"

Item {
    id: root

    signal scanFolderRequested(string path)

    FolderDialog {
        id: folderDialog
        title: "Select Music Folder to Scan"
        onAccepted: {
            var path = folderDialog.selectedFolder.toString();
            // Strip file:// prefix on Windows
            if (path.startsWith("file:///")) {
                path = path.substring(8);
            }
            root.scanFolderRequested(path);
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: 24
        clip: true

        Column {
            width: parent.width
            spacing: 28

            // Welcome Hero Banner
            SoniqCard {
                width: parent.width
                height: 140
                radius: Theme.cornerRadiusLarge

                Row {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 24

                    Rectangle {
                        width: 80
                        height: 80
                        radius: 20
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: Colors.accent }
                            GradientStop { position: 1.0; color: Qt.darker(Colors.accent, 1.4) }
                        }
                        Text {
                            anchors.centerIn: parent
                            text: "♫"
                            font.pixelSize: 40
                            color: "#FFFFFF"
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 260
                        spacing: 6

                        Text {
                            text: "Pure Sound. Infinite Possibilities."
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleLarge
                            font.weight: Typography.weightBold
                            color: Colors.textPrimary
                        }

                        Text {
                            text: "Offline-first audiophile music experience with bit-perfect hardware output."
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            color: Colors.textSecondary
                        }
                    }

                    SoniqButton {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Add Music Folder"
                        iconText: "＋"
                        onClicked: folderDialog.open()
                    }
                }
            }

            // Quick Stats Row
            Row {
                width: parent.width
                spacing: 16

                SoniqCard {
                    width: (parent.width - 32) / 3
                    height: 88

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 4

                        Text {
                            text: "TOTAL TRACKS"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: Colors.textMuted
                        }

                        Text {
                            text: libraryModel.count.toString()
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleLarge
                            font.weight: Typography.weightBold
                            color: Colors.textPrimary
                        }
                    }
                }

                SoniqCard {
                    width: (parent.width - 32) / 3
                    height: 88

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 4

                        Text {
                            text: "AUDIO ENGINE"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: Colors.textMuted
                        }

                        Text {
                            text: "WASAPI Core Audio"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleMedium
                            font.weight: Typography.weightBold
                            color: Colors.textPrimary
                        }
                    }
                }

                SoniqCard {
                    width: (parent.width - 32) / 3
                    height: 88

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 4

                        Text {
                            text: "BIT-PERFECT STATUS"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: Colors.textMuted
                        }

                        Row {
                            spacing: 8
                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                anchors.verticalCenter: parent.verticalCenter
                                color: (playbackController.bitPerfectStatus === "Verified") ? Colors.bitPerfectVerified : Colors.success
                            }
                            Text {
                                text: playbackController.bitPerfectStatus
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeTitleMedium
                                font.weight: Typography.weightBold
                                color: (playbackController.bitPerfectStatus === "Verified") ? Colors.bitPerfectVerified : Colors.textPrimary
                            }
                        }
                    }
                }
            }

            // Section: Recent Tracks
            Column {
                width: parent.width
                spacing: 12

                Text {
                    text: "Recently Added"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeTitleMedium
                    font.weight: Typography.weightBold
                    color: Colors.textPrimary
                }

                ListView {
                    width: parent.width
                    height: Math.min(240, count * 48)
                    model: libraryModel
                    clip: true
                    spacing: 4

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
                }
            }
        }
    }
}
