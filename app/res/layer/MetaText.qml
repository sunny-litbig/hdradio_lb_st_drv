import QtQuick 2.6
import "../view"

Item {
    id: base

    property alias funcST: funcSTicon.selected
    property alias funcLOC: funcLOCicon.selected
    property alias funcAF: funcAFicon.selected
    property alias funcTA: funcTAicon.selected
    property alias funcPTY: funcPTYicon.selected

    property alias announcement: announcementText.text
    property alias station: stationText.text
    property alias presetNo: presetMarkText.text
    property alias frequency: freqText.text
    property alias song: songText.text
    property alias artist: artistText.text
    property alias album: albumText.text

    // Radio functions
    Row {
        id: funcLayer
        x: 60
        y: 15
        spacing: 15

        ImageButton {
            id: funcSTicon
            visible: true
            source: "../image/rds_st_select.png"
//            source: "../image/rds_st_default.png"
//            {
//                if (pressed) return "../image/rds_st_press.png"
//                if (selected) return "../image/rds_st_select.png"
//                return "../image/rds_st_default.png"
//            }
//            onClicked: {
//                selected = !selected
//            }
        }
        ImageButton {
            id: funcLOCicon
            visible: false
            source: "../image/rds_loc_default.png"
//            {
//                if (pressed) return "../image/rds_loc_press.png"
//                if (selected) return "../image/rds_loc_select.png"
//                return "../image/rds_loc_default.png"
//            }
//            onClicked: {
//                selected = !selected
//            }
        }
        ImageButton {
            id: funcAFicon
            visible: false
            source: "../image/rds_af_default.png"
//            {
//                if (pressed) return "../image/rds_af_press.png"
//                if (selected) return "../image/rds_af_select.png"
//                return "../image/rds_af_default.png"
//            }
//            onClicked: {
//                selected = !selected
//            }
        }
        ImageButton {
            id: funcTAicon
            visible: false
            source: "../image/rds_ta_default.png"
//            {
//                if (pressed) return "../image/rds_ta_press.png"
//                if (selected) return "../image/rds_ta_select.png"
//                return "../image/rds_ta_default.png"
//            }
//            onClicked: {
//                selected = !selected
//            }
        }
        ImageButton {
            id: funcPTYicon
            visible: false
            source: "../image/rds_pty_default.png"
//            {
//                if (pressed) return "../image/rds_pty_press.png"
//                if (selected) return "../image/rds_pty_select.png"
//                return "../image/rds_pty_default.png"
//            }
//            onClicked: {
//                selected = !selected
//            }
        }
    }

    // Announcement
    Image {
        id: announceBoxLeft
        visible: true
        x: 60
        y: funcLayer.y + funcLayer.height + 15
        source: "../image/announcement_bar_left.png"
    }
    Image {
        id: announceBoxCenter
        visible: true
        width: announcementText.width - 12
        x: announceBoxLeft.x + announceBoxLeft.width
        anchors.verticalCenter: announceBoxLeft.verticalCenter
        source: "../image/announcement_bar_center.png"
    }
    Image {
        id: announceBoxRight
        visible: true
        x: announceBoxCenter.x + announceBoxCenter.width
        anchors.verticalCenter: announceBoxLeft.verticalCenter
        source: "../image/announcement_bar_right.png"
    }
    Text {
        id: announcementText
        visible: true
        x: announceBoxLeft.x + announceBoxLeft.width - 6
        anchors.verticalCenter: announceBoxLeft.verticalCenter

        color: "black"
        font.bold: true
        font.family: win.extraBold
        font.pixelSize: 20
        text: "Music"
    }

    // Station
    AnimationText {
        id: stationText
        visible: true
        x: announceBoxRight.x + announceBoxRight.width + 10
        anchors.verticalCenter: announceBoxRight.verticalCenter
        width: 380

        color: "white"
        fontFamily: win.boldFont
        fontPixelSize: 24
        text: "MBC FM4U"
    }

    // Frequency
    AnimationText {
        id: freqText
        x: 60
        y: songText.visible ? (announceBoxLeft.y + announceBoxLeft.height + 15) : 100
        width: 380

        color: "white"
        fontFamily: win.boldFont
        fontPixelSize: songText.visible ? 80 : 110 //80
    }
    Image {
        id: freqUnit
        x: freqText.x + freqText.width
        anchors.bottom: freqText.bottom
        anchors.bottomMargin: 30
        source: "../image/mhz.png"
    }

    Text {
        id: presetMarkText
        x: 30
        anchors.bottom: freqText.bottom
        anchors.bottomMargin: 20

        color: "#ff004e"
        font.family: win.boldFont
        font.pixelSize: 18
    }

    // Song meta
    Image {
        id: songIcon
        visible: true
        x: 60
        y: freqText.y + freqText.height + 6
        source: "../image/icon_song.png"
    }
    AnimationText {
        id: songText
        visible: true
        x: songIcon.x + songIcon.width + 10
        anchors.verticalCenter: songIcon.verticalCenter

        color: "white"
        fontFamily: win.boldFont
        fontPixelSize: 24
        text: "The Man Who Never Lied"
        width : 380
        elide: Text.ElideRight
    }
    Image {
        id: artistIcon
        visible: true
        x: 60
        y: songIcon.y + songIcon.height + 12
        source: "../image/icon_artist.png"
    }
    AnimationText {
        id: artistText
        visible: true
        x: artistIcon.x + artistIcon.width + 10
        anchors.verticalCenter: artistIcon.verticalCenter

        color: "white"
        fontFamily: win.boldFont
        fontPixelSize: 16
        text: "Maroon 5"
        width : 380
        elide: Text.ElideRight
    }
    Image {
        id: albumIcon
        visible: true
        x: 60
        y: artistIcon.y + artistIcon.height + 12
        source: "../image/icon_album.png"
    }
    AnimationText {
        id: albumText
        visible: true
        x: albumIcon.x + albumIcon.width + 10
        anchors.verticalCenter: albumIcon.verticalCenter

        color: "white"
        fontFamily: win.boldFont
        fontPixelSize: 16
        text: "Overexposed (2012)"
        width : 380
        elide: Text.ElideRight
    }
}
