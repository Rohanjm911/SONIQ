import QtQuick
import QtQuick.Controls.Basic
import "../themes"

Slider {
    id: control

    property bool showHoverExpansion: true
    property color activeColor: Colors.accent

    implicitHeight: 24
    implicitWidth: 200

    hoverEnabled: true

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: 200
        implicitHeight: (control.hovered || control.pressed) ? 6 : 4
        width: control.availableWidth
        height: implicitHeight
        radius: height / 2
        color: Colors.surfaceActive

        Behavior on implicitHeight {
            NumberAnimation { duration: Motion.durationFast; easing.type: Motion.easingStandard }
        }

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            color: control.activeColor
            radius: parent.radius
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: (control.hovered || control.pressed) ? 14 : 0
        implicitHeight: implicitWidth
        radius: width / 2
        color: Colors.textPrimary
        border.color: Qt.darker(Colors.surface, 1.2)
        border.width: 1
        opacity: (control.hovered || control.pressed) ? 1.0 : 0.0

        Behavior on implicitWidth {
            NumberAnimation { duration: Motion.durationFast; easing.type: Motion.easingStandard }
        }
        Behavior on opacity {
            NumberAnimation { duration: Motion.durationFast; easing.type: Motion.easingStandard }
        }
    }
}
