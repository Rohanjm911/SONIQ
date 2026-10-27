import QtQuick
import "../themes"

Rectangle {
    id: root

    property bool interactive: false
    property alias hovered: mouseArea.containsMouse
    signal clicked()

    radius: Theme.cornerRadiusMedium
    color: hovered && interactive ? Colors.surfaceHover : Colors.surface
    border.color: Colors.borderSubtle
    border.width: 1

    Behavior on color {
        ColorAnimation { duration: Motion.durationFast; easing.type: Motion.easingStandard }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.interactive
        hoverEnabled: root.interactive
        cursorShape: root.interactive ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.clicked()
    }
}
