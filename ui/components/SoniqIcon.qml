import QtQuick
import "../themes"

Item {
    id: root

    property string name: ""
    property color color: Colors.textPrimary
    property int size: 16

    implicitWidth: size
    implicitHeight: size
    width: size
    height: size

    readonly property bool isVector: {
        var n = name.toLowerCase();
        return n === "prev" || n === "previous" || name === "\u23EE" ||
               n === "next" || name === "\u23ED" ||
               n === "equalizer" || n === "eq" || name === "\uD83C\uDF9A" || name === "🎚" ||
               n === "inspector" || n === "search" || name === "\uD83D\uDD0D" || name === "🔍" ||
               n === "volume" || n === "vol" || name === "🔊" || name === "🔉" ||
               n === "mute" || name === "🔇" ||
               n === "shuffle" || name === "🔀" ||
               n === "repeat" ||
               n === "fullscreen" || n === "exit_fullscreen";
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        visible: root.isVector
        renderTarget: Canvas.Image

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var w = width;
            var h = height;
            var n = root.name.toLowerCase();
            var pad = 2;

            if (n === "prev" || n === "previous" || root.name === "\u23EE") {
                ctx.fillStyle = root.color;
                var barW = Math.max(2, Math.round(w * 0.14));
                var padY = h * 0.16;
                ctx.beginPath();
                ctx.rect(pad, padY, barW, h - 2 * padY);
                ctx.fill();

                ctx.beginPath();
                ctx.moveTo(w - pad, padY);
                ctx.lineTo(pad + barW + 1, h / 2);
                ctx.lineTo(w - pad, h - padY);
                ctx.closePath();
                ctx.fill();
            } else if (n === "next" || root.name === "\u23ED") {
                ctx.fillStyle = root.color;
                var barW = Math.max(2, Math.round(w * 0.14));
                var padY = h * 0.16;
                ctx.beginPath();
                ctx.moveTo(pad, padY);
                ctx.lineTo(w - pad - barW - 1, h / 2);
                ctx.lineTo(pad, h - padY);
                ctx.closePath();
                ctx.fill();

                ctx.beginPath();
                ctx.rect(w - pad - barW, padY, barW, h - 2 * padY);
                ctx.fill();
            } else if (n === "equalizer" || n === "eq" || root.name === "🎚" || root.name === "\uD83C\uDF9A") {
                ctx.strokeStyle = root.color;
                ctx.fillStyle = root.color;
                ctx.lineWidth = 1.6;
                var x1 = w * 0.22, x2 = w * 0.5, x3 = w * 0.78;
                var topY = h * 0.14, botY = h * 0.86;

                // Track 1
                ctx.beginPath(); ctx.moveTo(x1, topY); ctx.lineTo(x1, botY); ctx.stroke();
                ctx.beginPath(); ctx.arc(x1, h * 0.32, 2.5, 0, 2 * Math.PI); ctx.fill();

                // Track 2
                ctx.beginPath(); ctx.moveTo(x2, topY); ctx.lineTo(x2, botY); ctx.stroke();
                ctx.beginPath(); ctx.arc(x2, h * 0.68, 2.5, 0, 2 * Math.PI); ctx.fill();

                // Track 3
                ctx.beginPath(); ctx.moveTo(x3, topY); ctx.lineTo(x3, botY); ctx.stroke();
                ctx.beginPath(); ctx.arc(x3, h * 0.44, 2.5, 0, 2 * Math.PI); ctx.fill();
            } else if (n === "inspector" || n === "search" || root.name === "🔍" || root.name === "\uD83D\uDD0D") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.8;
                var r = w * 0.25;
                var cx = w * 0.40, cy = h * 0.40;
                ctx.beginPath();
                ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                ctx.stroke();

                var cos45 = 0.7071;
                ctx.beginPath();
                ctx.moveTo(cx + r * cos45, cy + r * cos45);
                ctx.lineTo(w * 0.84, h * 0.84);
                ctx.stroke();
            } else if (n === "volume" || n === "vol" || root.name === "🔊" || root.name === "🔉") {
                ctx.fillStyle = root.color;
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.rect(w * 0.08, h * 0.34, w * 0.18, h * 0.32);
                ctx.fill();

                ctx.beginPath();
                ctx.moveTo(w * 0.26, h * 0.34);
                ctx.lineTo(w * 0.52, h * 0.14);
                ctx.lineTo(w * 0.52, h * 0.86);
                ctx.lineTo(w * 0.26, h * 0.66);
                ctx.closePath();
                ctx.fill();

                ctx.beginPath();
                ctx.arc(w * 0.42, h * 0.5, w * 0.25, -0.6, 0.6);
                ctx.stroke();

                ctx.beginPath();
                ctx.arc(w * 0.42, h * 0.5, w * 0.40, -0.6, 0.6);
                ctx.stroke();
            } else if (n === "mute" || root.name === "🔇") {
                ctx.fillStyle = root.color;
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.rect(w * 0.08, h * 0.34, w * 0.18, h * 0.32);
                ctx.fill();

                ctx.beginPath();
                ctx.moveTo(w * 0.26, h * 0.34);
                ctx.lineTo(w * 0.52, h * 0.14);
                ctx.lineTo(w * 0.52, h * 0.86);
                ctx.lineTo(w * 0.26, h * 0.66);
                ctx.closePath();
                ctx.fill();

                // X mark
                ctx.beginPath();
                ctx.moveTo(w * 0.66, h * 0.36);
                ctx.lineTo(w * 0.88, h * 0.64);
                ctx.moveTo(w * 0.88, h * 0.36);
                ctx.lineTo(w * 0.66, h * 0.64);
                ctx.stroke();
            } else if (n === "shuffle" || root.name === "🔀") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.6;
                // Crossed arrows
                ctx.beginPath();
                ctx.moveTo(w * 0.12, h * 0.32);
                ctx.lineTo(w * 0.38, h * 0.32);
                ctx.lineTo(w * 0.62, h * 0.68);
                ctx.lineTo(w * 0.84, h * 0.68);
                ctx.stroke();

                ctx.beginPath();
                ctx.moveTo(w * 0.76, h * 0.56);
                ctx.lineTo(w * 0.86, h * 0.68);
                ctx.lineTo(w * 0.76, h * 0.80);
                ctx.stroke();

                ctx.beginPath();
                ctx.moveTo(w * 0.12, h * 0.68);
                ctx.lineTo(w * 0.38, h * 0.68);
                ctx.lineTo(w * 0.62, h * 0.32);
                ctx.lineTo(w * 0.84, h * 0.32);
                ctx.stroke();

                ctx.beginPath();
                ctx.moveTo(w * 0.76, h * 0.20);
                ctx.lineTo(w * 0.86, h * 0.32);
                ctx.lineTo(w * 0.76, h * 0.44);
                ctx.stroke();
            } else if (n === "repeat") {
                ctx.strokeStyle = root.color;
                ctx.fillStyle = root.color;
                ctx.lineWidth = 1.6;
                ctx.beginPath();
                ctx.arc(w * 0.5, h * 0.5, w * 0.32, 0.4, 2 * Math.PI - 0.4);
                ctx.stroke();

                ctx.beginPath();
                ctx.moveTo(w * 0.70, h * 0.20);
                ctx.lineTo(w * 0.88, h * 0.34);
                ctx.lineTo(w * 0.68, h * 0.42);
                ctx.closePath();
                ctx.fill();
            } else if (n === "fullscreen") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                var m = 2, len = 4;
                // Top-left
                ctx.beginPath(); ctx.moveTo(m, m + len); ctx.lineTo(m, m); ctx.lineTo(m + len, m); ctx.stroke();
                // Top-right
                ctx.beginPath(); ctx.moveTo(w - m - len, m); ctx.lineTo(w - m, m); ctx.lineTo(w - m, m + len); ctx.stroke();
                // Bottom-left
                ctx.beginPath(); ctx.moveTo(m, h - m - len); ctx.lineTo(m, h - m); ctx.lineTo(m + len, h - m); ctx.stroke();
                // Bottom-right
                ctx.beginPath(); ctx.moveTo(w - m - len, h - m); ctx.lineTo(w - m, h - m); ctx.lineTo(w - m, h - m - len); ctx.stroke();
            } else if (n === "exit_fullscreen") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                var m = 2, len = 4;
                // Top-left pointed inward
                ctx.beginPath(); ctx.moveTo(m + len, m); ctx.lineTo(m + len, m + len); ctx.lineTo(m, m + len); ctx.stroke();
                // Top-right pointed inward
                ctx.beginPath(); ctx.moveTo(w - m - len, m); ctx.lineTo(w - m - len, m + len); ctx.lineTo(w - m, m + len); ctx.stroke();
                // Bottom-left pointed inward
                ctx.beginPath(); ctx.moveTo(m + len, h - m); ctx.lineTo(m + len, h - m - len); ctx.lineTo(m, h - m - len); ctx.stroke();
                // Bottom-right pointed inward
                ctx.beginPath(); ctx.moveTo(w - m - len, h - m); ctx.lineTo(w - m - len, h - m - len); ctx.lineTo(w - m, h - m - len); ctx.stroke();
            }
        }
    }

    onColorChanged: if (isVector) canvas.requestPaint()
    onNameChanged: if (isVector) canvas.requestPaint()
    onWidthChanged: if (isVector) canvas.requestPaint()
    onHeightChanged: if (isVector) canvas.requestPaint()

    Text {
        anchors.centerIn: parent
        visible: !root.isVector
        text: root.name
        font.family: Typography.fontFamily
        font.pixelSize: root.size
        color: root.color
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
