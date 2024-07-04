import QtQuick 2.6
import "../view"

Item {
    id: base

    property alias presetNo: presetMarkText.text
    property alias frequency: freqText.text

    // Frequency
    Text {
        id: freqText
        x: 60
        y: 100

        color: "white"
        font.family: win.boldFont
        font.pixelSize: 110
    }
    Image {
        id: freqUnit
        x: freqText.x + freqText.width + 24
        anchors.bottom: freqText.baseline
        source: "../image/khz.png"
    }

    Text {
        id: presetMarkText
        x: 60
        anchors.bottom: freqText.top

        color: "#ff004e"
        font.family: win.boldFont
        font.pixelSize: 18
    }
}
