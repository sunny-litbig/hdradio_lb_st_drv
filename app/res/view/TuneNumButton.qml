import QtQuick 2.6

ImageButton {
    id: base

    property alias text: numText.text

    source: {
        if (!enabled) {
            return "../image/tune_number_non.png"
        } else if (pressed) {
            return "../image/tune_number_press.png"
        } else {
            return "../image/tune_number_default.png"
        }
    }

    Text {
        id: numText
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        horizontalAlignment: Text.Center
        color: "white"
        font.family: win.regularFont
        font.pixelSize: 30
    }
}
