pragma Singleton
import QtQuick

QtObject {
    id: root

    // Primary Font Family
    readonly property string fontFamily: "Segoe UI, -apple-system, Roboto, Helvetica Neue, sans-serif"
    readonly property string monospaceFamily: "Cascadia Code, Consolas, Monaco, monospace"

    // Typographic Scale
    readonly property int sizeDisplay: 32
    readonly property int sizeTitleLarge: 24
    readonly property int sizeTitleMedium: 18
    readonly property int sizeTitleSmall: 15
    readonly property int sizeBody: 13
    readonly property int sizeCaption: 11
    readonly property int sizeMicro: 9

    // Font Weights
    readonly property int weightLight: Font.Light
    readonly property int weightNormal: Font.Normal
    readonly property int weightMedium: Font.DemiBold
    readonly property int weightBold: Font.Bold
}
