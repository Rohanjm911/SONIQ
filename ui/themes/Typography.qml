pragma Singleton
import QtQuick

QtObject {
    id: root

    // Primary Font Family Stack (Native System Fonts)
    readonly property string fontFamily: "Segoe UI, -apple-system, SF Pro Display, Inter, Roboto, Helvetica Neue, sans-serif"
    readonly property string monospaceFamily: "Cascadia Code, Consolas, SF Mono, Menlo, monospace"

    // Typographic Scale
    readonly property int sizeDisplay: 32
    readonly property int sizeTitleLarge: 22
    readonly property int sizeTitleMedium: 17
    readonly property int sizeTitleSmall: 15
    readonly property int sizeBody: 13
    readonly property int sizeCaption: 11
    readonly property int sizeMicro: 9

    // Font Weights
    readonly property int weightLight: Font.Light
    readonly property int weightNormal: Font.Normal
    readonly property int weightRegular: Font.Normal
    readonly property int weightMedium: Font.DemiBold
    readonly property int weightBold: Font.Bold

    // Letter Spacing Tokens
    readonly property real letterSpacingTight: -0.2
    readonly property real letterSpacingNormal: 0.0
    readonly property real letterSpacingCaps: 0.8
    readonly property real letterSpacingWide: 1.4
}
