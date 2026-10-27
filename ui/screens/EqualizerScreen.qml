import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

ScrollView {
    id: root

    clip: true

    Column {
        width: Math.min(840, parent.width - 48)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 24
        topPadding: 24
        bottomPadding: 32

        // Header and Master Toggle
        Item {
            width: parent.width
            height: Math.max(headerCol.implicitHeight, toggleRow.implicitHeight)

            Column {
                id: headerCol
                anchors.left: parent.left
                anchors.right: toggleRow.left
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                Text {
                    text: "Equalizer & DSP"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeTitleLarge
                    font.weight: Typography.weightBold
                    color: Colors.textPrimary
                }
                Text {
                    text: "Precision 64-bit floating point IIR filters and acoustic enhancement."
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeBody
                    color: Colors.textSecondary
                }
            }

            Row {
                id: toggleRow
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Text {
                    text: dspService.enabled ? "ENABLED" : "BYPASS"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: dspService.enabled ? Colors.accent : Colors.textMuted
                    anchors.verticalCenter: parent.verticalCenter
                }

                Switch {
                    checked: dspService.enabled
                    onToggled: dspService.enabled = checked
                }
            }
        }

        // Bit-Perfect Warning Banner if DSP is Active
        SoniqCard {
            width: parent.width
            height: 48
            visible: dspService.altersSignal
            color: Colors.warningMuted
            border.color: Colors.warning

            Row {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Text {
                    text: "⚠️"
                    font.pixelSize: 16
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "DSP is active: Sample values are modified by audio processing. Bit-perfect output is bypassed."
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeBody
                    font.weight: Typography.weightMedium
                    color: Colors.warning
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Controls Bar: Presets & Reset
        Item {
            width: parent.width
            height: 38

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                Text {
                    text: "Preset:"
                    font.family: Typography.fontFamily
                    font.pixelSize: Typography.sizeBody
                    color: Colors.textSecondary
                    anchors.verticalCenter: parent.verticalCenter
                }

                ComboBox {
                    id: presetCombo
                    model: dspService.presets
                    anchors.verticalCenter: parent.verticalCenter
                    onActivated: dspService.applyPreset(currentText)
                }
            }

            SoniqButton {
                id: resetBtn
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                variant: "secondary"
                text: "Reset EQ"
                onClicked: dspService.resetEq()
            }
        }

        // Graphic Equalizer Card
        SoniqCard {
            width: parent.width
            height: 290

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                Text {
                    text: "10-BAND GRAPHIC EQUALIZER (ISO OCTAVE)"
                    font.family: Typography.monospaceFamily
                    font.pixelSize: Typography.sizeMicro
                    font.weight: Typography.weightBold
                    color: Colors.textMuted
                }

                // 10 Frequency Band Sliders
                Row {
                    id: sliderRow
                    width: parent.width
                    height: 210
                    spacing: Math.max(8, (width - (10 * 52)) / 9)

                    Repeater {
                        model: [
                            { label: "31 Hz", band: 0 },
                            { label: "63 Hz", band: 1 },
                            { label: "125 Hz", band: 2 },
                            { label: "250 Hz", band: 3 },
                            { label: "500 Hz", band: 4 },
                            { label: "1 kHz", band: 5 },
                            { label: "2 kHz", band: 6 },
                            { label: "4 kHz", band: 7 },
                            { label: "8 kHz", band: 8 },
                            { label: "16 kHz", band: 9 }
                        ]

                        Column {
                            width: 52
                            height: parent.height
                            spacing: 8

                            Text {
                                width: parent.width
                                horizontalAlignment: Text.AlignHCenter
                                text: {
                                    var val = (dspService.gains10 && dspService.gains10.length > modelData.band)
                                              ? dspService.gains10[modelData.band] : 0.0;
                                    return (val > 0 ? "+" : "") + val.toFixed(1) + " dB";
                                }
                                font.family: Typography.monospaceFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }

                            Slider {
                                id: bandSlider
                                orientation: Qt.Vertical
                                anchors.horizontalCenter: parent.horizontalCenter
                                height: 140
                                from: -12.0
                                to: 12.0
                                value: (dspService.gains10 && dspService.gains10.length > modelData.band)
                                       ? dspService.gains10[modelData.band] : 0.0
                                enabled: dspService.enabled
                                onMoved: dspService.setBandGain10(modelData.band, value)
                            }

                            Text {
                                width: parent.width
                                horizontalAlignment: Text.AlignHCenter
                                text: modelData.label
                                font.family: Typography.monospaceFamily
                                font.pixelSize: Typography.sizeMicro
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                        }
                    }
                }
            }
        }

        // Preamp & Enhancement Processing Row
        Row {
            width: parent.width
            spacing: 16

            // Master Preamp
            SoniqCard {
                width: (parent.width - 16) / 2
                height: 140

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Item {
                        width: parent.width
                        height: 20

                        Text {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            text: "PREAMP GAIN"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: Colors.textMuted
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            text: (dspService.preampDb > 0 ? "+" : "") + dspService.preampDb.toFixed(1) + " dB"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            color: Colors.textPrimary
                        }
                    }

                    SoniqSlider {
                        width: parent.width
                        from: -12.0
                        to: 12.0
                        value: dspService.preampDb
                        enabled: dspService.enabled
                        onMoved: dspService.preampDb = value
                    }
                }
            }

            // Headphone Crossfeed & Limiter
            SoniqCard {
                width: (parent.width - 16) / 2
                height: 140

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    // Crossfeed Row
                    Item {
                        width: parent.width
                        height: 40

                        Column {
                            anchors.left: parent.left
                            anchors.right: cfSwitch.left
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "Binaural Crossfeed"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                            Text {
                                text: "Bauer stereophonic acoustic simulation"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }

                        Switch {
                            id: cfSwitch
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            checked: dspService.crossfeedEnabled
                            onToggled: dspService.crossfeedEnabled = checked
                        }
                    }

                    // Limiter Row
                    Item {
                        width: parent.width
                        height: 40

                        Column {
                            anchors.left: parent.left
                            anchors.right: limSwitch.left
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2

                            Text {
                                text: "Peak Limiter / Anti-Clipping"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeBody
                                font.weight: Typography.weightMedium
                                color: Colors.textPrimary
                            }
                            Text {
                                text: "Prevents digital inter-sample clipping"
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeMicro
                                color: Colors.textMuted
                            }
                        }

                        Switch {
                            id: limSwitch
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            checked: dspService.limiterEnabled
                            onToggled: dspService.limiterEnabled = checked
                        }
                    }
                }
            }
        }
    }
}
