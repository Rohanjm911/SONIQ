import QtQuick
import "../themes"

Item {
    id: root

    property string mode: "bars" // "bars", "waveform", "circular", "off"
    property real peakL: 0.0
    property real peakR: 0.0
    property var waveformData: []
    property bool active: false
    property color visualizerColor: Colors.accent

    clip: true

    Canvas {
        id: canvas
        anchors.fill: parent
        visible: root.mode !== "off"
        renderTarget: Canvas.Image

        // Internal smoothing state for bars
        property var barHeights: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            if (!root.active && root.mode !== "waveform") return;

            var w = width;
            var h = height;

            if (root.mode === "bars") {
                var numBars = 24;
                var barGap = 3;
                var barWidth = Math.max(2, (w - (numBars - 1) * barGap) / numBars);

                var avgPeak = (root.peakL + root.peakR) * 0.5;

                for (var i = 0; i < numBars; i++) {
                    // Create natural frequency curve distribution
                    var freqScale = Math.sin((i / (numBars - 1)) * Math.PI);
                    var target = (avgPeak * freqScale * (0.4 + 0.6 * Math.sin(i * 1.7 + Date.now() * 0.006))) * h * 0.9;
                    if (!root.active) target = 0;

                    // Smooth decay
                    barHeights[i] = (barHeights[i] || 0) * 0.75 + target * 0.25;
                    var bh = Math.max(2, barHeights[i]);

                    var bx = i * (barWidth + barGap);
                    var by = h - bh;

                    var grad = ctx.createLinearGradient(0, by, 0, h);
                    grad.addColorStop(0, root.visualizerColor);
                    grad.addColorStop(1, Qt.darker(root.visualizerColor, 1.4));

                    ctx.fillStyle = grad;
                    ctx.fillRect(bx, by, barWidth, bh);
                }
            } else if (root.mode === "waveform") {
                ctx.strokeStyle = root.visualizerColor;
                ctx.lineWidth = 2.0;
                ctx.beginPath();

                var samples = root.waveformData;
                var count = samples ? samples.length : 0;
                var midY = h * 0.5;

                if (count > 0 && root.active) {
                    var step = w / Math.max(1, count - 1);
                    for (var s = 0; s < count; s++) {
                        var sx = s * step;
                        var sy = midY - (samples[s] * midY * 0.95);
                        if (s === 0) ctx.moveTo(sx, sy);
                        else ctx.lineTo(sx, sy);
                    }
                } else {
                    // Flat baseline when stopped
                    ctx.moveTo(0, midY);
                    ctx.lineTo(w, midY);
                }
                ctx.stroke();
            } else if (root.mode === "circular") {
                var cx = w * 0.5;
                var cy = h * 0.5;
                var radius = Math.min(cx, cy) * 0.55;
                var numRays = 32;
                var avgPeakVal = (root.peakL + root.peakR) * 0.5;

                for (var r = 0; r < numRays; r++) {
                    var angle = (r / numRays) * 2 * Math.PI;
                    var rayAmp = avgPeakVal * (0.3 + 0.7 * Math.abs(Math.sin(r * 2.1 + Date.now() * 0.005))) * (radius * 0.8);
                    if (!root.active) rayAmp = 0;

                    var x1 = cx + Math.cos(angle) * radius;
                    var y1 = cy + Math.sin(angle) * radius;
                    var x2 = cx + Math.cos(angle) * (radius + Math.max(2, rayAmp));
                    var y2 = cy + Math.sin(angle) * (radius + Math.max(2, rayAmp));

                    ctx.strokeStyle = root.visualizerColor;
                    ctx.lineWidth = 2.0;
                    ctx.beginPath();
                    ctx.moveTo(x1, y1);
                    ctx.lineTo(x2, y2);
                    ctx.stroke();
                }

                // Inner circle
                ctx.strokeStyle = Qt.rgba(root.visualizerColor.r, root.visualizerColor.g, root.visualizerColor.b, 0.4);
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.arc(cx, cy, radius, 0, 2 * Math.PI);
                ctx.stroke();
            }
        }
    }

    Timer {
        id: repaintTimer
        interval: 33 // 30 FPS smooth rendering with zero lag
        running: root.active && root.mode !== "off"
        repeat: true
        onTriggered: canvas.requestPaint()
    }

    onActiveChanged: canvas.requestPaint()
    onModeChanged: canvas.requestPaint()
    onPeakLChanged: if (!repaintTimer.running) canvas.requestPaint()
}

