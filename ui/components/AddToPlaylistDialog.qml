import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "../themes"

Item {
    id: root

    property bool isOpen: false
    property int trackId: 0
    property string trackTitle: ""
    property string trackArtist: ""

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
        width: Math.min(460, parent.width - 32)
        height: 380
        anchors.centerIn: parent
        radius: Theme.cornerRadiusMedium

        MouseArea {
            anchors.fill: parent
        }

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 14

            // Header Row
            Item {
                width: parent.width
                height: 28

                Row {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8

                    Text {
                        text: "Add to Playlist"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeTitleMedium
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
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

            // Track summary banner
            Rectangle {
                width: parent.width
                height: 38
                radius: Theme.cornerRadiusSmall
                color: Colors.surfaceElevated
                border.color: Colors.borderSubtle
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 8

                    Text {
                        text: "♫"
                        font.pixelSize: 14
                        color: Colors.accent
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: root.trackTitle + (root.trackArtist.length > 0 ? (" — " + root.trackArtist) : "")
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeCaption
                        font.weight: Typography.weightMedium
                        color: Colors.textPrimary
                        elide: Text.ElideRight
                        width: parent.width - 40
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // Playlist list header
            Text {
                text: "SELECT DESTINATION PLAYLIST"
                font.family: Typography.monospaceFamily
                font.pixelSize: Typography.sizeMicro
                font.weight: Typography.weightBold
                color: Colors.textMuted
            }

            Rectangle {
                width: parent.width
                height: 180
                radius: Theme.cornerRadiusSmall
                color: Colors.surfaceElevated
                border.color: Colors.borderSubtle
                border.width: 1
                clip: true

                ListView {
                    id: plListView
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 4
                    model: libraryModel.playlists

                    delegate: Rectangle {
                        width: plListView.width
                        height: 38
                        radius: 6
                        color: itemMouse.containsMouse ? Colors.surfaceHover : "transparent"

                        MouseArea {
                            id: itemMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                libraryModel.addTrackToPlaylist(modelData.id, root.trackId);
                                root.close();
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 8

                            Text {
                                text: "☰"
                                font.pixelSize: 13
                                color: Colors.accent
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: modelData.name
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                color: Colors.textPrimary
                                anchors.verticalCenter: parent.verticalCenter
                                elide: Text.ElideRight
                                width: Math.max(80, parent.width - 150)
                            }

                            Text {
                                text: modelData.trackCount + (modelData.trackCount === 1 ? " track" : " tracks")
                                font.family: Typography.monospaceFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Item { width: 1; height: 1; Layout.fillWidth: true }

                            Text {
                                text: "＋"
                                font.pixelSize: 14
                                font.weight: Typography.weightBold
                                color: Colors.accent
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }

                    // Empty state if no playlists exist yet
                    Text {
                        anchors.centerIn: parent
                        visible: plListView.count === 0
                        text: "No playlists created yet."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeCaption
                        color: Colors.textMuted
                    }
                }
            }

            // Bottom Buttons
            Row {
                anchors.right: parent.right
                spacing: 10

                SoniqButton {
                    variant: "secondary"
                    text: "+ New Playlist"
                    onClicked: {
                        root.close();
                        createPlaylistDialog.open();
                    }
                }

                SoniqButton {
                    variant: "secondary"
                    text: "Close"
                    onClicked: root.close()
                }
            }
        }
    }

    function openWith(id, title, artist) {
        root.trackId = id;
        root.trackTitle = title || "Selected Track";
        root.trackArtist = artist || "";
        root.isOpen = true;
    }

    function close() {
        root.isOpen = false;
        root.trackId = 0;
    }
}
