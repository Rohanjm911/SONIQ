import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

ScrollView {
    id: root

    clip: true

    Column {
        width: Math.min(880, parent.width - 48)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 20
        topPadding: 24
        bottomPadding: 32

        // 1. Header and Master Toggle
        Item {
            width: parent.width
            height: 56

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
                    id: masterSwitch
                    checked: dspService.enabled
                    onToggled: dspService.enabled = checked
                }
            }
        }

        // 2. Bit-Perfect Warning Banner if DSP is Active
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

        // 3. Controls Bar: Presets & Reset EQ
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
                    currentIndex: {
                        var idx = dspService.presets.indexOf(dspService.currentPreset);
                        return idx >= 0 ? idx : 0;
                    }
                    anchors.verticalCenter: parent.verticalCenter

                    background: Rectangle {
                        implicitWidth: 160
                        implicitHeight: 34
                        color: Colors.surfaceElevated
                        radius: Theme.cornerRadiusSmall
                        border.color: presetCombo.activeFocus ? Colors.accent : Colors.borderSubtle
                        border.width: 1
                    }

                    contentItem: Text {
                        leftPadding: 12
                        rightPadding: 24
                        text: presetCombo.currentText
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        font.weight: Typography.weightMedium
                        color: Colors.textPrimary
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    onActivated: {
                        dspService.applyPreset(currentText);
                    }
                }
            }

            SoniqButton {
                id: resetBtn
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                variant: "secondary"
                iconText: "↺"
                text: "Reset EQ"
                onClicked: {
                    dspService.resetEq();
                }
            }
        }

        // 4. Live Frequency Response Curve Visualizer Card
        SoniqCard {
            width: parent.width
            height: 110

            Column {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 8

                Item {
                    width: parent.width
                    height: 16

                    Text {
                        anchors.left: parent.left
                        text: "FREQUENCY RESPONSE CURVE"
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: Colors.textMuted
                    }

                    Text {
                        anchors.right: parent.right
                        text: dspService.enabled ? (dspService.currentPreset + " (" + (dspService.altersSignal ? "Active EQ" : "Flat") + ")") : "Bypassed"
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        color: dspService.enabled ? Colors.accent : Colors.textMuted
                    }
                }

                Canvas {
                    id: curveCanvas
                    width: parent.width
                    height: 64

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);

                        var midY = height / 2;

                        // Center 0 dB reference dashed line
                        ctx.strokeStyle = "rgba(255, 255, 255, 0.12)";
                        ctx.lineWidth = 1;
                        ctx.beginPath();
                        ctx.moveTo(0, midY);
                        ctx.lineTo(width, midY);
                        ctx.stroke();

                        if (!dspService.enabled) {
                            return;
                        }

                        // Sample curve across 20 Hz to 20000 Hz logarithmically
                        var points = [];
                        var step = Math.max(2, Math.floor(width / 120));
                        for (var px = 0; px <= width; px += step) {
                            var freq = 20.0 * Math.pow(1000.0, px / width);
                            var gainDb = dspService.gainAtFrequency(freq);
                            // Scale: +/-15 dB maps to canvas height
                            var y = midY - (gainDb / 15.0) * (midY * 0.85);
                            y = Math.max(2, Math.min(height - 2, y));
                            points.push({ x: px, y: y });
                        }

                        if (points.length < 2) return;

                        // Draw smooth gradient fill under the curve
                        var grad = ctx.createLinearGradient(0, 0, 0, height);
                        grad.addColorStop(0.0, Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.35));
                        grad.addColorStop(0.5, Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.10));
                        grad.addColorStop(1.0, Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.0));

                        ctx.fillStyle = grad;
                        ctx.beginPath();
                        ctx.moveTo(points[0].x, midY);
                        for (var i = 0; i < points.length; ++i) {
                            ctx.lineTo(points[i].x, points[i].y);
                        }
                        ctx.lineTo(points[points.length - 1].x, midY);
                        ctx.closePath();
                        ctx.fill();

                        // Draw the glowing curve line
                        ctx.strokeStyle = Colors.accent;
                        ctx.lineWidth = 2.5;
                        ctx.beginPath();
                        ctx.moveTo(points[0].x, points[0].y);
                        for (var j = 1; j < points.length; ++j) {
                            ctx.lineTo(points[j].x, points[j].y);
                        }
                        ctx.stroke();
                    }

                    Connections {
                        target: dspService
                        function onDspChanged() {
                            curveCanvas.requestPaint();
                        }
                    }
                }
            }
        }

        // 5. 10-Band Graphic Equalizer Faders Card
        SoniqCard {
            width: parent.width
            height: 310

            Column {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 12

                Item {
                    width: parent.width
                    height: 16

                    Text {
                        anchors.left: parent.left
                        text: "10-BAND GRAPHIC EQUALIZER (ISO OCTAVE)"
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        font.weight: Typography.weightBold
                        color: Colors.textMuted
                    }

                    Text {
                        anchors.right: parent.right
                        text: "Range: -12.0 dB to +12.0 dB"
                        font.family: Typography.monospaceFamily
                        font.pixelSize: Typography.sizeMicro
                        color: Colors.textMuted
                    }
                }

                // 10 Precision Vertical Faders
                Row {
                    id: sliderRow
                    width: parent.width
                    height: 236

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

                        Item {
                            id: faderDelegate
                            width: sliderRow.width / 10
                            height: parent.height

                            property double currentBandGain: (dspService.gains10 && dspService.gains10.length > modelData.band)
                                                             ? dspService.gains10[modelData.band] : 0.0

                            Column {
                                anchors.centerIn: parent
                                spacing: 8

                                // Gain readout (+6.0 dB / -3.5 dB / 0.0 dB)
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: {
                                        var val = faderDelegate.currentBandGain;
                                        return (val > 0 ? "+" : "") + val.toFixed(1) + " dB";
                                    }
                                    font.family: Typography.monospaceFamily
                                    font.pixelSize: Typography.sizeMicro
                                    font.weight: Math.abs(faderDelegate.currentBandGain) > 0.05 ? Typography.weightBold : Typography.weightRegular
                                    color: Math.abs(faderDelegate.currentBandGain) > 0.05 ? Colors.accent : Colors.textMuted
                                }

                                // Audiophile Vertical Slider Fader
                                Slider {
                                    id: bandSlider
                                    orientation: Qt.Vertical
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    height: 160
                                    // In vertical slider: from=12 (top) to=-12 (bottom) so UP is BOOST and DOWN is CUT
                                    from: 12.0
                                    to: -12.0
                                    value: faderDelegate.currentBandGain
                                    enabled: dspService.enabled

                                    onMoved: {
                                        dspService.setBandGain10(modelData.band, value);
                                    }

                                    Connections {
                                        target: dspService
                                        function onDspChanged() {
                                            if (!bandSlider.pressed) {
                                                bandSlider.value = faderDelegate.currentBandGain;
                                            }
                                        }
                                    }

                                    // Custom Fader Track Styling
                                    background: Rectangle {
                                        x: bandSlider.availableWidth / 2 - width / 2
                                        y: bandSlider.topPadding
                                        implicitWidth: 6
                                        implicitHeight: 160
                                        width: 6
                                        height: bandSlider.availableHeight
                                        radius: 3
                                        color: Colors.surfaceActive

                                        // 0 dB center tick mark
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: 14
                                            height: 1
                                            color: "#40FFFFFF"
                                        }

                                        // Active Accent fill to 0 dB line
                                        Rectangle {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            width: parent.width
                                            radius: 3
                                            color: bandSlider.enabled ? Colors.accent : Colors.textMuted

                                            // visualPosition: 0 is top (+12dB), 0.5 is center (0dB), 1.0 is bottom (-12dB)
                                            y: bandSlider.visualPosition < 0.5
                                               ? (bandSlider.visualPosition * bandSlider.availableHeight)
                                               : (0.5 * bandSlider.availableHeight)
                                            height: Math.abs(0.5 - bandSlider.visualPosition) * bandSlider.availableHeight
                                        }
                                    }

                                    // Custom Fader Knob Handle
                                    handle: Rectangle {
                                        x: bandSlider.leftPadding + bandSlider.availableWidth / 2 - width / 2
                                        y: bandSlider.topPadding + bandSlider.visualPosition * (bandSlider.availableHeight - height)
                                        width: (bandSlider.hovered || bandSlider.pressed) ? 22 : 18
                                        height: 12
                                        radius: 3
                                        color: bandSlider.pressed ? Colors.accent : (bandSlider.hovered ? "#FFFFFF" : Colors.surfaceElevated)
                                        border.color: bandSlider.enabled ? (bandSlider.pressed ? "#FFFFFF" : Colors.accent) : Colors.borderSubtle
                                        border.width: 1.5

                                        // Grip ribs in center of knob
                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: parent.width - 6
                                            height: 1
                                            color: bandSlider.pressed ? "#000000" : (bandSlider.hovered ? "#333333" : "#66FFFFFF")
                                        }

                                        Behavior on width {
                                            NumberAnimation { duration: Motion.durationFast }
                                        }
                                    }
                                }

                                // Frequency Label (31 Hz ... 16 kHz)
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
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
        }

        // 6. Preamp & Acoustic Processing Row
        Row {
            width: parent.width
            spacing: 16

            // Master Preamp Gain Card
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
                            text: "MASTER PREAMP GAIN"
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
                            font.weight: Typography.weightBold
                            color: Math.abs(dspService.preampDb) > 0.05 ? Colors.accent : Colors.textPrimary
                        }
                    }

                    SoniqSlider {
                        id: preampSlider
                        width: parent.width
                        from: -12.0
                        to: 12.0
                        value: dspService.preampDb
                        enabled: dspService.enabled
                        onMoved: {
                            dspService.setPreampDb(value);
                        }

                        Connections {
                            target: dspService
                            function onDspChanged() {
                                if (!preampSlider.pressed) {
                                    preampSlider.value = dspService.preampDb;
                                }
                            }
                        }
                    }

                    Text {
                        text: "Digital gain staging before EQ filters to avoid inter-band clipping."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeMicro
                        color: Colors.textMuted
                    }
                }
            }

            // Headphone Crossfeed & Limiter Card
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
                                text: "Bauer stereophonic acoustic simulation for headphones"
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
                                text: "Prevents digital clipping when boosted EQ peaks exceed 0 dBFS"
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
