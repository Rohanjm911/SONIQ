import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import "../themes"
import "../components"

ScrollView {
    id: root

    clip: true

    FileDialog {
        id: fileDialog
        title: "Select Audio File to Inspect"
        nameFilters: ["Audio Files (*.flac *.wav *.mp3 *.m4a *.aac *.ogg *.opus *.aiff *.ape *.wv *.dsf *.dff)"]
        onAccepted: {
            var path = fileDialog.selectedFile.toString();
            audioInspector.inspectFile(path);
        }
    }

    Column {
        width: Math.min(840, parent.width - 48)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 24
        topPadding: 24
        bottomPadding: 32

        // Header
        Item {
            width: parent.width
            height: Math.max(headerCol.implicitHeight, inspectBtn.implicitHeight)

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
                    text: "Deep container, codec, audio stream analysis, and bit-perfect verification audit."
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
                iconText: "🔍"
                onClicked: fileDialog.open()
            }
        }

        // Active Track or Inspected File Summary
        SoniqCard {
            width: parent.width
            height: 96

            Item {
                anchors.fill: parent
                anchors.margins: 20

                Rectangle {
                    id: iconRect
                    width: 56
                    height: 56
                    radius: 12
                    color: Colors.surfaceElevated
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "♫"
                        font.pixelSize: 24
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

        // Audio Stream Specifications Grid
        SoniqCard {
            width: parent.width
            height: 230

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                Text {
                    text: "AUDIO STREAM SPECIFICATIONS"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                }

                Grid {
                    id: specGrid
                    columns: 3
                    width: parent.width
                    rowSpacing: 18
                    columnSpacing: 16

                    Repeater {
                        model: [
                            { label: "Container Format", val: audioInspector.containerFormat.length > 0 ? audioInspector.containerFormat : "WAV (RIFF)" },
                            { label: "Codec Name", val: audioInspector.codecLongName.length > 0 ? audioInspector.codecLongName : "Linear PCM" },
                            { label: "Sample Rate", val: (audioInspector.sampleRate > 0 ? (audioInspector.sampleRate / 1000.0).toFixed(1) : "44.1") + " kHz" },
                            { label: "Bit Depth", val: (audioInspector.bitDepth > 0 ? audioInspector.bitDepth : 16) + "-bit" },
                            { label: "Channel Layout", val: audioInspector.channels === 1 ? "Mono (1.0)" : "Stereo (2.0)" },
                            { label: "Bitrate", val: (audioInspector.bitrate > 0 ? audioInspector.bitrate : 1411) + " kbps" }
                        ]

                        Column {
                            width: (specGrid.width - 32) / 3
                            spacing: 4

                            Text {
                                text: modelData.label
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeCaption
                                color: Colors.textMuted
                            }

                            Text {
                                text: modelData.val
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
        }

        // Bit-Perfect Eligibility Assessment
        SoniqCard {
            width: parent.width
            height: 120

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Row {
                    spacing: 8
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: audioInspector.isBitPerfectEligible ? Colors.success : Colors.warning
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "BIT-PERFECT AUDIT"
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
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
