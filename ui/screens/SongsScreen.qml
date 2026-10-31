import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import "../themes"
import "../components"

Item {
    id: root

    signal scanFolderRequested(string path)

    // Current Library Sub-View: "songs", "albums", "artists", "genres"
    property string activeTab: (window && window.activeLibraryTab) ? window.activeLibraryTab : "songs"

    FolderDialog {
        id: folderDialog
        title: "Select Music Folder"
        onAccepted: {
            root.scanFolderRequested(folderDialog.selectedFolder.toString());
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: Theme.spacingXL
        spacing: 16

        // 1. Top Control Bar: View Switcher, Title, Search, and Sort
        Item {
            width: parent.width
            height: 42

            // Left: Title and Active Filter Pill
            Row {
                id: titleGroup
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Text {
                    text: {
                        if (libraryModel.activePlaylistId > 0) return libraryModel.activePlaylistName;
                        if (libraryModel.activeFilterType === "album") return libraryModel.activeFilterValue;
                        if (libraryModel.activeFilterType === "artist") return libraryModel.activeFilterValue;
                        if (libraryModel.activeFilterType === "genre") return libraryModel.activeFilterValue;
                        if (libraryModel.activeFilterType === "favorites") return "Favorites";
                        if (libraryModel.activeFilterType === "recent_played") return "Recently Played";
                        if (root.activeTab === "albums") return "Albums";
                        if (root.activeTab === "artists") return "Artists";
                        if (root.activeTab === "genres") return "Genres";
                        return "Songs";
                    }
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeTitleLarge
                    font.weight: Typography.weightBold
                    color: Colors.textPrimary
                    anchors.verticalCenter: parent.verticalCenter
                }

                // Filter active indicator with quick clear
                Rectangle {
                    visible: libraryModel.activePlaylistId > 0 || (libraryModel.activeFilterType && libraryModel.activeFilterType.length > 0)
                    height: 24
                    width: clearPlRow.implicitWidth + 14
                    radius: Theme.cornerRadiusPill
                    color: Colors.accentMuted
                    border.color: Colors.accent
                    border.width: 1
                    anchors.verticalCenter: parent.verticalCenter

                    Row {
                        id: clearPlRow
                        anchors.centerIn: parent
                        spacing: 4

                        Text {
                            text: "✕ Clear Filter"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightMedium
                            color: Colors.accent
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            libraryModel.clearFilter();
                            root.activeTab = "songs";
                            if (window) window.activeLibraryTab = "songs";
                        }
                    }
                }
            }

            // Center: Segmented View Switcher Tabs (Songs | Albums | Artists | Genres)
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                height: 34
                width: 320
                radius: Theme.cornerRadiusSmall
                color: Colors.surfaceElevated
                border.color: Colors.borderSubtle
                border.width: 1
                visible: parent.width > 780

                Row {
                    anchors.fill: parent
                    anchors.margins: 2
                    spacing: 2

                    Repeater {
                        model: [
                            { id: "songs", label: "Songs" },
                            { id: "albums", label: "Albums" },
                            { id: "artists", label: "Artists" },
                            { id: "genres", label: "Genres" }
                        ]

                        delegate: Rectangle {
                            width: (parent.width - 6) / 4
                            height: parent.height
                            radius: Theme.cornerRadiusSmall - 1
                            property bool isCurrentTab: (root.activeTab === modelData.id)
                            color: isCurrentTab ? Colors.surfaceCard : (tabMouse.containsMouse ? Colors.surfaceHover : "transparent")

                            Behavior on color { ColorAnimation { duration: Motion.durationFast } }

                            MouseArea {
                                id: tabMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.activeTab = modelData.id;
                                    if (window) window.activeLibraryTab = modelData.id;
                                    if (modelData.id === "songs" && libraryModel.activeFilterType !== "favorites" && libraryModel.activeFilterType !== "recent_played") {
                                        libraryModel.clearFilter();
                                    }
                                }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: modelData.label
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeCaption
                                font.weight: isCurrentTab ? Typography.weightMedium : Typography.weightRegular
                                color: isCurrentTab ? Colors.textPrimary : Colors.textSecondary
                            }
                        }
                    }
                }
            }

            // Right: Search Bar, Sort selector, and Add Folder
            Row {
                id: topActionRow
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                // Search Bar
                Rectangle {
                    width: 200
                    height: 34
                    radius: Theme.cornerRadiusSmall
                    color: Colors.surfaceElevated
                    border.color: searchInput.activeFocus ? Colors.borderFocus : Colors.borderSubtle
                    border.width: 1

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 6

                        SoniqIcon {
                            name: "search"
                            size: 13
                            color: Colors.textMuted
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        TextInput {
                            id: searchInput
                            width: parent.width - 42
                            anchors.verticalCenter: parent.verticalCenter
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            color: Colors.textPrimary
                            selectByMouse: true
                            onTextChanged: libraryModel.search(text)
                        }

                        Text {
                            visible: searchInput.text.length > 0
                            text: "✕"
                            font.pixelSize: 11
                            color: Colors.textMuted
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: searchInput.text = ""
                            }
                        }
                    }
                }

                // Sort Dropdown Button
                SoniqButton {
                    variant: "secondary"
                    customRadius: Theme.cornerRadiusSmall
                    text: libraryModel.sortLabel
                    iconText: "sort"
                    implicitWidth: 150
                    onClicked: sortMenu.open()

                    Menu {
                        id: sortMenu
                        y: parent.height + 4
                        MenuItem { text: "Title (A → Z)"; onTriggered: libraryModel.setSort("title", true) }
                        MenuItem { text: "Title (Z → A)"; onTriggered: libraryModel.setSort("title", false) }
                        MenuItem { text: "Artist (A → Z)"; onTriggered: libraryModel.setSort("artist", true) }
                        MenuItem { text: "Album (A → Z)"; onTriggered: libraryModel.setSort("album", true) }
                        MenuItem { text: "Duration (Shortest)"; onTriggered: libraryModel.setSort("duration", true) }
                        MenuItem { text: "Duration (Longest)"; onTriggered: libraryModel.setSort("duration", false) }
                        MenuItem { text: "Date Added (Newest)"; onTriggered: libraryModel.setSort("date_added", false) }
                        MenuItem { text: "Format (Lossless First)"; onTriggered: libraryModel.setSort("format", true) }
                        MenuItem { text: "Most Played"; onTriggered: libraryModel.setSort("play_count", false) }
                    }
                }

                // Shuffle button
                SoniqIconButton {
                    iconSymbol: "shuffle"
                    iconPixelSize: 14
                    tooltipText: "Shuffle All Tracks"
                    onClicked: {
                        libraryModel.shuffleList();
                        if (libraryModel.count > 0) {
                            var t = libraryModel.getTrackAt(0);
                            if (t && t.filePath) playbackController.playFile(t.filePath);
                        }
                    }
                }

                // Add Folder Button
                SoniqIconButton {
                    iconSymbol: "folder"
                    iconPixelSize: 14
                    tooltipText: "Scan Folder"
                    onClicked: folderDialog.open()
                }
            }
        }

        // 2. View Content Area: Switch between Songs List, Albums Grid, Artists Grid, Genres Grid
        Item {
            width: parent.width
            height: parent.height - 58
            clip: true

            // === 2A. SONGS LIST VIEW ===
            ListView {
                id: songListView
                anchors.fill: parent
                visible: root.activeTab === "songs"
                model: libraryModel
                clip: true
                spacing: 3

                delegate: SoniqTrackRow {
                    required property int index
                    required property var model
                    trackNumber: index + 1
                    trackId: model.id
                    title: model.title
                    artist: model.artist
                    album: model.album
                    duration: model.durationFormatted
                    format: model.format
                    artworkPath: model.artworkPath ? model.artworkPath : ""
                    isFavorite: model.isFavorite
                    filePath: model.filePath
                    inPlaylist: libraryModel.activePlaylistId > 0
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
                    onRemoveFromPlaylistClicked: {
                        libraryModel.removeTrackFromPlaylist(libraryModel.activePlaylistId, model.id);
                    }
                }

                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            }

            // === 2B. ALBUMS GRID VIEW ===
            GridView {
                id: albumGridView
                anchors.fill: parent
                visible: root.activeTab === "albums"
                clip: true
                cellWidth: Math.max(160, Math.floor(width / Math.max(2, Math.floor(width / 170))))
                cellHeight: cellWidth + 56
                model: root.activeTab === "albums" ? libraryModel.getAlbums() : []

                delegate: Item {
                    width: albumGridView.cellWidth
                    height: albumGridView.cellHeight

                    SoniqCard {
                        anchors.fill: parent
                        anchors.margins: 6
                        color: albMouse.containsMouse ? Colors.surfaceHover : Colors.surfaceCard
                        border.color: albMouse.containsMouse ? Colors.accent : Colors.borderSubtle
                        radius: Theme.cornerRadiusMedium

                        MouseArea {
                            id: albMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                libraryModel.filterByAlbum(modelData.name);
                                root.activeTab = "songs";
                                if (window) window.activeLibraryTab = "songs";
                            }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8

                            // Square Album Artwork
                            Rectangle {
                                width: parent.width
                                height: width
                                radius: Theme.cornerRadiusSmall
                                color: Colors.surfaceElevated
                                clip: true

                                Image {
                                    anchors.fill: parent
                                    source: (modelData.artworkPath && modelData.artworkPath.length > 0)
                                            ? ("file:///" + modelData.artworkPath)
                                            : ""
                                    fillMode: Image.PreserveAspectCrop
                                    smooth: true
                                    visible: status === Image.Ready
                                }

                                SoniqIcon {
                                    anchors.centerIn: parent
                                    name: "albums"
                                    size: 36
                                    color: Colors.textMuted
                                    visible: !modelData.artworkPath || modelData.artworkPath.length === 0
                                }

                                // Quick Play Overlay on Hover
                                Rectangle {
                                    anchors.fill: parent
                                    color: Qt.rgba(0, 0, 0, 0.4)
                                    opacity: albMouse.containsMouse ? 1.0 : 0.0
                                    Behavior on opacity { NumberAnimation { duration: Motion.durationFast } }

                                    Rectangle {
                                        width: 36
                                        height: 36
                                        radius: 18
                                        color: Colors.accent
                                        anchors.centerIn: parent

                                        SoniqIcon {
                                            anchors.centerIn: parent
                                            name: "play"
                                            size: 14
                                            color: Colors.textInverse
                                        }
                                    }
                                }
                            }

                            // Album Title & Artist
                            Column {
                                width: parent.width
                                spacing: 2

                                Text {
                                    width: parent.width
                                    text: modelData.name
                                    font.family: Typography.fontFamily
                                    font.pixelSize: Typography.sizeBody
                                    font.weight: Typography.weightMedium
                                    color: Colors.textPrimary
                                    elide: Text.ElideRight
                                }

                                Row {
                                    width: parent.width
                                    spacing: 4

                                    Text {
                                        text: modelData.artist
                                        font.family: Typography.fontFamily
                                        font.pixelSize: Typography.sizeCaption
                                        color: Colors.textSecondary
                                        elide: Text.ElideRight
                                        width: Math.min(implicitWidth, parent.width - 36)
                                    }

                                    Text {
                                        text: "• " + modelData.trackCount + "t"
                                        font.family: Typography.monospaceFamily
                                        font.pixelSize: Typography.sizeMicro
                                        color: Colors.textMuted
                                    }
                                }
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            }

            // === 2C. ARTISTS GRID VIEW ===
            GridView {
                id: artistGridView
                anchors.fill: parent
                visible: root.activeTab === "artists"
                clip: true
                cellWidth: Math.max(160, Math.floor(width / Math.max(2, Math.floor(width / 170))))
                cellHeight: cellWidth + 40
                model: root.activeTab === "artists" ? libraryModel.getArtists() : []

                delegate: Item {
                    width: artistGridView.cellWidth
                    height: artistGridView.cellHeight

                    SoniqCard {
                        anchors.fill: parent
                        anchors.margins: 6
                        color: artCardMouse.containsMouse ? Colors.surfaceHover : Colors.surfaceCard
                        border.color: artCardMouse.containsMouse ? Colors.accent : Colors.borderSubtle
                        radius: Theme.cornerRadiusMedium

                        MouseArea {
                            id: artCardMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                libraryModel.filterByArtist(modelData.name);
                                root.activeTab = "songs";
                                if (window) window.activeLibraryTab = "songs";
                            }
                        }

                        Column {
                            anchors.centerIn: parent
                            spacing: 10
                            width: parent.width - 20

                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: Math.min(90, parent.width * 0.65)
                                height: width
                                radius: width / 2
                                color: Colors.surfaceElevated
                                border.color: Colors.borderSubtle
                                border.width: 1

                                SoniqIcon {
                                    anchors.centerIn: parent
                                    name: "artists"
                                    size: 32
                                    color: Colors.accent
                                }
                            }

                            Text {
                                text: modelData.name
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                                horizontalAlignment: Text.AlignHCenter
                                elide: Text.ElideRight
                                width: parent.width
                            }

                            Text {
                                text: modelData.trackCount + " tracks • " + modelData.albumCount + " albums"
                                font.family: Typography.monospaceFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                                horizontalAlignment: Text.AlignHCenter
                                width: parent.width
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            }

            // === 2D. GENRES VIEW ===
            GridView {
                id: genreGridView
                anchors.fill: parent
                visible: root.activeTab === "genres"
                clip: true
                cellWidth: 200
                cellHeight: 80
                model: root.activeTab === "genres" ? libraryModel.getGenres() : []

                delegate: Item {
                    width: genreGridView.cellWidth
                    height: genreGridView.cellHeight

                    SoniqCard {
                        anchors.fill: parent
                        anchors.margins: 6
                        color: genMouse.containsMouse ? Colors.surfaceHover : Colors.surfaceCard
                        border.color: genMouse.containsMouse ? Colors.accent : Colors.borderSubtle
                        radius: Theme.cornerRadiusMedium

                        MouseArea {
                            id: genMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                libraryModel.filterByGenre(modelData.name);
                                root.activeTab = "songs";
                                if (window) window.activeLibraryTab = "songs";
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Rectangle {
                                width: 44
                                height: 44
                                radius: 12
                                color: Colors.accentMuted
                                border.color: Colors.accent
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                SoniqIcon {
                                    anchors.centerIn: parent
                                    name: "genres"
                                    size: 20
                                    color: Colors.accent
                                }
                            }

                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - 64
                                spacing: 2

                                Text {
                                    text: modelData.name
                                    font.family: Typography.fontFamily
                                    font.pixelSize: Typography.sizeBody
                                    font.weight: Typography.weightMedium
                                    color: Colors.textPrimary
                                    elide: Text.ElideRight
                                    width: parent.width
                                }

                                Text {
                                    text: modelData.trackCount + " tracks"
                                    font.family: Typography.monospaceFamily
                                    font.pixelSize: Typography.sizeCaption
                                    color: Colors.textMuted
                                }
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            }

            // === 2E. EMPTY STATE FOR SEARCH OR EMPTY LIBRARY ===
            Item {
                anchors.centerIn: parent
                width: parent.width - 60
                height: 180
                visible: (root.activeTab === "songs" && libraryModel.count === 0)

                Column {
                    anchors.centerIn: parent
                    spacing: 12

                    SoniqIcon {
                        anchors.horizontalCenter: parent.horizontalCenter
                        name: "search"
                        size: 36
                        color: Colors.textMuted
                    }

                    Text {
                        text: searchInput.text.length > 0 ? "No Matching Tracks Found" : "No Tracks in this Selection"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeTitleSmall
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: searchInput.text.length > 0 ? "Check your spelling or try clearing the search query." : "Scan a folder to add music to your library."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeCaption
                        color: Colors.textSecondary
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    SoniqButton {
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: searchInput.text.length > 0
                        text: "Clear Search"
                        onClicked: searchInput.text = ""
                    }
                }
            }
        }
    }
}
