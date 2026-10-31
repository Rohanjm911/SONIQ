import QtQuick
import "../themes"

Rectangle {
    id: root

    property string message: ""
    property string iconName: "check"
    property int durationMs: 2500

    function show(msg, icon) {
        root.message = msg;
        root.iconName = icon || "check";
        timer.restart();
        opacity = 1.0;
        y = -10;
    }

    width: row.implicitWidth + 32
    height: 42
    radius: Theme.cornerRadiusPill
    color: Colors.surfaceElevated
    border.color: Colors.borderMedium
    border.width: 1

    opacity: 0.0
    visible: opacity > 0.0

    Behavior on opacity {
        NumberAnimation { duration: Motion.durationNormal; easing.type: Motion.easingStandard }
    }
    Behavior on y {
        NumberAnimation { duration: Motion.durationNormal; easing.type: Motion.easingStandard }
    }

    Timer {
        id: timer
        interval: root.durationMs
        onTriggered: {
            root.opacity = 0.0;
            root.y = 10;
        }
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 10

        SoniqIcon {
            name: root.iconName
            size: 16
            color: Colors.accent
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root.message
            font.family: Typography.fontFamily
            font.pixelSize: Typography.sizeBody
            font.weight: Typography.weightMedium
            color: Colors.textPrimary
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}

