import QtQuick
import QtQuick.Controls.Basic
import "../themes"

Button {
    id: control

    property string iconSymbol: "play"
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
            if (!control.enabled) return "transparent";
            if (control.down) return Colors.surfaceActive;
            if (control.hovered) return Colors.surfaceHover;
            return "transparent";
        }

        border.color: control.activeFocus ? Colors.borderFocus : "transparent"
        border.width: 1

        Behavior on color {
            ColorAnimation { duration: Motion.durationFast; easing.type: Motion.easingStandard }
        }
    }

    contentItem: SoniqIcon {
        anchors.centerIn: parent
        name: control.iconSymbol
        size: control.iconPixelSize
        color: control.enabled ? control.iconColor : Colors.textMuted
    }

    ToolTip.visible: control.hovered && control.tooltipText.length > 0
    ToolTip.text: control.tooltipText
    ToolTip.delay: 400
    ToolTip.toolTip.contentItem: Text {
        text: control.tooltipText
        font.family: Typography.fontFamily
        font.pixelSize: Typography.sizeCaption
        color: Colors.textPrimary
    }
    ToolTip.toolTip.background: Rectangle {
        color: Colors.surfaceElevated
        border.color: Colors.borderMedium
        border.width: 1
        radius: Theme.cornerRadiusSmall
    }
}
