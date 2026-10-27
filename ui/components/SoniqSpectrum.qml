import QtQuick
import "../themes"

Item {
    id: root

    property float peakL: 0.0
    property float peakR: 0.0
    property bool active: false
    property color barColor: Colors.accent

    implicitWidth: 120
    implicitHeight: 32

    Row {
        anchors.fill: parent
        spacing: 3

        Repeater {
            model: 16

            Rectangle {
                width: (parent.width - (15 * 3)) / 16
                anchors.bottom: parent.bottom

                // Calculate animated height from real peak levels with frequency-tilt weighting
                property real targetHeight: {
                    if (!root.active) return 2;
                    var base = (index % 2 === 0) ? root.peakL : root.peakR;
                    var factor = 0.3 + 0.7 * Math.sin((index / 16.0) * Math.PI);
                    return Math.max(2, Math.min(parent.height, base * factor * parent.height * 1.5));
                }

                height: targetHeight
                radius: 2
                color: root.barColor

                Behavior on height {
                    NumberAnimation { duration: 80; easing.type: Easing.OutQuad }
                }
            }
        }
    }
}
