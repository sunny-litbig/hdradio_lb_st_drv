import QtQuick 2.6

Item {
    property alias duration: bgAnimation.duration
    property alias interval: bgChangeTimer.interval

    property int bgIndex: 1

    function changeBackgroud() {
        bgIndex++
        if (bgIndex > 5) {
            bgIndex = 1
        }

        var imageName = "../image/img_0" + bgIndex + ".jpg"
        bgImageChanger.source = imageName
        bgAnimation.running = true
    }

    // Backgroud

    Timer {
        id: bgChangeTimer
        interval: 10000
        running: true
        repeat: false

        onTriggered: {
            changeBackgroud()
        }
    }

    Image {
        id: bgImage
        anchors.fill: parent
        source: "../image/img_01.jpg"
    }

    Image {
        id: bgImageChanger
        anchors.fill: parent
        opacity: 0

        OpacityAnimator {
            id: bgAnimation
            target: bgImageChanger
            running: false
            from: 0
            to: 1
            duration: 3000

            onStopped: {
                bgImage.source = bgImageChanger.source
                bgImageChanger.opacity = 0
                bgChangeTimer.restart()
            }
        }
    }
}
