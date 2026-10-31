import QtQuick
import QtQuick.Controls.Basic
import "../themes"
import "../components"

Item {
    id: root

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
            width: Math.min(960, Math.max(520, flickable.width - 48))
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 24
            spacing: 20

            // 1. Header & Master Bypass Toggle
            Item {
                width: parent.width
                height: 52

                Column {
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
                        font.pixelSize: Typography.sizeCaption
                        font.weight: Typography.weightBold
                        color: dspService.enabled ? Colors.accent : Colors.textMuted
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Switch {
                        id: masterSwitch
                        checked: dspService.enabled
                        anchors.verticalCenter: parent.verticalCenter
                        onToggled: dspService.enabled = checked
                    }
                }
            }

            // 2. Bit-Perfect Warning Banner (Visible when DSP modifies audio)
            SoniqCard {
                width: parent.width
                height: 48
                visible: dspService.altersSignal
                color: Colors.warningMuted
                border.color: Colors.warning
                radius: Theme.cornerRadiusSmall

                Row {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    SoniqIcon {
                        name: "warning"
                        size: 16
                        color: Colors.warning
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "DSP is active: Audio samples are modified by the DSP pipeline. Bit-perfect direct hardware output is bypassed."
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        font.weight: Typography.weightMedium
                        color: Colors.warning
                        anchors.verticalCenter: parent.verticalCenter
                        elide: Text.ElideRight
                        width: parent.width - 40
                    }
                }
            }

            // 3. Preset Selector & Quick Reset Bar
            Item {
                width: parent.width
                height: 40

                Row {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 12

                    Text {
                        text: "Preset:"
                        font.family: Typography.fontFamily
                        font.pixelSize: Typography.sizeBody
                        font.weight: Typography.weightMedium
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

                        implicitWidth: 180
                        implicitHeight: 36

                        background: Rectangle {
                            color: Colors.surfaceElevated
                            radius: Theme.cornerRadiusSmall
                            border.color: presetCombo.activeFocus ? Colors.accent : Colors.borderMedium
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

                        popup: Popup {
                            y: presetCombo.height + 4
                            width: presetCombo.width
                            padding: 4
                            background: Rectangle {
                                color: Colors.surfaceElevated
                                border.color: Colors.borderMedium
                                border.width: 1
                                radius: Theme.cornerRadiusSmall
                            }
                            contentItem: ListView {
                                clip: true
                                implicitHeight: Math.min(240, contentHeight)
                                model: presetCombo.popup.visible ? presetCombo.delegateModel : null
                                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                            }
                        }

                        delegate: ItemDelegate {
                            width: presetCombo.width - 8
                            height: 32
                            highlighted: presetCombo.highlightedIndex === index

                            background: Rectangle {
                                color: highlighted ? Colors.surfaceHover : "transparent"
                                radius: 4
                            }

                            contentItem: Text {
                                text: modelData
                                font.family: Typography.fontFamily
                                font.pixelSize: Typography.sizeCaption
                                font.weight: (modelData === dspService.currentPreset) ? Typography.weightBold : Typography.weightRegular
                                color: (modelData === dspService.currentPreset) ? Colors.accent : Colors.textPrimary
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 8
                            }

                            onClicked: {
                                presetCombo.currentIndex = index;
                                dspService.applyPreset(modelData);
                                presetCombo.popup.close();
                            }
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
                    iconText: "recent"
                    text: "Reset EQ"
                    onClicked: {
                        dspService.resetEq();
                    }
                }
            }

            // 4. Live Frequency Response Curve Visualizer Card
            SoniqCard {
                width: parent.width
                height: 156
                color: Colors.surfaceCard
                border.color: Colors.borderMedium
                radius: Theme.cornerRadiusMedium

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    Item {
                        width: parent.width
                        height: 18

                        Row {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            SoniqIcon {
                                name: "equalizer"
                                size: 14
                                color: Colors.accent
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: "FREQUENCY RESPONSE CURVE"
                                font.family: Typography.monospaceFamily
                                font.pixelSize: Typography.sizeMicro
                                font.weight: Typography.weightBold
                                font.letterSpacing: Typography.letterSpacingCaps
                                color: Colors.textMuted
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            text: dspService.enabled ? (dspService.currentPreset + (dspService.altersSignal ? " (Active EQ)" : " (Flat)")) : "Bypassed"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            font.weight: Typography.weightBold
                            color: dspService.enabled ? Colors.accent : Colors.textMuted
                        }
                    }

                    Canvas {
                        id: curveCanvas
                        width: parent.width
                        height: 96

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.clearRect(0, 0, width, height);

                            var midY = height / 2;

                            // dB Grid Reference lines (-12, -6, 0, +6, +12)
                            ctx.lineWidth = 1;
                            var gridDbs = [-12, -6, 0, 6, 12];
                            for (var g = 0; g < gridDbs.length; g++) {
                                var gy = midY - (gridDbs[g] / 15.0) * (midY * 0.85);
                                ctx.strokeStyle = (gridDbs[g] === 0) ? "rgba(255, 255, 255, 0.20)" : "rgba(255, 255, 255, 0.06)";
                                ctx.beginPath();
                                ctx.moveTo(0, gy);
                                ctx.lineTo(width, gy);
                                ctx.stroke();

                                // dB labels on left
                                ctx.fillStyle = "rgba(255, 255, 255, 0.4)";
                                ctx.font = "9px monospace";
                                ctx.fillText((gridDbs[g] > 0 ? "+" : "") + gridDbs[g], 4, gy - 2);
                            }

                            if (!dspService.enabled) {
                                // Draw flat neutral reference line
                                ctx.strokeStyle = Colors.textMuted;
                                ctx.lineWidth = 1.5;
                                ctx.beginPath();
                                ctx.moveTo(0, midY);
                                ctx.lineTo(width, midY);
                                ctx.stroke();
                                return;
                            }

                            // Sample frequency response curve logarithmically from 20 Hz to 20,000 Hz
                            var points = [];
                            var step = Math.max(2, Math.floor(width / 120));
                            for (var px = 0; px <= width; px += step) {
                                var freq = 20.0 * Math.pow(1000.0, px / width);
                                var gainDb = dspService.gainAtFrequency(freq);
                                var y = midY - (gainDb / 15.0) * (midY * 0.85);
                                y = Math.max(2, Math.min(height - 2, y));
                                points.push({ x: px, y: y });
                            }

                            if (points.length < 2) return;

                            // Smooth gradient fill below curve
                            var grad = ctx.createLinearGradient(0, 0, 0, height);
                            grad.addColorStop(0.0, Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.35));
                            grad.addColorStop(0.5, Qt.rgba(Colors.accent.r, Colors.accent.g, Colors.accent.b, 0.10));
                            grad.addColorStop(1.0, "transparent");

                            ctx.fillStyle = grad;
                            ctx.beginPath();
                            ctx.moveTo(points[0].x, midY);
                            for (var i = 0; i < points.length; ++i) {
                                ctx.lineTo(points[i].x, points[i].y);
                            }
                            ctx.lineTo(points[points.length - 1].x, midY);
                            ctx.closePath();
                            ctx.fill();

                            // Glowing curve line
                            ctx.strokeStyle = Colors.accent;
                            ctx.lineWidth = 2.5;
                            ctx.beginPath();
                            ctx.moveTo(points[0].x, points[0].y);
                            for (var j = 1; j < points.length; ++j) {
                                ctx.lineTo(points[j].x, points[j].y);
                            }
                            ctx.stroke();
                        }

                        onWidthChanged: requestPaint()
                        onHeightChanged: requestPaint()

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
                height: 330
                color: Colors.surfaceCard
                border.color: Colors.borderMedium
                radius: Theme.cornerRadiusMedium

                Column {
                    anchors.fill: parent
                    anchors.margins: 18
                    spacing: 12

                    Item {
                        width: parent.width
                        height: 18

                        Row {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8

                            SoniqIcon {
                                name: "equalizer"
                                size: 14
                                color: Colors.accent
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: "10-BAND GRAPHIC EQUALIZER (ISO OCTAVE)"
                                font.family: Typography.monospaceFamily
                                font.pixelSize: Typography.sizeMicro
                                font.weight: Typography.weightBold
                                font.letterSpacing: Typography.letterSpacingCaps
                                color: Colors.textMuted
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            text: "Range: -12.0 dB to +12.0 dB"
                            font.family: Typography.monospaceFamily
                            font.pixelSize: Typography.sizeMicro
                            color: Colors.textMuted
                        }
                    }

                    // 10 Precision Vertical Faders Row
                    Row {
                        id: sliderRow
                        width: parent.width
                        height: 256

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

                                    // Gain readout badge (+6.0 dB / 0.0 dB / -3.5 dB)
                                    Rectangle {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        width: 52
                                        height: 20
                                        radius: 4
                                        color: Math.abs(faderDelegate.currentBandGain) > 0.05 ? Colors.accentMuted : Colors.surfaceElevated
                                        border.color: Math.abs(faderDelegate.currentBandGain) > 0.05 ? Colors.accent : Colors.borderSubtle
                                        border.width: 1

                                        Text {
                                            anchors.centerIn: parent
                                            text: {
                                                var val = faderDelegate.currentBandGain;
                                                return (val > 0 ? "+" : "") + val.toFixed(1);
                                            }
                                            font.family: Typography.monospaceFamily
                                            font.pixelSize: Typography.sizeMicro
                                            font.weight: Typography.weightBold
                                            color: Math.abs(faderDelegate.currentBandGain) > 0.05 ? Colors.accent : Colors.textMuted
                                        }
                                    }

                                    // Vertical Fader Slider
                                    Slider {
                                        id: bandSlider
                                        orientation: Qt.Vertical
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        height: 170
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

                                        // Fader Track Groove
                                        background: Rectangle {
                                            x: bandSlider.availableWidth / 2 - width / 2
                                            y: bandSlider.topPadding
                                            implicitWidth: 6
                                            implicitHeight: 170
                                            width: 6
                                            height: bandSlider.availableHeight
                                            radius: 3
                                            color: Colors.surfaceActive

                                            // Center 0 dB Reference Tick
                                            Rectangle {
                                                anchors.centerIn: parent
                                                width: 16
                                                height: 1
                                                color: Qt.rgba(1.0, 1.0, 1.0, 0.30)
                                            }

                                            // Dynamic Accent Fill to 0 dB line
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

                                        // Console Hardware Fader Knob
                                        handle: Rectangle {
                                            x: bandSlider.leftPadding + bandSlider.availableWidth / 2 - width / 2
                                            y: bandSlider.topPadding + bandSlider.visualPosition * (bandSlider.availableHeight - height)
                                            width: 24
                                            height: 14
                                            radius: 3
                                            color: bandSlider.pressed ? Colors.accent : (bandSlider.hovered ? "#FFFFFF" : Colors.surfaceElevated)
                                            border.color: bandSlider.enabled ? (bandSlider.pressed ? "#FFFFFF" : Colors.accent) : Colors.borderSubtle
                                            border.width: 1.5

                                            // Tactile grip line
                                            Rectangle {
                                                anchors.centerIn: parent
                                                width: parent.width - 6
                                                height: 1.5
                                                color: bandSlider.pressed ? "#000000" : (bandSlider.hovered ? "#333333" : "#88FFFFFF")
                                            }
                                        }
                                    }

                                    // Frequency Label
                                    Text {
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: modelData.label
                                        font.family: Typography.monospaceFamily
                                        font.pixelSize: Typography.sizeCaption
                                        font.weight: Typography.weightBold
                                        color: Colors.textPrimary
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // 6. Preamp Gain & Acoustic Enhancement Row
            Row {
                width: parent.width
                spacing: 16

                // Master Preamp Gain Card
                SoniqCard {
                    width: (parent.width - 16) / 2
                    height: 148
                    color: Colors.surfaceCard
                    border.color: Colors.borderMedium
                    radius: Theme.cornerRadiusMedium

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
                                font.letterSpacing: Typography.letterSpacingCaps
                                color: Colors.textMuted
                            }

                            Rectangle {
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                width: 56
                                height: 20
                                radius: 4
                                color: Math.abs(dspService.preampDb) > 0.05 ? Colors.accentMuted : Colors.surfaceElevated
                                border.color: Math.abs(dspService.preampDb) > 0.05 ? Colors.accent : Colors.borderSubtle
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: (dspService.preampDb > 0 ? "+" : "") + dspService.preampDb.toFixed(1) + " dB"
                                    font.family: Typography.monospaceFamily
                                    font.pixelSize: Typography.sizeMicro
                                    font.weight: Typography.weightBold
                                    color: Math.abs(dspService.preampDb) > 0.05 ? Colors.accent : Colors.textPrimary
                                }
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
                                dspService.preampDb = value;
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
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                    }
                }

                // Headphone Crossfeed & Limiter Card
                SoniqCard {
                    width: (parent.width - 16) / 2
                    height: 148
                    color: Colors.surfaceCard
                    border.color: Colors.borderMedium
                    radius: Theme.cornerRadiusMedium

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 10

                        // Crossfeed Row
                        Item {
                            width: parent.width
                            height: 38

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
                                    text: "Bauer acoustic simulation reducing headphone fatigue"
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

                        Rectangle { width: parent.width; height: 1; color: Colors.borderSubtle }

                        // Limiter Row
                        Item {
                            width: parent.width
                            height: 38

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
}
