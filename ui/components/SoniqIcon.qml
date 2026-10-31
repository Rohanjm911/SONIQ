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
               n === "play" || name === "\u25B6" ||
               n === "pause" || name === "\u23F8" ||
               n === "equalizer" || n === "eq" || name === "🎚" ||
               n === "inspector" || n === "search" ||
               n === "volume" || n === "vol" ||
               n === "mute" ||
               n === "shuffle" ||
               n === "repeat" || n === "repeat_one" ||
               n === "fullscreen" || n === "exit_fullscreen" ||
               n === "home" || name === "⌂" ||
               n === "songs" || n === "music" || name === "♫" ||
               n === "albums" || n === "album" ||
               n === "artists" || n === "artist" ||
               n === "genres" || n === "genre" ||
               n === "playlists" || n === "playlist" ||
               n === "favorites" || n === "heart" || n === "heart_filled" ||
               n === "recent" || n === "history" ||
               n === "queue" || name === "☰" ||
               n === "lyrics" ||
               n === "visualizer" ||
               n === "settings" || name === "⚙" ||
               n === "add" || n === "plus" || name === "＋" || name === "+" ||
               n === "close" || n === "x" || name === "✕" ||
               n === "check" ||
               n === "warning" ||
               n === "info" ||
               n === "trash" || n === "delete" ||
               n === "edit" ||
               n === "folder";
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
            } else if (n === "play" || root.name === "\u25B6") {
                ctx.fillStyle = root.color;
                ctx.beginPath();
                ctx.moveTo(w * 0.22, h * 0.16);
                ctx.lineTo(w * 0.84, h * 0.50);
                ctx.lineTo(w * 0.22, h * 0.84);
                ctx.closePath();
                ctx.fill();
            } else if (n === "pause" || root.name === "\u23F8") {
                ctx.fillStyle = root.color;
                var barW = w * 0.22;
                ctx.fillRect(w * 0.20, h * 0.18, barW, h * 0.64);
                ctx.fillRect(w * 0.58, h * 0.18, barW, h * 0.64);
            } else if (n === "home" || root.name === "⌂") {
                ctx.strokeStyle = root.color;
                ctx.fillStyle = root.color;
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.moveTo(w * 0.50, h * 0.16);
                ctx.lineTo(w * 0.86, h * 0.48);
                ctx.lineTo(w * 0.74, h * 0.48);
                ctx.lineTo(w * 0.74, h * 0.84);
                ctx.lineTo(w * 0.26, h * 0.84);
                ctx.lineTo(w * 0.26, h * 0.48);
                ctx.lineTo(w * 0.14, h * 0.48);
                ctx.closePath();
                ctx.stroke();
                // Door
                ctx.fillRect(w * 0.42, h * 0.58, w * 0.16, h * 0.26);
            } else if (n === "songs" || n === "music" || root.name === "♫") {
                ctx.fillStyle = root.color;
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.6;
                // Double note
                ctx.beginPath();
                ctx.arc(w * 0.30, h * 0.72, w * 0.15, 0, 2 * Math.PI);
                ctx.arc(w * 0.74, h * 0.62, w * 0.15, 0, 2 * Math.PI);
                ctx.fill();
                ctx.beginPath();
                ctx.moveTo(w * 0.42, h * 0.70);
                ctx.lineTo(w * 0.42, h * 0.24);
                ctx.lineTo(w * 0.86, h * 0.14);
                ctx.lineTo(w * 0.86, h * 0.60);
                ctx.stroke();
                // Beam
                ctx.beginPath();
                ctx.moveTo(w * 0.42, h * 0.24);
                ctx.lineTo(w * 0.86, h * 0.14);
                ctx.lineWidth = 2.4;
                ctx.stroke();
            } else if (n === "albums" || n === "album") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                // Outer record circle
                ctx.beginPath();
                ctx.arc(w * 0.50, h * 0.50, w * 0.40, 0, 2 * Math.PI);
                ctx.stroke();
                // Inner grooves
                ctx.beginPath();
                ctx.arc(w * 0.50, h * 0.50, w * 0.24, 0, 2 * Math.PI);
                ctx.stroke();
                // Center hole
                ctx.fillStyle = root.color;
                ctx.beginPath();
                ctx.arc(w * 0.50, h * 0.50, w * 0.09, 0, 2 * Math.PI);
                ctx.fill();
            } else if (n === "artists" || n === "artist") {
                ctx.strokeStyle = root.color;
                ctx.fillStyle = root.color;
                ctx.lineWidth = 1.5;
                // Head
                ctx.beginPath();
                ctx.arc(w * 0.50, h * 0.32, w * 0.20, 0, 2 * Math.PI);
                ctx.stroke();
                // Shoulders / torso
                ctx.beginPath();
                ctx.arc(w * 0.50, h * 0.90, w * 0.38, Math.PI, 0);
                ctx.stroke();
            } else if (n === "genres" || n === "genre") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                // Tag shape
                ctx.beginPath();
                ctx.moveTo(w * 0.16, h * 0.50);
                ctx.lineTo(w * 0.50, h * 0.16);
                ctx.lineTo(w * 0.84, h * 0.50);
                ctx.lineTo(w * 0.50, h * 0.84);
                ctx.closePath();
                ctx.stroke();
                ctx.fillStyle = root.color;
                ctx.beginPath();
                ctx.arc(w * 0.50, h * 0.50, w * 0.10, 0, 2 * Math.PI);
                ctx.fill();
            } else if (n === "playlists" || n === "playlist" || n === "queue" || root.name === "☰") {
                ctx.fillStyle = root.color;
                var lh = Math.max(1.8, h * 0.12);
                ctx.fillRect(w * 0.16, h * 0.22, w * 0.68, lh);
                ctx.fillRect(w * 0.16, h * 0.44, w * 0.68, lh);
                ctx.fillRect(w * 0.16, h * 0.66, w * 0.46, lh);
            } else if (n === "favorites" || n === "heart" || n === "heart_filled") {
                ctx.fillStyle = root.color;
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.4;
                var cx = w * 0.5, cy = h * 0.38;
                ctx.beginPath();
                ctx.moveTo(cx, h * 0.82);
                ctx.bezierCurveTo(w * 0.12, h * 0.56, w * 0.08, h * 0.24, cx, h * 0.40);
                ctx.bezierCurveTo(w * 0.92, h * 0.24, w * 0.88, h * 0.56, cx, h * 0.82);
                if (n === "heart_filled" || n === "favorites") {
                    ctx.fill();
                } else {
                    ctx.stroke();
                }
            } else if (n === "recent" || n === "history") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.arc(w * 0.50, h * 0.50, w * 0.38, 0, 2 * Math.PI);
                ctx.stroke();
                ctx.beginPath();
                ctx.moveTo(w * 0.50, h * 0.24);
                ctx.lineTo(w * 0.50, h * 0.50);
                ctx.lineTo(w * 0.70, h * 0.50);
                ctx.stroke();
            } else if (n === "lyrics") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.rect(w * 0.20, h * 0.14, w * 0.60, h * 0.72);
                ctx.stroke();
                ctx.fillStyle = root.color;
                ctx.fillRect(w * 0.30, h * 0.30, w * 0.40, 1.8);
                ctx.fillRect(w * 0.30, h * 0.48, w * 0.32, 1.8);
                ctx.fillRect(w * 0.30, h * 0.66, w * 0.38, 1.8);
            } else if (n === "visualizer") {
                ctx.fillStyle = root.color;
                var bw = w * 0.14;
                ctx.fillRect(w * 0.14, h * 0.44, bw, h * 0.42);
                ctx.fillRect(w * 0.36, h * 0.22, bw, h * 0.64);
                ctx.fillRect(w * 0.58, h * 0.34, bw, h * 0.52);
                ctx.fillRect(w * 0.80, h * 0.56, bw, h * 0.30);
            } else if (n === "settings" || root.name === "⚙") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.6;
                ctx.beginPath();
                ctx.arc(w * 0.50, h * 0.50, w * 0.22, 0, 2 * Math.PI);
                ctx.stroke();
                // 6 spokes
                for (var i = 0; i < 6; i++) {
                    var ang = i * (Math.PI / 3);
                    ctx.beginPath();
                    ctx.moveTo(w * 0.5 + Math.cos(ang) * (w * 0.25), h * 0.5 + Math.sin(ang) * (h * 0.25));
                    ctx.lineTo(w * 0.5 + Math.cos(ang) * (w * 0.42), h * 0.5 + Math.sin(ang) * (h * 0.42));
                    ctx.stroke();
                }
            } else if (n === "add" || n === "plus" || root.name === "＋" || root.name === "+") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.8;
                ctx.beginPath();
                ctx.moveTo(w * 0.50, h * 0.18);
                ctx.lineTo(w * 0.50, h * 0.82);
                ctx.moveTo(w * 0.18, h * 0.50);
                ctx.lineTo(w * 0.82, h * 0.50);
                ctx.stroke();
            } else if (n === "close" || n === "x" || root.name === "✕") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.8;
                ctx.beginPath();
                ctx.moveTo(w * 0.22, h * 0.22);
                ctx.lineTo(w * 0.78, h * 0.78);
                ctx.moveTo(w * 0.78, h * 0.22);
                ctx.lineTo(w * 0.22, h * 0.78);
                ctx.stroke();
            } else if (n === "check") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 2.0;
                ctx.beginPath();
                ctx.moveTo(w * 0.18, h * 0.52);
                ctx.lineTo(w * 0.42, h * 0.76);
                ctx.lineTo(w * 0.82, h * 0.24);
                ctx.stroke();
            } else if (n === "warning") {
                ctx.strokeStyle = root.color;
                ctx.fillStyle = root.color;
                ctx.lineWidth = 1.6;
                ctx.beginPath();
                ctx.moveTo(w * 0.50, h * 0.16);
                ctx.lineTo(w * 0.88, h * 0.84);
                ctx.lineTo(w * 0.12, h * 0.84);
                ctx.closePath();
                ctx.stroke();
                ctx.fillRect(w * 0.47, h * 0.42, w * 0.06, h * 0.22);
                ctx.fillRect(w * 0.47, h * 0.70, w * 0.06, w * 0.06);
            } else if (n === "info") {
                ctx.strokeStyle = root.color;
                ctx.fillStyle = root.color;
                ctx.lineWidth = 1.6;
                ctx.beginPath();
                ctx.arc(w * 0.50, h * 0.50, w * 0.38, 0, 2 * Math.PI);
                ctx.stroke();
                ctx.fillRect(w * 0.46, h * 0.28, w * 0.08, w * 0.08);
                ctx.fillRect(w * 0.46, h * 0.44, w * 0.08, h * 0.26);
            } else if (n === "trash" || n === "delete") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.moveTo(w * 0.20, h * 0.28);
                ctx.lineTo(w * 0.80, h * 0.28);
                ctx.stroke();
                ctx.beginPath();
                ctx.rect(w * 0.28, h * 0.32, w * 0.44, h * 0.52);
                ctx.stroke();
                ctx.beginPath();
                ctx.moveTo(w * 0.38, h * 0.24);
                ctx.lineTo(w * 0.62, h * 0.24);
                ctx.stroke();
            } else if (n === "edit") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.moveTo(w * 0.20, h * 0.80);
                ctx.lineTo(w * 0.36, h * 0.78);
                ctx.lineTo(w * 0.78, h * 0.36);
                ctx.lineTo(w * 0.64, h * 0.22);
                ctx.lineTo(w * 0.22, h * 0.64);
                ctx.closePath();
                ctx.stroke();
            } else if (n === "folder") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                ctx.beginPath();
                ctx.moveTo(w * 0.14, h * 0.32);
                ctx.lineTo(w * 0.38, h * 0.32);
                ctx.lineTo(w * 0.46, h * 0.42);
                ctx.lineTo(w * 0.86, h * 0.42);
                ctx.lineTo(w * 0.86, h * 0.78);
                ctx.lineTo(w * 0.14, h * 0.78);
                ctx.closePath();
                ctx.stroke();
            } else if (n === "equalizer" || n === "eq" || root.name === "🎚") {
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
            } else if (n === "inspector" || n === "search") {
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
            } else if (n === "volume" || n === "vol") {
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
            } else if (n === "mute") {
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
            } else if (n === "shuffle") {
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
                ctx.moveTo(w * 0.74, h * 0.56);
                ctx.lineTo(w * 0.86, h * 0.68);
                ctx.lineTo(w * 0.74, h * 0.80);
                ctx.stroke();

                ctx.beginPath();
                ctx.moveTo(w * 0.12, h * 0.68);
                ctx.lineTo(w * 0.38, h * 0.68);
                ctx.lineTo(w * 0.62, h * 0.32);
                ctx.lineTo(w * 0.84, h * 0.32);
                ctx.stroke();

                ctx.beginPath();
                ctx.moveTo(w * 0.74, h * 0.20);
                ctx.lineTo(w * 0.86, h * 0.32);
                ctx.lineTo(w * 0.74, h * 0.44);
                ctx.stroke();
            } else if (n === "repeat" || n === "repeat_one") {
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

                if (n === "repeat_one") {
                    ctx.font = "bold 9px monospace";
                    ctx.textAlign = "center";
                    ctx.textBaseline = "middle";
                    ctx.fillText("1", w * 0.5, h * 0.52);
                }
            } else if (n === "fullscreen") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                var m = 2, len = 4;
                ctx.beginPath(); ctx.moveTo(m, m + len); ctx.lineTo(m, m); ctx.lineTo(m + len, m); ctx.stroke();
                ctx.beginPath(); ctx.moveTo(w - m - len, m); ctx.lineTo(w - m, m); ctx.lineTo(w - m, m + len); ctx.stroke();
                ctx.beginPath(); ctx.moveTo(m, h - m - len); ctx.lineTo(m, h - m); ctx.lineTo(m + len, h - m); ctx.stroke();
                ctx.beginPath(); ctx.moveTo(w - m - len, h - m); ctx.lineTo(w - m, h - m); ctx.lineTo(w - m, h - m - len); ctx.stroke();
            } else if (n === "exit_fullscreen") {
                ctx.strokeStyle = root.color;
                ctx.lineWidth = 1.5;
                var m = 2, len = 4;
                ctx.beginPath(); ctx.moveTo(m + len, m); ctx.lineTo(m + len, m + len); ctx.lineTo(m, m + len); ctx.stroke();
                ctx.beginPath(); ctx.moveTo(w - m - len, m); ctx.lineTo(w - m - len, m + len); ctx.lineTo(w - m, m + len); ctx.stroke();
                ctx.beginPath(); ctx.moveTo(m + len, h - m); ctx.lineTo(m + len, h - m - len); ctx.lineTo(m, h - m - len); ctx.stroke();
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
