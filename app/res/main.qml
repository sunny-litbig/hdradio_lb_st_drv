import QtQuick 2.6
import QtQuick.Window 2.2
import "layer"
import "view"

Window {
    id: win

	x: (Screen.width - 1024) / 2
	y: (Screen.height - 600) / 2 + 65
    width: 1024
    height: 535
    flags: Qt.FramelessWindowHint
    visible: DEBUG_MODE

    title: qsTr("TCC-Radio")

    readonly property alias regularFont: nanumSquareReqgular.name
    readonly property alias boldFont: nanumSquareBold.name
    readonly property alias extraBold: nanumSquareExtra.name

    property variant viewStack: []
    property bool fmRadio: true
    property bool hdRadioChecked : false
    property bool isDab: false

    FontLoader { id: nanumSquareReqgular; source: "font/NanumSquareR.ttf" }
    FontLoader { id: nanumSquareBold; source: "font/NanumSquareB.ttf" }
    FontLoader { id: nanumSquareExtra; source: "font/NanumSquareEB.ttf" }

    function setContentsHome() {
        setContents(fmRadio ? "scene/radio_fm_main.qml" : "scene/radio_am_main.qml")
        indicator.sourceName = fmRadio ? "FM Radio" : "AM Radio"
        viewStack = []
    }

    function setContents(source) {
        viewStack.push(rootView.source)
        removePopover()
        rootView.source = source
    }

    function moveToBack() {
        if (viewStack.length > 0) {
            var view = viewStack.pop()
            dimmedLayer.visible = false
            popoverLoader.source = ""
            rootView.source = view
        }
    }

    function showPopover(source) {
        popoverLoader.setSource(source)
        dimmedLayer.show()

    }

    function hidePopover() {
        dimmedLayer.hide()
    }

    function removePopover() {
        dimmedLayer.opacity = 0
        dimmedLayer.visible = false
        popoverLoader.source = ""
    }

    // Background
    ImageChanger {
        id: backgroundImage
        width: parent.width
        height: parent.height
    }

    // Indicator
    Indicator {
        id: indicator
        width: parent.width
        height: 60

        onSourceChangeClicked: {
            // Disable Source Menu
            //win.showPopover("../layer/SourceMenu.qml")
            // AM/FM Radio Toggle
            fmRadio = !fmRadio
            Native.setBandFM(fmRadio)
            setContentsHome()
        }
    }
    // Main contents
    Loader {
        id: rootView
        y: indicator.y + indicator.height
        width: parent.width
        height: parent.height - y
    }

    // Dimmed layer for pop-over contents
    Rectangle {
        id: dimmedLayer
        width: parent.width
        height: parent.height
        color: "black"
        opacity: 0
        visible: false

        MouseArea {
            id: dimClick
            anchors.fill: parent
            onClicked: {
                if (popoverLoader.item.cancelable) {
                    dimmedLayer.hide()
                }
            }
        }

        OpacityAnimator {
            id: dimShowAnim
            target: dimmedLayer
            running: false
            duration: 200
            from: dimmedLayer.opacity
            to: 0.7
        }

        OpacityAnimator {
            id: dimHideAnim
            target: dimmedLayer
            running: false
            duration: 200
            from: dimmedLayer.opacity
            to: 0

            onStopped: {
                dimmedLayer.visible = false
            }
        }

        function show() {
            dimmedLayer.visible = true
            dimShowAnim.running = true
            popoverLoader.item.show()
        }

        function hide() {
            dimHideAnim.running = true
            popoverLoader.item.hide()
        }
    }

    // Pop-over contents
    Loader {
        id: popoverLoader
        width: parent.width
        height: parent.height
    }

    Component.onCompleted: {
        //        if(Native.latestDab()){
        //          Native.backToDab()
        //        }
        //        else{
        //          fmRadio = Native.isCurrentBandFM()
        //          setContentsHome()
        //        }
        fmRadio = Native.isCurrentBandFM()
        setContentsHome()
    }

    Connections {
        target: Native
        onShowApplication: {
            if(isDab){
//                fmRadio = Native.isCurrentBandFM()
//                setContentsHome()
                Native.backToDab()
            }
            else{
            	if(win.visible == false)
            	{
                	console.log("QML show application")
	                win.show()
	            }
            }
        }
        onHideApplication: {
            console.log("QML hide application")
            win.hide()
        }
	onMoveToBack: {
	if (viewStack.length > 0) {
	    var view = viewStack.pop()
			dimmedLayer.visible = false
	    popoverLoader.source = ""
	    rootView.source = view
	}
		else if(dimmedLayer.visible) {
    	dimmedLayer.hide()
		}
		else {
			Native.backToApps()
		}
	}
        onSourceChange: {
            // AM/FM Radio Toggle
            fmRadio = !fmRadio
            Native.setBandFM(fmRadio)
            setContentsHome()
        }

    }
    Connections {
        target : popoverLoader.item
        onHideDimmedlayer: {
            console.log("QML DimmedLayer hide")
            dimmedLayer.hide()
        }
        onShowHdRadioService : {
            console.log("QML hd Radio Service show")
            rootView.item.tuneLayerId.ishdRadioIcon = win.hdRadioChecked
            rootView.item.tuneLayerId.ishdRadio  = win.hdRadioChecked
            if(win.hdRadioChecked) {
            //	Native.setHdrEnable(true)
            	Native.setHdrAudio(2)
            }
            else {
            //	Native.setHdrEnable(false)
            	Native.setHdrAudio(1)
            }
        }
    }

}
