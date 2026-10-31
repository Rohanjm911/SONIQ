pragma Singleton
import QtQuick

QtObject {
    id: root

    // Accessibility / Performance Flag
    property bool reducedMotion: false

    // Duration Tokens (Smooth 60fps animations)
    readonly property int durationInstant: reducedMotion ? 0 : 80
    readonly property int durationFast: reducedMotion ? 0 : 150
    readonly property int durationNormal: reducedMotion ? 0 : 250
    readonly property int durationSlow: reducedMotion ? 0 : 350
    readonly property int durationPageTransition: reducedMotion ? 0 : 280

    // Easing Curves
    readonly property int easingStandard: Easing.OutCubic
    readonly property int easingEntrance: Easing.OutQuart
    readonly property int easingExit: Easing.InQuart
}
