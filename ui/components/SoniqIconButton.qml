import QtQuick
import QtQuick.Controls.Basic
import "../themes"

Button {
    id: control

    property string iconSymbol: "▶"
    property int iconPixelSize: 16
    property bool isAccent: false
    property bool isPill: false
    property color iconColor: isAccent ? Colors.accent : Colors.textPrimary
    property string tooltipText: ""

    implicitWidth: 36
    implicitHeight: 36

    hoverEnabled: true

    background: Rectangle {
        radius: control.isPill ? Theme.cornerRadiusPill : Theme.cornerRadiusSmall
        color: {
            if (control.down) return Colors.surfaceActive;
            if (control.hovered) return Colors.surfaceHover;
            return "transparent";
        }

        Behavior on color {
            ColorAnimation { duration: Motion.durationFast; easing.type: Motion.easingStandard }
        }
    }

    contentItem: Text {
        text: control.iconSymbol
        font.family: Typography.fontFamily
        font.pixelSize: control.iconPixelSize
        color: control.enabled ? control.iconColor : Colors.textMuted
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    ToolTip.visible: control.hovered && control.tooltipText.length > 0
    ToolTip.text: control.tooltipText
    ToolTip.delay: 500
}
