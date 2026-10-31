pragma Singleton
import QtQuick

QtObject {
    id: root

    // Layout Dimensions
    readonly property int sidebarWidth: 230
    readonly property int miniPlayerHeight: 76
    readonly property int headerHeight: 56
    readonly property int trackRowHeight: 52

    // Spacing Tokens
    readonly property int spacingXXS: 4
    readonly property int spacingXS: 8
    readonly property int spacingS: 12
    readonly property int spacingM: 16
    readonly property int spacingL: 20
    readonly property int spacingXL: 24
    readonly property int spacingXXL: 32

    // Corner Radius Tokens
    readonly property int cornerRadiusSmall: 6
    readonly property int cornerRadiusMedium: 10
    readonly property int cornerRadiusLarge: 14
    readonly property int cornerRadiusCard: 16
    readonly property int cornerRadiusPill: 999

    // Control Heights
    readonly property int buttonHeight: 36
    readonly property int inputHeight: 36

    // Icon Size Tokens
    readonly property int iconSizeSmall: 14
    readonly property int iconSizeNormal: 16
    readonly property int iconSizeLarge: 22
}
