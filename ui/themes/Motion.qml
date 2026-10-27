pragma Singleton
import QtQuick

QtObject {
    id: root

    // Duration Tokens
    readonly property int durationInstant: 100
    readonly property int durationFast: 150
    readonly property int durationNormal: 250
    readonly property int durationSlow: 400
    readonly property int durationPageTransition: 300

    // Easing Curves
    readonly property int easingStandard: Easing.OutCubic
    readonly property int easingEntrance: Easing.OutQuart
    readonly property int easingExit: Easing.InQuart
}
