import QtQuick 2.0

ImageButton {
    id: base

    property alias text: menuText.text
    property alias showCheckbox: checkBox.visible

    property bool checked: false

    source: pressed ? "../image/lnb_list_press.png" : "../image/lnb_list_default.png"

    Text {
        id: menuText
        x: 30
        anchors.verticalCenter: parent.verticalCenter

        color: pressed ? "white" : "black"
        font.family: win.boldFont
        font.pixelSize: 20
    }

    Image {
        id: checkBox
        x: parent.width - width - 10
        anchors.verticalCenter: parent.verticalCenter
        source: parent.checked ? "../image/lnb_check_select.png" : "../image/lnb_check_default.png"
        visible: false
    }
}
