import QtQuick 2.6

ImageButton {
    property alias freqText: frequencyeText.text
    property alias stationName: stationText.text

    property int frequency: 0

    width: parent.width
    height: 71 // 70 + 1 for split line

    source: pressed ? "../image/list_drop_sub_press.png" : ""

    AnimatedImage {
        id: playIcon
        x: 30
        anchors.verticalCenter: parent.verticalCenter
        source: "../image/play_gif.gif"
        visible: Native.frequency === frequency
    }

    Text {
        id: frequencyeText
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: playIcon.right
        anchors.leftMargin: 10

        color: "white"
        font.family: win.boldFont
        font.pixelSize: 20
    }
    Text {
        id: stationText
        x: 280
        anchors.verticalCenter: parent.verticalCenter

        color: "white"
        font.family: win.boldFont
        font.pixelSize: 20
    }

    Image {
        id: splitLine
        anchors.bottom: parent.bottom
        source: "../image/list_line.png"
    }
}
