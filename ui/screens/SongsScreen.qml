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
        title: "Select Music Folder"
        onAccepted: {
            root.scanFolderRequested(folderDialog.selectedFolder.toString());
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        // Header, Search Bar, and Sort Controls
        Item {
            width: parent.width
            height: 38

            Row {
                id: titleGroup
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10

                Text {
                    id: songsTitle
                    text: libraryModel.activePlaylistId > 0 ? libraryModel.activePlaylistName : "Songs"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeTitleLarge
                    font.weight: Typography.weightBold
                    color: Colors.textPrimary
                    anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                    visible: libraryModel.activePlaylistId > 0
                    height: 24
                    width: clearPlText.width + 16
                    radius: 12
                    color: Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.15)
                    border.color: Colors.accent
                    border.width: 1
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        id: clearPlText
                        anchors.centerIn: parent
                        text: "✕ Clear Filter"
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: Colors.accent
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: libraryModel.clearPlaylistFilter()
                    }
                }
            }

            Rectangle {
                id: searchBar
                anchors.left: titleGroup.right
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(260, Math.max(140, topActionRow.x - titleGroup.x - titleGroup.width - 32))
                height: 36
                radius: Theme.cornerRadiusSmall
                color: Colors.surfaceElevated
                border.color: searchInput.activeFocus ? Colors.borderFocus : Colors.borderSubtle

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    SoniqIcon {
                        name: "search"
                        size: 14
                        color: Colors.textMuted
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    TextInput {
                        id: searchInput
                        width: parent.width - 24
                        anchors.verticalCenter: parent.verticalCenter
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        color: Colors.textPrimary
                        selectByMouse: true
                        onTextChanged: libraryModel.search(text)

                        Text {
                            visible: parent.text.length === 0
                            text: "Search songs, artists..."
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeBody
                            color: Colors.textMuted
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            Row {
                id: topActionRow
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                // Delete Playlist Button (if currently viewing a playlist)
                SoniqButton {
                    id: deletePlBtn
                    variant: "secondary"
                    iconText: "✕"
                    text: "Delete Playlist"
                    visible: libraryModel.activePlaylistId > 0
                    onClicked: libraryModel.deletePlaylist(libraryModel.activePlaylistId)
                }

                // Sort Dropdown Button
                SoniqButton {
                    id: sortBtn
                    variant: "secondary"
                    iconText: "⇅"
                    text: libraryModel.sortLabel
                    onClicked: sortPopup.visible ? sortPopup.close() : sortPopup.open()
                }

                // Shuffle Playlist Button
                SoniqButton {
                    id: shuffleBtn
                    variant: "secondary"
                    iconText: "shuffle"
                    text: "Shuffle"
                    onClicked: libraryModel.shuffleList()
                }

                // Add Folder Button
                SoniqButton {
                    id: addFolderBtn
                    text: "Add Folder"
                    iconText: "＋"
                    onClicked: folderDialog.open()
                }

                // Clear Library Button
                SoniqButton {
                    id: clearLibraryBtn
                    variant: "secondary"
                    text: "Clear"
                    iconText: "✕"
                    visible: libraryModel.count > 0 && libraryModel.activePlaylistId <= 0
                    onClicked: libraryModel.clearLibrary()
                }
            }
        }

        // Table Column Headers (Clickable for instant column sorting)
        Rectangle {
            width: parent.width
            height: 32
            color: "transparent"

            // 1. Left Group: # and TITLE
            Row {
                id: headerLeftGroup
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                width: Math.max(180, (parent.width - 154 - 80) * 0.50)
                spacing: 12

                Item {
                    width: 28
                    height: parent.height
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "#" + (libraryModel.sortField === "track_number" ? (libraryModel.sortAscending ? " ▲" : " ▼") : "")
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: libraryModel.sortField === "track_number" ? Colors.accent : (hNumMouse.containsMouse ? Colors.textPrimary : Colors.textMuted)
                    }

                    MouseArea {
                        id: hNumMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: libraryModel.sortBy("track_number")
                    }
                }

                Item {
                    height: parent.height
                    width: parent.width - 40
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "TITLE" + (libraryModel.sortField === "title" ? (libraryModel.sortAscending ? " ▲" : " ▼") : "")
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: libraryModel.sortField === "title" ? Colors.accent : (hTitleMouse.containsMouse ? Colors.textPrimary : Colors.textMuted)
                    }

                    MouseArea {
                        id: hTitleMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: libraryModel.sortBy("title")
                    }
                }
            }

            // 2. Center: ALBUM
            Item {
                id: headerAlbumItem
                anchors.left: headerLeftGroup.right
                anchors.leftMargin: 16
                anchors.right: headerRightGroup.left
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                height: parent.height

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "ALBUM" + (libraryModel.sortField === "album" ? (libraryModel.sortAscending ? " ▲" : " ▼") : "")
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: libraryModel.sortField === "album" ? Colors.accent : (hAlbumMouse.containsMouse ? Colors.textPrimary : Colors.textMuted)
                    elide: Text.ElideRight
                }

                MouseArea {
                    id: hAlbumMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: libraryModel.sortBy("album")
                }
            }

            // 3. Right Group: FORMAT + FAV + TIME
            Row {
                id: headerRightGroup
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Item {
                    width: 58
                    height: parent.height
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "FORMAT" + (libraryModel.sortField === "format" ? (libraryModel.sortAscending ? " ▲" : " ▼") : "")
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: libraryModel.sortField === "format" ? Colors.accent : (hFmtMouse.containsMouse ? Colors.textPrimary : Colors.textMuted)
                    }

                    MouseArea {
                        id: hFmtMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: libraryModel.sortBy("format")
                    }
                }

                Item {
                    width: 28
                    height: parent.height
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "FAV" + (libraryModel.sortField === "favorite" ? (libraryModel.sortAscending ? " ▲" : " ▼") : "")
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: libraryModel.sortField === "favorite" ? Colors.accent : (hFavMouse.containsMouse ? Colors.textPrimary : Colors.textMuted)
                    }

                    MouseArea {
                        id: hFavMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: libraryModel.sortBy("favorite")
                    }
                }

                Item {
                    width: 44
                    height: parent.height
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        text: "TIME" + (libraryModel.sortField === "duration" ? (libraryModel.sortAscending ? " ▲" : " ▼") : "")
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: libraryModel.sortField === "duration" ? Colors.accent : (hTimeMouse.containsMouse ? Colors.textPrimary : Colors.textMuted)
                    }

                    MouseArea {
                        id: hTimeMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: libraryModel.sortBy("duration")
                    }
                }
            }
        }

        // Sort Dropdown Menu Popup
        Popup {
            id: sortPopup
            x: Math.max(10, root.width - width - 24)
            y: 44
            width: 240
            padding: 6
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
            background: Rectangle {
                radius: Theme.cornerRadiusSmall
                color: Colors.surfaceElevated
                border.color: Colors.borderSubtle
                border.width: 1
            }
            contentItem: Column {
                spacing: 2
                Repeater {
                    model: [
                        { label: "Title (A → Z)", field: "title", asc: true },
                        { label: "Title (Z → A)", field: "title", asc: false },
                        { label: "Artist (A → Z)", field: "artist", asc: true },
                        { label: "Artist (Z → A)", field: "artist", asc: false },
                        { label: "Album (A → Z)", field: "album", asc: true },
                        { label: "Album (Z → A)", field: "album", asc: false },
                        { label: "Duration (Shortest First)", field: "duration", asc: true },
                        { label: "Duration (Longest First)", field: "duration", asc: false },
                        { label: "Date Added (Newest First)", field: "date_added", asc: false },
                        { label: "Format (Lossless First)", field: "format", asc: true },
                        { label: "Favorites First", field: "favorite", asc: false },
                        { label: "Most Played First", field: "play_count", asc: false }
                    ]
                    delegate: Rectangle {
                        width: parent.width
                        height: 30
                        radius: 4
                        property bool isSelected: libraryModel.sortField === modelData.field && libraryModel.sortAscending === modelData.asc
                        color: isSelected ? Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.15) : (itemMouse.containsMouse ? Colors.surfaceHover : "transparent")

                        MouseArea {
                            id: itemMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                libraryModel.setSort(modelData.field, modelData.asc);
                                sortPopup.close();
                            }
                        }

                        Item {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10

                            Text {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.label
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeCaption
                                font.weight: isSelected ? Typography.weightBold : Typography.weightRegular
                                color: isSelected ? Colors.accent : Colors.textPrimary
                            }

                            Text {
                                visible: isSelected
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                text: "✓"
                                font.pixelSize: 12
                                color: Colors.accent
                            }
                        }
                    }
                }
            }
        }

        // Track List View
        ListView {
            id: trackListView
            width: parent.width
            height: Math.max(100, parent.height - y)
            clip: true
            model: libraryModel
            spacing: 2

            delegate: SoniqTrackRow {
                required property int index
                required property var model
                trackId: model.id !== undefined ? model.id : 0
                trackNumber: index + 1
                title: model.title
                artist: model.artist
                album: model.album
                genre: model.genre !== undefined ? model.genre : ""
                year: model.year !== undefined ? model.year : 0
                filePath: model.filePath !== undefined ? model.filePath : ""
                duration: model.durationFormatted
                format: model.format
                artworkPath: model.artworkPath ? model.artworkPath : ""
                isFavorite: model.isFavorite
                inPlaylist: libraryModel.activePlaylistId > 0
                isPlaying: playbackController.trackFilePath === model.filePath && playbackController.isPlaying

                onPlayClicked: playbackController.playFile(model.filePath)
                onFavoriteClicked: libraryModel.toggleFavorite(index)
                onEditTagsClicked: tagEditorDialog.openWith({
                    id: model.id,
                    title: model.title,
                    artist: model.artist,
                    album: model.album,
                    genre: model.genre,
                    year: model.year,
                    filePath: model.filePath
                })
                onAddToPlaylistClicked: addToPlaylistDialog.openWith(model.id, model.title, model.artist)
                onRemoveFromPlaylistClicked: libraryModel.removeTrackFromPlaylist(libraryModel.activePlaylistId, model.id)
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
