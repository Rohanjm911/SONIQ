import QtQuick
import "../themes"

Rectangle {
    id: root

    property string badgeText: "LOSSLESS"
    property string variant: "standard" // "standard", "bitperfect", "hires", "warning"

    implicitHeight: 20
    implicitWidth: label.implicitWidth + 12

    radius: Theme.cornerRadiusSmall
    color: {
        if (variant === "bitperfect") return Qt.rgba(0.0, 0.94, 1.0, 0.15);
        if (variant === "hires") return Qt.rgba(0.96, 0.62, 0.04, 0.15);
        if (variant === "warning") return Colors.warningMuted;
        return Colors.surfaceActive;
    }

    border.color: {
        if (variant === "bitperfect") return Colors.bitPerfectVerified;
        if (variant === "hires") return Colors.hiResGold;
        if (variant === "warning") return Colors.warning;
        return Colors.borderSubtle;
    }
    border.width: 1

    Text {
        id: label
        anchors.centerIn: parent
        text: root.badgeText
        font.family: Typography.monospaceFamily
        font.pixelSize: Typography.sizeMicro
        font.weight: Typography.weightBold
        color: {
            if (variant === "bitperfect") return Colors.bitPerfectVerified;
            if (variant === "hires") return Colors.hiResGold;
            if (variant === "warning") return Colors.warning;
            return Colors.textSecondary;
        }
    }
}
