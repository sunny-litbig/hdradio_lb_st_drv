import QtQuick 2.6
import "../layer"

Item {
    id: base

    StationLogo {
        id: stationLogo
        x: 528
        y: 15
        logo: "../image/graphic_am_default.png"
        showText: false
    }

    FreqText {
        id: freqText
        width: 624
        height: 300
        frequency: Native.frequency
        presetNo: (Native.presetNo < 0) ? "" : "P" + (Native.presetNo + 1)
    }

    TunerLayer {
        id: tuneLayer
        width: parent.width
        height: 60
        y: parent.height - 221

        showHdIcon: false

        onMenuButtonClicked: {
            win.showPopover("../layer/SideMenu.qml")
        }
        onAutoScanClicked: {
            Native.autoScan(true)
        }
    }

    PresetLayer {
        id: presetLayer
        width: parent.width
        height: 136
        anchors.bottom: parent.bottom
        frequency: Native.frequency
    }

    Connections {
        target: Native
        onStationChanged: {
            console.log("Station list changed: " + count)
            presetLayer.update()
        }
    }
}
