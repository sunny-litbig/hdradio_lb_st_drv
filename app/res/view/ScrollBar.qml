import QtQuick 2.6

Image {
    id: base
    source: "../image/scroll_bg.png"
    visible: handleBar.height < maxHeight

    property ListView listView: null

    property int minHeight: 12
    property int maxHeight: parent.height - 108

    property int minPos: 24
    property int maxPos: parent.height - 24 - handleBar.height

    property int contentsBottomPosition: 0
    property int moveRange: 0

    Image {
        id: handleTop
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: handleBar.top
        source: "../image/scroll_top.png"
    }
    Image {
        id: handleBar
        y: minPos
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../image/scroll_bar.png"
    }
    Image {
        id: handleBottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: handleBar.bottom
        source: "../image/scroll_bottom.png"
    }

    Item {
        id: dummy
        width: 1
        height: 1

        onYChanged: {
            var rate = (y - minPos) / (maxPos - minPos)
            if (rate < 0) {
                rate = 0
            } else if (rate > 1) {
                rate = 1
            }

            var pos = contentsBottomPosition * rate
            listView.contentY = pos
        }
    }

    MouseArea {
        id: dragArea
        anchors.fill: parent

        drag.target: dummy
        drag.minimumX: 0
        drag.maximumX: 0
        drag.minimumY: minPos
        drag.maximumY: maxPos

        onPressed: {
            var rate = listView.contentY / contentsBottomPosition
            var position = rate * (maxPos - minPos) + minPos
            dummy.y = position
        }
    }

    Connections {
        target: listView

        onContentHeightChanged: {
            if (listView.contentHeight == 0) {
                handleBar.height = maxHeight
                return
            }

            var rate = listView.height / listView.contentHeight
            var range = maxHeight - minHeight
            var height = rate * range

            if (height < minHeight) {
                height = minHeight
            } else if (height > maxHeight) {
                height = maxHeight
            }

            console.log("rate: " + rate)
            console.log("range: " + range)
            console.log("height: " + height + " (" + minHeight + ", " + maxHeight + ")")

            contentsBottomPosition = listView.contentHeight - listView.height
            moveRange = maxHeight - height
            handleBar.height = height

            listView.y = listView.y
        }

        onContentYChanged: {
            if (contentsBottomPosition == 0) {
                return
            }

            var rate = target.contentY / contentsBottomPosition
            if (rate < 0) {
                rate = 0
            } else if (rate > 1) {
                rate = 1
            }

            var position = rate * moveRange + minPos
            handleBar.y = position
        }
    }
}
