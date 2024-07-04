import QtQuick 2.6

Image {
    property alias logo: logoImage.source

    property bool showText: true

    source: "../image/graphic_shadow_02.png"

    Image {
        id: frameImage
        x: 36
        y: 25
        source: "../image/graphic_area_02.png"
    }

    Image {
        id: logoImage
        x: 36
        y: 25
        source: "../image/graphic_fm_default.png"
    }

    Image {
        id: textDim
        x: 36
        y: 25
        source: "../image/text_dim_01.png"
        visible: showText
    }

    Column {
        id: radioTextLayer
        width: textDim.width - 40
        height: textDim.height - 38
        x: 56
        y: 44
        clip: true
        spacing: 7 // 8 in UI guide
        visible: showText

        Text {
            id: radioTextRow1
            color: "white"
            font.family: win.boldFont
            font.pixelSize: 20
            text: "학교 다닐 때 전혀 도움이 되지 않는 삶의"
            opacity: 0.2
        }
        Text {
            id: radioTextRow2
            color: "white"
            font.family: win.boldFont
            font.pixelSize: 20
            text: "지도, 지식의 지도 때문에 고생했던 경험이"
            opacity: 0.4
        }
        Text {
            id: radioTextRow3
            color: "white"
            font.family: win.boldFont
            font.pixelSize: 20
            text: "그로 하여금 <당혹한 이들을 위한 안내서>를"
            opacity: 0.6
        }
        Text {
            id: radioTextRow4
            color: "white"
            font.family: win.boldFont
            font.pixelSize: 20
            text: "쓰게 했습니다."
            opacity: 0.8
        }
        Text {
            id: radioTextRow5
            color: "white"
            font.family: win.boldFont
            font.pixelSize: 20
            text: " "
        }
        Text {
            id: radioTextRow6
            color: "white"
            font.family: win.boldFont
            font.pixelSize: 20
            text: "-학교 다닐 때 전혀 도움이 되지 않는 삶의"
        }
        Text {
            id: radioTextRow7
            color: "white"
            font.family: win.boldFont
            font.pixelSize: 20
            text: "지도, 지식의 지도 때문에 고생했던 경험이"
        }
    }

    Image {
        id: textGradient
        x: 36
        y: 25
        source: "../image/text_dim_02.png"
        visible: showText
    }
}
