pragma Singleton
import QtQuick

QtObject {
    id: root

    // Theme Mode: "Dark" or "Light"
    property bool isDark: true

    // Neutral Foundation Colors
    readonly property color background: isDark ? "#0A0B10" : "#F6F7F9"
    readonly property color backgroundSecondary: isDark ? "#10131B" : "#FFFFFF"
    readonly property color surface: isDark ? "#141722" : "#FFFFFF"
    readonly property color surfaceElevated: isDark ? "#1C2030" : "#F0F2F5"
    readonly property color surfaceCard: isDark ? "#181B28" : "#FFFFFF"
    readonly property color surfaceGrouped: isDark ? "#171A26" : "#F2F4F7"
    readonly property color surfaceHover: isDark ? "#252A3D" : "#E8ECF0"
    readonly property color surfaceActive: isDark ? "#2F354D" : "#DFE3E8"

    // Borders & Dividers
    readonly property color borderSubtle: isDark ? "#262B3A" : "#E5E8ED"
    readonly property color borderMedium: isDark ? "#363C50" : "#D1D6DE"
    readonly property color borderFocus: accent
    readonly property color divider: isDark ? "#222736" : "#EDF0F4"

    // Typography
    readonly property color textPrimary: isDark ? "#FFFFFF" : "#111317"
    readonly property color textSecondary: isDark ? "#A0A6B8" : "#585E6D"
    readonly property color textMuted: isDark ? "#6B7285" : "#8C93A3"
    readonly property color textTertiary: isDark ? "#484E60" : "#B5BAC4"
    readonly property color textInverse: isDark ? "#0A0B10" : "#FFFFFF"

    // Accent System (Default: Apple Music Vibrant Crimson #FA2D48)
    property color accent: "#FA2D48"
    readonly property color accentHover: Qt.lighter(accent, 1.15)
    readonly property color accentPressed: Qt.darker(accent, 1.2)
    readonly property color accentMuted: Qt.rgba(accent.r, accent.g, accent.b, 0.18)
    readonly property color accentSubtle: Qt.rgba(accent.r, accent.g, accent.b, 0.08)

    // Palette of Available Accents for Theme Selection
    readonly property var accentPalette: [
        { name: "Crimson", color: "#FA2D48" },
        { name: "Sapphire", color: "#0A84FF" },
        { name: "Emerald", color: "#30D158" },
        { name: "Amber", color: "#FF9F0A" },
        { name: "Purple", color: "#AF52DE" },
        { name: "Neon Cyan", color: "#00F0FF" }
    ]

    // Semantic States
    readonly property color success: "#10B981"
    readonly property color successMuted: isDark ? "#064E3B" : "#D1FAE5"
    readonly property color warning: "#F59E0B"
    readonly property color warningMuted: isDark ? "#451A03" : "#FEF3C7"
    readonly property color error: "#EF4444"
    readonly property color errorMuted: isDark ? "#450A0A" : "#FEE2E2"

    // Audiophile Badges
    readonly property color bitPerfectVerified: "#00F0FF" // Neon cyan
    readonly property color hiResGold: "#F59E0B"
    readonly property color dsdPurple: "#A855F7"
    readonly property color losslessGreen: "#10B981"
}
