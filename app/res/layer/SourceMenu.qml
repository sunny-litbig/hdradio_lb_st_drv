import QtQuick 2.6
import "../view"


Item {
    id: base
    //x: 30
    //y: -100
    x:0
    y:0
    visible: false
    property bool cancelable: true
    property bool isSourceMenu: true
    signal hideDimmedlayer

    Row{
        id: row
        x: 80
        y: -44
        Text {
            id: sourceTextList
            width :115
            //anchors.verticalCenter:
            color: "white"
            font.family: win.boldFont
            font.pixelSize: 24
            text: win.fmRadio ? "FM Radio" : "AM Radio"
        }

        Image {
            id: sourcecheckbox
            anchors.verticalCenterOffset: 0
            anchors.verticalCenter: parent.verticalCenter
            source: "../image/droplist_icon_selet.png"
        }

    }



    function show() {
        //base.y = -100
        if(win.fmRadio){
            fmRadioList.showCheckbox = true
        }
        else{
            amRadioList.showCheckbox = true
        }

        base.visible = true
        showAnim.running = true
    }

    function hide() {
        hideAnim.running = true
    }

    NumberAnimation on y {
        id: showAnim
        running: false
        duration: 200
        from: 60
        to: 60
    }

    NumberAnimation on y {
        id: hideAnim
        running: false
        duration: 200
        from: 60
        to: 60

        onStopped: {
            win.removePopover()
        }
    }

    Column {
        x: 60
        //spacing: 1
        SourceMenuItem {
            id: fmRadioList
            showCheckbox: false
            text: "FM Radio"
            source: pressed ? "../image/droplist_bar_press.png" : "../image/droplist_bar_default.png"
            onClicked: {
                  sourceTextList.text = "FM Radio"
                  fmRadioList.showCheckbox = true
                  amRadioList.showCheckbox = false
                  dabList.showCheckbox = false
                  win.fmRadio = true
                  Native.setBandFM(fmRadio)
                  win.setContentsHome()
            }
        }
        Image {
            id: line2
            source: "../image/droplist_line.png"
        }
        SourceMenuItem {
            id: amRadioList
            showCheckbox: false
            text: "AM Radio"
            source: pressed ? "../image/droplist_bar_press.png" : "../image/droplist_bar_default.png"
            onClicked: {
                sourceTextList.text = "AM Radio"
                fmRadioList.showCheckbox = false
                amRadioList.showCheckbox = true
                dabList.showCheckbox = false
                win.fmRadio = false
                Native.setBandFM(fmRadio)
                win.setContentsHome()
            }
        }
        Image {
            id: line3
            source: "../image/droplist_line.png"
        }
        SourceMenuItem {
            id: dabList
            showCheckbox: false
            text: "DAB"
            source: pressed ? "../image/droplist_bar_press.png" : "../image/droplist_bar_default.png"
            onClicked: {
                //sourceTextList.text = "DAB"
                fmRadioList.showCheckbox = false
                amRadioList.showCheckbox = false
                dabList.showCheckbox = true
                hideDimmedlayer()
                win.isDab=true
                Native.setLatestDab(true)
                Native.backToDab()
            }
        }
    }
}
