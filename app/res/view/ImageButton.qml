import QtQuick 2.6

Image {
    id: base

    signal clicked
    signal longClicked

    property alias pressed: touch.pressed

    property bool selected: false

    MouseArea {
        id: touch
        anchors.fill: parent
        hoverEnabled: false

        onClicked: {
            base.clicked()
        }

        onPressAndHold: {
            base.longClicked()
        }
    }
}
