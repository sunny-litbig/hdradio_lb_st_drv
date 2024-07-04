import QtQuick 2.6
import "../view"

Item {
    id: base

    Image {
        id: topBar
        source: "../image/list_bar.png"

        Text {
            x: 30
            anchors.verticalCenter: parent.verticalCenter

            color: "black"
            font.family: win.boldFont
            font.pixelSize: 24
            text: "Station List"
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
        ImageButton {
            id: updateButton
            anchors.right: backButton.left
            anchors.verticalCenter: parent.verticalCenter
            source: pressed ? "../image/list_refresh_press.png" : "../image/list_refresh_default.png"
            onClicked: {
                win.showPopover("../popup/UpdateStationPopup.qml")
                Native.autoScan(false)
            }
        }
    }

    ListView {
        id: stationList
        y: topBar.height
        width: parent.width
        height: parent.height - y
        clip: true
        delegate: StationListItem {
            frequency: model.modelData.frequency
            freqText: model.modelData.freq
            stationName: model.modelData.name === undefined ? "" : model.modelData.name
            onClicked: {
                Native.setFrequencyInt(frequency)
            }
        }
    }
    ScrollBar {
        id: scrollBar
        y: topBar.height
        height: stationList.height
        anchors.right: parent.right
        listView: stationList
    }

    Component.onCompleted: {
        var items = Native.getStationList()
        if (items.length > 0) {
            stationList.model = items
            stationList.update()

        } else {
            win.showPopover("../popup/UpdateStationPopup.qml")
            Native.autoScan(false)
        }
    }

    Connections {
        target: Native
        onStationChanged: {
            var items = Native.getStationList()
            stationList.model = items
            stationList.update()
            win.hidePopover()
        }
    }
}
