import QtQuick 2.6
import "../view"

Item {
    id: base

    readonly property int lastPageNo: 2 // 3 pages

    property int pageNo: 0
    property int frequency: 0

    function update() {
        presetBgP1.update(pageNo * 6    , frequency)
        presetBgP2.update(pageNo * 6 + 1, frequency)
        presetBgP3.update(pageNo * 6 + 2, frequency)
        presetBgP4.update(pageNo * 6 + 3, frequency)
        presetBgP5.update(pageNo * 6 + 4, frequency)
        presetBgP6.update(pageNo * 6 + 5, frequency)

        console.log("Update " + pageNo + ", " + frequency)
    }

    onFrequencyChanged: {
        console.log("Frequency changed: " + frequency)
        update()
    }

    Component.onCompleted: {
        update()
    }


    // Page icon
    Image {
        id: pageIcon
        anchors.horizontalCenter: parent.horizontalCenter
        source: "../image/pagenation_0" + (pageNo + 1) + ".png"
    }

    // Prev button
    ImageButton {
        id: presetPrevButton
        anchors.top: pageIcon.bottom
        anchors.topMargin: 10
        source: pressed ? "../image/btn_page_press.png" : "../image/btn_page_default.png"
        onClicked: {
            console.log("<")
            if (pageNo > 0) {
                pageNo -= 1
            } else {
                pageNo = lastPageNo
            }
            base.update()
        }

        Image {
            anchors.centerIn: parent
            source: presetPrevButton.pressed ? "../image/left_page_on.png" : "../image/left_page_off.png"
        }
    }

    // Next button
    ImageButton {
        id: presetNextButton
        x: 964
        anchors.top: pageIcon.bottom
        anchors.topMargin: 10
        source: pressed ? "../image/btn_page_press.png" : "../image/btn_page_default.png"
        onClicked: {
            console.log(">")
            if (pageNo < lastPageNo) {
                pageNo += 1
            } else {
                pageNo = 0
            }
            base.update()
        }

        Image {
            anchors.centerIn: parent
            source: presetNextButton.pressed ? "../image/right_page_on.png" : "../image/right_page_off.png"
        }
    }

    // Presets
    Column {
        x: 61
        anchors.top: pageIcon.bottom
        anchors.topMargin: 10
        width: parent.width - presetPrevButton.width - presetNextButton.width
        spacing: 1

        Row {
            width: parent.width
            spacing: 1

            PresetButton {
                id: presetBgP1
            }
            PresetButton {
                id: presetBgP2
            }
            PresetButton {
                id: presetBgP3
            }
        }

        Row {
            width: parent.width
            spacing: 1

            PresetButton {
                id: presetBgP4
            }
            PresetButton {
                id: presetBgP5
            }
            PresetButton {
                id: presetBgP6
            }
        }
    }
}
