import QtQuick 2.6

ImageButton {
    id: base

    property int frequency: 0
    property int presetNo: 0

    function update(index, curFreq) {
        var data = Native.getPresetData(index)
        var freq = data["freq"]
        var name = data["name"]

        if (name.length === 0) {
            name = "Empty"
        }
        else{
           if(win.fmRadio){
               name += " MHz"
           }
           else{
               name += " KHz"
           }
         }
        presetName.text = name

        selected = (freq === curFreq)
        frequency = freq
        presetNo = index
    }

    source: {
        if (selected) return "../image/btn_preset_select.png"
        if (pressed) return "../image/btn_preset_press.png"
        return "../image/btn_preset_default.png"
    }

    onClicked: {
        if (frequency > 0) {
            Native.setFrequencyInt(frequency)
        }
    }
    onLongClicked: {
        Native.savePreset(presetNo)
    }

    Text {
        id: presetNoText
        x: 10
        y: 10
        color: "white"
        font.family: win.boldFont
        font.pixelSize: 14
        text: "P" + (presetNo + 1)
    }
    Text {
        id: presetName
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        horizontalAlignment: Text.Center
        color: "white"
        font.family: win.boldFont
        font.pixelSize: 20
    }
}
