import QtQuick 2.6
import "../view"

Item {
    id: base
    x: 400
    visible: false

    property bool cancelable: true
    property bool isSourceMenu: false
    signal showHdRadioService

    function show() {
        base.x = 400
        base.visible = true
        showAnim.running = true
    }

    function hide() {
        hideAnim.running = true
    }

    NumberAnimation on x {
        id: showAnim
        running: false
        duration: 200
        from: base.x
        to: 0
    }

    NumberAnimation on x {
        id: hideAnim
        running: false
        duration: 200
        from: base.x
        to: 400

        onStopped: {
            win.removePopover()
        }
    }

    Image {
        id: bg
        x: 624
        y: 0
        source: "../image/lnb_back.png"
    }

    Column {
        x: 624
        spacing: 1

        Image {
            id: header
            source: "../image/lnb_bar.png"
        }
        SideMenuItem {
            id: dabStation
            text: win.fmRadio ? "FM Stations" : "AM Stations"
            onClicked: {
                win.setContents("../scene/radio_station_list.qml")
            }
        }
/*
        SideMenuItem {
            id: hdRadioService
            visible : win.fmRadio
            showCheckbox: true
            checked: win.hdRadioChecked
            text: "HD Radio"
            onClicked: {
                checked=!checked
                win.hdRadioChecked=checked
                showHdRadioService()
            }
        }
*/
        SideMenuItem {
            id: version
            text: "Middleware Version"
            onClicked: {
                win.showPopover("../popup/VersionPopup.qml")
            }
        }
    }
}
