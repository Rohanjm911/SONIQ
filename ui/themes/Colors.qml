pragma Singleton
import QtQuick

QtObject {
    id: root

    // Theme Mode: "Dark" or "Light"
    property bool isDark: true

    // Neutral Foundation Colors
    readonly property color background: isDark ? "#0A0B0E" : "#F8F9FA"
    readonly property color surface: isDark ? "#12141A" : "#FFFFFF"
    readonly property color surfaceElevated: isDark ? "#1A1D26" : "#F1F3F5"
    readonly property color surfaceHover: isDark ? "#222634" : "#E9ECEF"
    readonly property color surfaceActive: isDark ? "#2C3142" : "#DEE2E6"

    // Borders & Dividers
    readonly property color borderSubtle: isDark ? "#1F2330" : "#E2E5E9"
    readonly property color borderMedium: isDark ? "#2E3447" : "#CED4DA"
    readonly property color borderFocus: accent

    // Typography
    readonly property color textPrimary: isDark ? "#FFFFFF" : "#1A1D20"
    readonly property color textSecondary: isDark ? "#A0A6B8" : "#596172"
    readonly property color textMuted: isDark ? "#5D6478" : "#868E96"
    readonly property color textInverse: isDark ? "#0A0B0E" : "#FFFFFF"

    // Accents
    property color accent: "#FA2D48" // Apple Music Vibrant Crimson / Coral
    readonly property color accentHover: Qt.lighter(accent, 1.15)
    readonly property color accentPressed: Qt.darker(accent, 1.2)
    readonly property color accentMuted: Qt.rgba(accent.r, accent.g, accent.b, 0.15)

    // Semantic States
    readonly property color success: "#10B981"
    readonly property color successMuted: isDark ? "#064E3B" : "#D1FAE5"
    readonly property color warning: "#F59E0B"
    readonly property color warningMuted: isDark ? "#78350F" : "#FEF3C7"
    readonly property color error: "#EF4444"
    readonly property color errorMuted: isDark ? "#7F1D1D" : "#FEE2E2"
    readonly property color bitPerfectVerified: "#00F0FF" // Neon cyan for verified bit-perfect
    readonly property color hiResGold: "#F59E0B"
}
