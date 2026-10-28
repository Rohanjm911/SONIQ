import QtQuick
import "../themes"

// Full-screen splash overlay that plays a GIF for 6 seconds,
// then smoothly fades out to reveal the main application.
Item {
    id: splashRoot

    anchors.fill: parent
    z: 99999
    visible: opacity > 0.005

    property bool splashFinished: false
    property int splashDurationMs: 6000

    // Dark cinematic background absorbing all mouse input
    Rectangle {
        anchors.fill: parent
        color: "#0A0B0E"

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.AllButtons
            onClicked: {
                if (!splashRoot.splashFinished) {
                    splashRoot.dismiss();
                }
            }
        }
    }

    // Centered Content
    Item {
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.6, parent.height * 0.6, 500)
        height: width

        // Subtle accent pulsing ring
        Rectangle {
            id: pulseRing
            anchors.centerIn: parent
            width: parent.width * 0.85
            height: parent.height * 0.85
            radius: width / 2
            color: "transparent"
            border.color: Colors.accent
            border.width: 2
            opacity: 0.15

            SequentialAnimation {
                running: !splashRoot.splashFinished
                loops: Animation.Infinite
                NumberAnimation {
                    target: pulseRing
                    property: "scale"
                    from: 0.95
                    to: 1.06
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    target: pulseRing
                    property: "scale"
                    from: 1.06
                    to: 0.95
                    duration: 2000
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // Animated GIF
        AnimatedImage {
            id: splashGif
            anchors.centerIn: parent
            width: parent.width * 0.72
            height: width
            source: "qrc:/ui/assets/splash.gif"
            fillMode: Image.PreserveAspectFit
            playing: true
            smooth: true
            cache: true
        }

        // Branding
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: splashGif.bottom
            anchors.topMargin: 20
            spacing: 6

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "SONIQ"
                font.family: Typography.fontFamily
                font.pixelSize: 26
                font.weight: Typography.weightBold
                font.letterSpacing: 4.0
                color: "#FFFFFF"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "PURE SOUND. INFINITE POSSIBILITIES."
                font.family: Typography.monospaceFamily
                font.pixelSize: 10
                font.weight: Typography.weightBold
                font.letterSpacing: 2.0
                color: Colors.accent
            }
        }
    }

    // Smooth fade-out behavior
    opacity: 1.0
    Behavior on opacity {
        NumberAnimation {
            duration: 800
            easing.type: Easing.InOutCubic
        }
    }

    function dismiss() {
        splashFinished = true;
        splashRoot.opacity = 0.0;
    }

    // 6-second timer
    Timer {
        id: splashTimer
        interval: splashRoot.splashDurationMs
        running: true
        repeat: false
        onTriggered: {
            splashRoot.dismiss();
        }
    }
}
