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
            root.scanFolderRequested(folderDialog.selectedFolder.toString());
        }
    }

    Flickable {
        id: homeFlick
        anchors.fill: parent
        anchors.margins: Theme.spacingXL
        contentWidth: width
        contentHeight: homeCol.implicitHeight + 48
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Column {
            id: homeCol
            width: homeFlick.width
            spacing: 24

            // 1. Welcome Hero Banner
            SoniqCard {
                width: parent.width
                height: 130
                radius: Theme.cornerRadiusLarge
                color: Colors.surfaceCard

                Item {
                    anchors.fill: parent
                    anchors.margins: 20

                    Rectangle {
                        id: heroIcon
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: 76
                        height: 76
                        radius: 18
                        color: Colors.accentMuted
                        border.color: Colors.accent
                        border.width: 1

                        SoniqIcon {
                            anchors.centerIn: parent
                            name: "songs"
                            size: 38
                            color: Colors.accent
                        }
                    }

                    Row {
                        id: actionButtonsRow
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 10

                        SoniqButton {
                            id: addFolderBtn
                            text: "Add Music Folder"
                            iconText: "add"
                            onClicked: folderDialog.open()
                        }

                        SoniqButton {
                            id: clearLibraryBtn
                            variant: "secondary"
                            text: "Clear Library"
                            iconText: "trash"
                            visible: libraryModel.count > 0
                            onClicked: clearConfirmDialog.open()
                        }
                    }

                    Column {
                        anchors.left: heroIcon.right
                        anchors.leftMargin: 20
                        anchors.right: actionButtonsRow.left
                        anchors.rightMargin: 20
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4

                        Text {
                            text: "Pure Sound. Infinite Possibilities."
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleLarge
                            font.weight: Typography.weightBold
                            color: Colors.textPrimary
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: "Audiophile music experience with direct WASAPI hardware output and bit-perfect verification."
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            color: Colors.textSecondary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }
                }
            }

            // 2. Audiophile Telemetry Quick Stats Row
            Row {
                width: parent.width
                spacing: 14

                SoniqCard {
                    width: (parent.width - 28) / 3
                    height: 82

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 3

                        Text {
                            text: "TOTAL TRACKS"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            font.letterSpacing: Typography.letterSpacingCaps
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
                    width: (parent.width - 28) / 3
                    height: 82

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 3

                        Text {
                            text: "AUDIO ENGINE"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            font.letterSpacing: Typography.letterSpacingCaps
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
                    width: (parent.width - 28) / 3
                    height: 82

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 3

                        Text {
                            text: "BIT-PERFECT OUTPUT"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            font.letterSpacing: Typography.letterSpacingCaps
                            color: Colors.textMuted
                        }

                        Row {
                            spacing: 8
                            Rectangle {
                                width: 8
                                height: 8
                                radius: 4
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

            // 3. First-Run Polished Empty State (Visible when no tracks)
            SoniqCard {
                width: parent.width
                height: 260
                visible: libraryModel.count === 0
                color: Colors.surfaceGrouped

                Column {
                    anchors.centerIn: parent
                    spacing: 14
                    width: Math.min(520, parent.width - 40)

                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 64
                        height: 64
                        radius: 20
                        color: Colors.accentMuted
                        border.color: Colors.accent
                        border.width: 1

                        SoniqIcon {
                            anchors.centerIn: parent
                            name: "folder"
                            size: 28
                            color: Colors.accent
                        }
                    }

                    Text {
                        text: "Your Music Library is Empty"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeTitleMedium
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "SONIQ supports FLAC, DSD/DSF, WAV, ALAC, AIFF, OGG, Opus, and MP3 with bit-perfect hardware output. Add your local music folder to start listening."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        color: Colors.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        width: parent.width
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    SoniqButton {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Select Music Folder"
                        iconText: "folder"
                        onClicked: folderDialog.open()
                    }
                }
            }

            // 4. Recently Added / Listening Section
            Column {
                width: parent.width
                spacing: 12
                visible: libraryModel.count > 0

                Item {
                    width: parent.width
                    height: 28

                    Text {
                        text: "Recently Added"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeTitleMedium
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    SoniqButton {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        variant: "ghost"
                        text: "View All Songs →"
                        onClicked: {
                            libraryModel.clearFilter();
                            if (window) window.activeScreen = "songs";
                        }
                    }
                }

                ListView {
                    width: parent.width
                    height: Math.min(10, count) * 56
                    model: libraryModel
                    interactive: false
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
                        filePath: model.filePath
                        isPlaying: playbackController.trackFilePath === model.filePath && playbackController.isPlaying

                        onPlayClicked: playbackController.playFile(model.filePath)
                        onFavoriteClicked: libraryModel.toggleFavorite(index)
                        onAddToPlaylistClicked: {
                            addToPlaylistDialog.trackId = model.id;
                            addToPlaylistDialog.trackTitle = model.title;
                            addToPlaylistDialog.open();
                        }
                        onEditTagsClicked: {
                            tagEditorDialog.trackId = model.id;
                            tagEditorDialog.title = model.title;
                            tagEditorDialog.artist = model.artist;
                            tagEditorDialog.album = model.album;
                            tagEditorDialog.genre = model.genre;
                            tagEditorDialog.year = model.year;
                            tagEditorDialog.open();
                        }
                    }
                }
            }
        }
    }

    // Confirmation dialog for Clear Library
    Dialog {
        id: clearConfirmDialog
        title: "Clear Library"
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        Text {
            text: "Are you sure you want to clear all tracks from the SONIQ library database? (Audio files on disk will not be deleted)."
            font.family: Typography.fontFamily
            font.pixelSize: Typography.sizeBody
            color: Colors.textPrimary
            wrapMode: Text.WordWrap
            width: 280
        }

        onAccepted: libraryModel.clearLibrary()
    }
}
