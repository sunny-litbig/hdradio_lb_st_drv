import QtQuick 2.6
import "../view"

Image {
    source: "../image/menu_bar.png"

    property alias sourceName: sourceText.text

    property var bandbtn: true

    signal sourceChangeClicked


    // Source text
    Text {
        id: sourceText
        width: 115

        x: 20
        anchors.verticalCenter: parent.verticalCenter

        color: "white"
        font.family: win.boldFont
        font.pixelSize: 24
        text: qsTr("FM Radio")

    }

    // Dropdown Arrow Comment
//    Image {
//            x: sourceText.x + 115
//            id: sourcecheckbox
//            anchors.verticalCenterOffset: 0
//            anchors.verticalCenter: parent.verticalCenter
//            source: "../image/droplist_icon_selet.png"
//    }

    MouseArea {
        width: sourceText.width+ 30
        height: 60
        anchors.left: sourceText.left
        enabled: bandbtn
        onClicked: {
            bandbtn = false;
            mtimer.start();
            sourceChangeClicked()
        }
    }

    Timer {
            id:mtimer
            interval: 300;
            repeat: false
            onTriggered: {
                bandbtn = true;
            }
        }

    // HD Radio Logo
    Image {
    	id: hdLogoBar
    	x: 870
    	y: 10
    	source: "../image/hd_logo_full_name.png"
    	visible: true
    }

/*
    // Clock
    Text {
        id: clockText
        x: parent.width - 20 - width
        anchors.verticalCenter: parent.verticalCenter

        color: "white"
        font.family: win.boldFont
        font.pixelSize: 20
        text: new Date().toLocaleTimeString(Qt.locale(), "hh:mm A")
    }
    Timer {
        id: clockTimer
        interval: 1000
        repeat: true
        running: true
        onTriggered:
            clockText.text = new Date().toLocaleTimeString(Qt.locale(), "hh:mm A")
    }
*/
}
