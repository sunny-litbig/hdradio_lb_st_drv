import QtQuick 2.6
import "../layer"

Item {
    id: base
    property alias tuneLayerId: tuneLayer

    StationLogo {
        id: stationLogo
        x: 528
        y: 15
        logo: "../image/graphic_fm_default.png"
        showText: false
    }

    MetaText {
        id: metaText
        width: 624
        height: 300
        station: Native.staionShortName
        announcement: Native.metaProgramType
        song: Native.metaTitle
        artist: Native.metaArtist
        album: Native.metaAlbum        
        frequency: (Native.frequency * 0.001).toFixed(2)
        presetNo: (Native.presetNo < 0) ? "" : "P" + (Native.presetNo + 1)
    }

    TunerLayer {
        id: tuneLayer
        width: parent.width
        height: 60
        y: parent.height - 221

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
