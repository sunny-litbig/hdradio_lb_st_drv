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
        height: 240

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        color: "white"

        Text {
            width: parent.width
            y: 40

            color: "black"
            horizontalAlignment: Text.Center
            font.family: win.boldFont
            font.pixelSize: 32
            text: "Middleware Version"
        }

        Text {
            width: parent.width
            y: 100

            color: "#787878"
            horizontalAlignment: Text.Center
            font.family: win.boldFont
            font.pixelSize: 20
            text: "AM/FM/HD Radio V181220a Rel"
        }

        ImageButton {
            id: backButton
            y: 160
            anchors.horizontalCenter: parent.horizontalCenter
            source: pressed ? "../image/popup_back_press.png" : "../image/popup_back_default.png"
            onClicked: {
                win.hidePopover()
            }
        }
    }
}
