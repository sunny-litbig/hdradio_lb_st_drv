import QtQuick 2.6
import "../view"

Item {
    id: base

    function numpadClicked(text) {
        var ft = freqText.text

        if (text === "-") {
            if (ft.length > 0) {
                freqText.text = ft.substring(0, ft.length - 1)
            }
        } else if (text === ".") {
            if (ft.indexOf(".") < 0) {
                if (ft.length > 0) {
                    freqText.text = ft + "."
                } else {
                    freqText.text = "0."
                }
            }
        } else {
            if (ft === "0") {
                freqText.text = text
            } else {
                if(ft.length == 3){
                  if(ft.indexOf(".") == 2)  {
                      freqText.text = ft + text
                  }
                  else if(!win.fmRadio)
                  {
                      freqText.text = ft + text
                  }
                }
                else if(ft.length == 4){
                   if(ft.indexOf(".") == 3){
                     freqText.text = ft + text
                   }
                }
                else if(ft.length != 5 && ft.indexOf(".") != 3 && ft.indexOf(".") != 4){
                    freqText.text = ft + text
                }
            }
        }
        if(win.fmRadio)
        {
            invalidText_Fm.visible = Native.isInvalidFrequency(freqText.text)
        }
        else
        {
            invalidText_Am.visible = Native.isInvalidFrequency(freqText.text)
        }

        //invalidText.visible = Native.isInvalidFrequency(freqText.text)
        //console.log("so, is visible " + invalidText.visible)
    }

    Image {
        id: topBar
        source: "../image/list_bar.png"

        Text {
            x: 30
            anchors.verticalCenter: parent.verticalCenter

            color: "black"
            font.family: win.boldFont
            font.pixelSize: 24
            text: "Direct Tune"
        }
        ImageButton {
            id: backButton
            x: parent.width - width - 30
            anchors.verticalCenter: parent.verticalCenter
            source: pressed ? "../image/list_back_press.png" : "../image/list_back_default.png"
            onClicked: {
                win.moveToBack()
            }
        }
    }

    Image {
        id: tuneBg
        x: 20
        y: topBar.height + 20
        source: "../image/tune_back.png"

        // Frequency text
        Image {
            id: freqBox
            x: 60
            y: 36
            source: win.fmRadio ? "../image/tune_textarea_mhz.png" : "../image/tune_textarea_khz.png"
        }
        Text {
            id: freqText
            anchors.bottom: freqBox.bottom
            anchors.right: freqBox.right
            anchors.rightMargin: 74
            color: "white"
            font.family: win.regularFont
            font.pixelSize: 82
            text: win.fmRadio ? (Native.frequency * 0.001).toFixed(2) : Native.frequency
        }

        // Invalid text
        Text {
            id: invalidText_Fm
            x: 60
            y: freqBox.y + freqBox.height + 76
            visible: false

            color: "white"
            font.family: win.boldFont
            font.pixelSize: 36
            text: "FM range : 87.5-108MHz"
        }
        Text {
            id: invalidText_Am
            x: 60
            y: freqBox.y + freqBox.height + 76
            visible: false

            color: "white"
            font.family: win.boldFont
            font.pixelSize: 36
            text: "AM range : 530-1710KHz"
        }
        // Apply button
        ImageButton {
            id: goButton
            x: 60
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 22
            source: pressed ? "../image/tune_go_press.png" : "../image/tune_go_default.png"
            onClicked: {
                if(!(Native.isInvalidFrequency(freqText.text)))
                {
                    Native.setFrequency(freqText.text)
                    win.moveToBack()
                }
                //Native.setFrequency(freqText.text)
                //win.moveToBack()
            }
        }

        // Numpad
        Column {
            id: numCols
            anchors.right: parent.right
            anchors.rightMargin: 22
            y: 22
            spacing: 5

            Row {
                spacing: 5
                TuneNumButton {
                    text: "1"
                    onClicked: numpadClicked("1")
                }
                TuneNumButton {
                    text: "2"
                    onClicked: numpadClicked("2")
                }
                TuneNumButton {
                    text: "3"
                    onClicked: numpadClicked("3")
                }
            }

            Row {
                spacing: 5
                TuneNumButton {
                    text: "4"
                    onClicked: numpadClicked("4")
                }
                TuneNumButton {
                    text: "5"
                    onClicked: numpadClicked("5")
                }
                TuneNumButton {
                    text: "6"
                    onClicked: numpadClicked("6")
                }
            }

            Row {
                spacing: 5
                TuneNumButton {
                    text: "7"
                    onClicked: numpadClicked("7")
                }
                TuneNumButton {
                    text: "8"
                    onClicked: numpadClicked("8")
                }
                TuneNumButton {
                    text: "9"
                    onClicked: numpadClicked("9")
                }
            }

            Row {
                spacing: 5
                TuneNumButton {
                    text: fmRadio ? "." : ""
                    enabled: fmRadio
                    onClicked: numpadClicked(".")
                }
                TuneNumButton {
                    text: "0"
                    onClicked: numpadClicked("0")
                }
                TuneNumButton {
                    text: "←"
                    onClicked: numpadClicked("-")
                    onLongClicked: { freqText.text = "" }
                }
            }
        }
    }
}
