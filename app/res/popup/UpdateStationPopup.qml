import QtQuick 2.6
import "../view"

Item {
    id: base
    opacity: 0

    property bool cancelable: false

    function show() {
        showAnim.running = true
    }

    function hide() {
        hideAnim.running = true
    }

    OpacityAnimator {
        id: showAnim
        target: base
        running: false
        duration: 200
        from: base.opacity
        to: 1
    }

    OpacityAnimator {
        id: hideAnim
        target: base
        running: false
        duration: 200
        from: base.opacity
        to: 0

        onStopped: {
            win.removePopover()
        }
    }

    // UI

    Rectangle {
        width: 520
        height: 320

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        color: "white"

        Text {
            id: titleText
            width: parent.width
            y: 30

            color: "black"
            horizontalAlignment: Text.Center
            font.family: win.boldFont
            font.pixelSize: 32
            text: "Update station"
        }
        AnimatedImage {
            id: progressIcon
            anchors.top: titleText.bottom
            anchors.topMargin: 25
            anchors.horizontalCenter: parent.horizontalCenter
            source: "../image/linking_gif.gif"
        }
        Text {
            id: searchText
            width: parent.width
            anchors.top: progressIcon.bottom
            anchors.topMargin: 13

            color: "#787878"
            horizontalAlignment: Text.Center
            font.family: win.boldFont
            font.pixelSize: 20
            text: "Searching..."
        }

        ImageButton {
            id: backButton
            anchors.top: searchText.bottom
            anchors.topMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter
            source: pressed ? "../image/popup_stop_press.png" : "../image/popup_stop_default.png"
            onClicked: {
                Native.stopScan()
                win.hidePopover()
            }
        }
    }
}
