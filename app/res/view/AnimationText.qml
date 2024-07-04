import QtQuick 2.0

Item {
    id:marqueeText
    height: scrollingText.height
    clip: true
    property int tempX: 0
    property alias text: scrollingText.text

    property alias color: scrollingText.color
    property alias fontFamily: scrollingText.font.family
    property alias fontPixelSize: scrollingText.font.pixelSize
    property alias elide: scrollingText.elide

    //property alias timerRunning: timer.running

    Text {
        id:scrollingText
        x: tempX
    }

//    MouseArea {
//        id:mouseArea
//        anchors.fill: parent
//        onClicked: {
//            tempX = 0
//            timer.running = true
//        }
//    }

    Timer {
        id: timer
        interval: 100; running: true; repeat: true
        onTriggered:{
            tempX = tempX + 5
            scrollingText.x = -tempX

            if( tempX + marqueeText.width > scrollingText.width ) {
                timer.running = false
                pauseTimer.running = true
                scrollingText.x = 0
            }
        }
    }

    Timer {
        id: pauseTimer
        interval: 2000; running: false; repeat: false
        onTriggered: {
            //scrollingText.x = 0
            tempX = 0
            timer.running = true
        }
    }
}
