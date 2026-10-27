import QtQuick
import QtQuick.Controls.Basic
import "../themes"

Button {
    id: control

    property string variant: "primary" // "primary", "secondary", "ghost", "danger"
    property string iconText: ""
    property int customRadius: Theme.cornerRadiusSmall

    implicitHeight: 36
    implicitWidth: Math.max(90, contentRow.implicitWidth + 24)

    hoverEnabled: true

    background: Rectangle {
        radius: control.customRadius
        color: {
            if (!control.enabled) return Qt.rgba(0.5, 0.5, 0.5, 0.2);
            if (control.variant === "primary") {
                if (control.down) return Colors.accentPressed;
                if (control.hovered) return Colors.accentHover;
                return Colors.accent;
            } else if (control.variant === "secondary") {
                if (control.down) return Colors.surfaceActive;
                if (control.hovered) return Colors.surfaceHover;
                return Colors.surfaceElevated;
            } else if (control.variant === "ghost") {
                if (control.down) return Colors.surfaceHover;
                if (control.hovered) return Qt.rgba(Colors.textPrimary.r, Colors.textPrimary.g, Colors.textPrimary.b, 0.08);
                return "transparent";
            } else if (control.variant === "danger") {
                if (control.down) return Qt.darker(Colors.error, 1.2);
                if (control.hovered) return Qt.lighter(Colors.error, 1.15);
                return Colors.error;
            }
            return Colors.surfaceElevated;
        }

        border.color: {
            if (control.activeFocus) return Colors.borderFocus;
            if (control.variant === "secondary") return Colors.borderSubtle;
            return "transparent";
        }
        border.width: 1

        Behavior on color {
            ColorAnimation { duration: Motion.durationFast; easing.type: Motion.easingStandard }
        }
    }

    contentItem: Row {
        id: contentRow
        spacing: 8
        anchors.centerIn: parent

        Text {
            visible: control.iconText.length > 0
            text: control.iconText
            font.family: Typography.fontFamily
            font.pixelSize: Typography.sizeTitleMedium
            color: control.variant === "primary" ? Colors.textInverse : Colors.textPrimary
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: control.text
            font.family: Typography.fontFamily
            font.pixelSize: Typography.sizeBody
            font.weight: Typography.weightMedium
            color: control.variant === "primary" ? Colors.textInverse : Colors.textPrimary
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
