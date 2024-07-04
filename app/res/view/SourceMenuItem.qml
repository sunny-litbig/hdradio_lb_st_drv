import QtQuick 2.0

ImageButton {
    id: base

    property alias text: menuText.text
    property alias showCheckbox: checkBox.visible

    property bool checked: false

    source: pressed ? "../image/lnb_list_press.png" : "../image/lnb_list_default.png"

    Text {
        id: menuText
        x: 50
        anchors.verticalCenter: parent.verticalCenter

        color: pressed ? "white" : "black"
        font.family: win.boldFont
        font.pixelSize: 20
    }

    Image {
        id: checkBox
        x: 10
        anchors.verticalCenter: parent.verticalCenter
        source: "../image/droplist_check_select.png"
        //source: parent.checked ? "../image/droplist_check_select.png" : "../image/droplist_check_press.png"
        visible: false
    }
}
