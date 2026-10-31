import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import "../themes"
import "../components"

Item {
    id: root

    FileDialog {
        id: fileDialog
        title: "Select Audio File to Inspect"
        nameFilters: ["Audio Files (*.flac *.wav *.mp3 *.m4a *.aac *.ogg *.opus *.aiff *.ape *.wv *.dsf *.dff)"]
        onAccepted: {
            var path = fileDialog.selectedFile.toString();
            audioInspector.inspectFile(path);
        }
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: width
        contentHeight: contentCol.implicitHeight + 64
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Column {
            id: contentCol
            width: Math.min(880, Math.max(480, flickable.width - 48))
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 24
            spacing: 20

        // Header
        Item {
            width: parent.width
            height: 56

            Column {
                id: headerCol
                anchors.left: parent.left
                anchors.right: inspectBtn.left
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                Text {
                    text: "Audio File Inspector"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeTitleLarge
                    font.weight: Typography.weightBold
                    color: Colors.textPrimary
                }
                Text {
                    text: "Container, codec, audio stream analysis, and bit-perfect hardware verification audit."
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeBody
                    color: Colors.textSecondary
                }
            }

            SoniqButton {
                id: inspectBtn
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: "Inspect File"
                iconText: "search"
                onClicked: fileDialog.open()
            }
        }

        // Active Track or Inspected File Summary
        SoniqCard {
            width: parent.width
            height: 96
            color: Colors.surfaceCard

            Item {
                anchors.fill: parent
                anchors.margins: 18

                Rectangle {
                    id: iconRect
                    width: 56
                    height: 56
                    radius: Theme.cornerRadiusMedium
                    color: Colors.surfaceElevated
                    border.color: Colors.borderSubtle
                    border.width: 1
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    SoniqIcon {
                        anchors.centerIn: parent
                        name: "songs"
                        size: 26
                        color: Colors.accent
                    }
                }

                Column {
                    anchors.left: iconRect.right
                    anchors.right: parent.right
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Text {
                        width: parent.width
                        text: audioInspector.title.length > 0 ? audioInspector.title : playbackController.trackTitle
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeTitleMedium
                        font.weight: Typography.weightBold
                        color: Colors.textPrimary
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: audioInspector.technicalSummary.length > 0 ? audioInspector.technicalSummary : playbackController.trackFormat
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeBody
                        color: Colors.textSecondary
                        elide: Text.ElideRight
                    }
                }
            }
        }

        // Section 1: Audio Stream Specifications Grid
        SoniqCard {
            width: parent.width
            height: 220
            color: Colors.surfaceCard

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 16

                Text {
                    text: "AUDIO STREAM SPECIFICATIONS"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    font.letterSpacing: Typography.letterSpacingCaps
                    color: Colors.textMuted
                }

                // Row 1: Container Format | Codec Name | Sample Rate
                Item {
                    width: parent.width
                    height: 52

                    Column {
                        width: Math.floor((parent.width - 32) / 3)
                        anchors.left: parent.left
                        spacing: 4

                        Text {
                            text: "Container Format"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            color: Colors.textMuted
                        }
                        Text {
                            text: audioInspector.containerFormat.length > 0 ? audioInspector.containerFormat : "Linear WAV"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleSmall
                            font.weight: Typography.weightMedium
                            color: Colors.textPrimary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    Column {
                        width: Math.floor((parent.width - 32) / 3)
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 4

                        Text {
                            text: "Codec Name"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            color: Colors.textMuted
                        }
                        Text {
                            text: audioInspector.codecLongName.length > 0 ? audioInspector.codecLongName : "PCM"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleSmall
                            font.weight: Typography.weightMedium
                            color: Colors.textPrimary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    Column {
                        width: Math.floor((parent.width - 32) / 3)
                        anchors.right: parent.right
                        spacing: 4

                        Text {
                            text: "Sample Rate"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            color: Colors.textMuted
                        }
                        Text {
                            text: (audioInspector.sampleRate > 0 ? (audioInspector.sampleRate / 1000.0).toFixed(1) : "44.1") + " kHz"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleSmall
                            font.weight: Typography.weightMedium
                            color: Colors.textPrimary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }
                }

                // Row 2: Bit Depth | Channel Layout | Bitrate
                Item {
                    width: parent.width
                    height: 52

                    Column {
                        width: Math.floor((parent.width - 32) / 3)
                        anchors.left: parent.left
                        spacing: 4

                        Text {
                            text: "Bit Depth"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            color: Colors.textMuted
                        }
                        Text {
                            text: (audioInspector.bitDepth > 0 ? audioInspector.bitDepth : 16) + "-bit"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleSmall
                            font.weight: Typography.weightMedium
                            color: Colors.textPrimary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    Column {
                        width: Math.floor((parent.width - 32) / 3)
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 4

                        Text {
                            text: "Channel Layout"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            color: Colors.textMuted
                        }
                        Text {
                            text: audioInspector.channels === 1 ? "Mono (1.0)" : "Stereo (2.0)"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleSmall
                            font.weight: Typography.weightMedium
                            color: Colors.textPrimary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    Column {
                        width: Math.floor((parent.width - 32) / 3)
                        anchors.right: parent.right
                        spacing: 4

                        Text {
                            text: "Bitrate"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeCaption
                            color: Colors.textMuted
                        }
                        Text {
                            text: (audioInspector.bitrate > 0 ? audioInspector.bitrate : 1411) + " kbps"
                            font.family: Typography.fontFamily
                            font.pixelSize: Typography.sizeTitleSmall
                            font.weight: Typography.weightMedium
                            color: Colors.textPrimary
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }
                }
            }
        }

        // Section 2: Bit-Perfect Hardware Audit Card
        SoniqCard {
            width: parent.width
            height: 120
            color: Colors.surfaceCard

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 10

                Row {
                    spacing: 8
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: audioInspector.isBitPerfectEligible ? Colors.bitPerfectVerified : Colors.warning
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "BIT-PERFECT OUTPUT VERIFICATION AUDIT"
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        font.letterSpacing: Typography.letterSpacingCaps
                        color: Colors.textMuted
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Text {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: audioInspector.bitPerfectAssessment.length > 0
                          ? audioInspector.bitPerfectAssessment
                          : (playbackController.bitPerfectStatus === "Verified"
                             ? "Source stream is bit-perfect eligible. With WASAPI Exclusive mode and DSP bypassed, original raw samples pass untouched to DAC."
                             : "Signal is currently being routed through shared mixer or DSP pipeline.")
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeBody
                    color: Colors.textSecondary
                }
            }
        }
    }
}
}
