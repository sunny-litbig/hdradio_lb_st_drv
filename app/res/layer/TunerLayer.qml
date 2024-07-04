import QtQuick 2.6
import "../view"

Item {
    id: base
    property bool showHdIcon: false
    property int hd: 1
    property var debug_count: 0

    property alias ishdRadio : hdLayer.visible
    property alias ishdRadioIcon : hdIcon.visible

    signal autoScanClicked
    signal tuneButtonClicked
    signal menuButtonClicked

	// Debug Mode
    ImageButton {
        id: debugMode
        x: 10
        y: 10
        width: 70
        height: 70
        source: ""
        onClicked: {
            if(debug_count > 2){
               debug_count = 0
            //   Native.setHdrAudio(77)		// for test (hdr digital audio data dump)
            }else{
               debug_count++
            }
        }
    }
    
    // Auto scan
    ImageButton {
        id: autoScan
        x: 60
        source: pressed ? "../image/auto_scan_press.png" : "../image/auto_scan_default.png"
        onClicked: {
            console.log("onClickedPressed: " + Native.getPresetScan())
            console.log("win fm raido : " + win.fmRadio)
            if(!(Native.getPresetScan())){
               if(win.fmRadio){
                   console.log("setFMfrequency")
                   Native.setFrequency("87.5")
               }
               else{
                   console.log("setAMfrequency")
                   Native.setFrequency("530")
               }
               autoScanClicked()
            }
            else{
               console.log("stop scan")
               Native.setPresetScan(false);
               Native.stopScan()
            }
        }
    }

    // Tune
    ImageButton {
        id: tunePrev
        x: autoScan.x + autoScan.width + 30
        anchors.verticalCenter: autoScan.verticalCenter
        source: pressed ? "../image/prev_press.png" : "../image/prev_default.png"
        onClicked: {
            Native.seekManualDown()
            hd = 1
        }
        onLongClicked: {
            Native.seekAutoDown()
            hd = 1
        }
    }
    ImageButton {
        id: tuneButton
        x: tunePrev.x + tunePrev.width + 10
        anchors.verticalCenter: tunePrev.verticalCenter
        source: pressed ? "../image/tune_press.png" : "../image/tune_default.png"
        onClicked: {
            win.setContents("../scene/radio_direct_tune.qml")
        }
    }
    ImageButton {
        id: tuneNext
        x: tuneButton.x + tuneButton.width + 10
        anchors.verticalCenter: tuneButton.verticalCenter
        source: pressed ? "../image/next_press.png" : "../image/next_default.png"
        onClicked: {
            Native.seekManualUp()
            hd = 1
        }
        onLongClicked: {
            Native.seekAutoUp()
            hd = 1
        }
    }

    // Menu
    ImageButton {
        id: menuButton
        x: tuneNext.x + tuneNext.width + 30
        anchors.verticalCenter: tuneNext.verticalCenter
        source: pressed ? "../image/menu_press.png" : "../image/menu_default.png"
        onClicked: {
            menuButtonClicked()
        }
    }

    // HD
    Image {
        id: hdIcon
        x: menuButton.x + menuButton.width + 22
        anchors.verticalCenter: menuButton.verticalCenter
        visible: (Native.hdrStatus == 0) ? false : true
        source: (Native.hdrStatus == 3) ? "../image/hd_icon_new.png" : "../image/hd_icon_new_mono.png"
    //    source: "../image/hd_icon.png"
    //    visible: showHdIcon
    }
    Column {
        id: hdLayer
        x: hdIcon.x + hdIcon.width
        anchors.verticalCenter: hdIcon.verticalCenter
        visible: (Native.hdrStatus == 0) ? false : true
    //  visible: showHdIcon

        Row {
            ImageButton {
                onClicked: {
               		hd = 1
               		Native.setHdrService(hd-1);
                }
                source: {
                   	if (pressed) return "../image/hd_1_press.png"
                   	return hd == 1 ? "../image/hd_1_select.png" : "../image/hd_1_w.png" 
                }
            }
            ImageButton {
                onClicked: {
	                if(Native.hdrAvailableProgram & 0x02) {
               		 	hd = 2
                		Native.setHdrService(hd-1);
                	}
                }
                source: {
                	if(Native.hdrAvailableProgram & 0x02) {
                    	if (pressed) return "../image/hd_2_press.png"
                    	return hd == 2 ? "../image/hd_2_select.png" : "../image/hd_2_w.png"
                    }
                    return "../image/hd_2_default.png"
                }
            }
            ImageButton {
                onClicked: {
                	if(Native.hdrAvailableProgram & 0x04) {
                		hd = 3
                		Native.setHdrService(hd-1);
                	}
                }
                source: {
                	if(Native.hdrAvailableProgram & 0x04) {
                    	if (pressed) return "../image/hd_3_press.png"
                    	return hd == 3 ? "../image/hd_3_select.png" : "../image/hd_3_w.png"
                    }
                    return "../image/hd_3_default.png"
                }
            }
            ImageButton {
                onClicked: {
                	if(Native.hdrAvailableProgram & 0x08) {
                		hd = 4
                		Native.setHdrService(hd-1);
                	}
                }
                source: {
                	if(Native.hdrAvailableProgram & 0x08) {
                    	if (pressed) return "../image/hd_4_press.png"
                    	return hd == 4 ? "../image/hd_4_select.png" : "../image/hd_4_w.png"
                    }
                    return "../image/hd_4_default.png"
                }
            }
        }

        Row {
            ImageButton {
                onClicked: {
                	if(Native.hdrAvailableProgram & 0x10) {
	                	hd = 5
	                	Native.setHdrService(hd-1);
                	}
                }
                source: {
                	if(Native.hdrAvailableProgram & 0x10) {
	                    if (pressed) return "../image/hd_5_press.png"
	                    return hd == 5 ? "../image/hd_5_select.png" : "../image/hd_5_w.png"
                    }
                    return "../image/hd_5_default.png"
                }
            }
            ImageButton {
                onClicked: {
                	if(Native.hdrAvailableProgram & 0x20) {
                		hd = 6
                		Native.setHdrService(hd-1);
                	}
                }
                source: {
                	if(Native.hdrAvailableProgram & 0x20) {
                    	if (pressed) return "../image/hd_6_press.png"
                    	return hd == 6 ? "../image/hd_6_select.png" : "../image/hd_6_w.png"
                    }
                    return "../image/hd_6_default.png"
                }
            }
            ImageButton {
                onClicked: {
                	if(Native.hdrAvailableProgram & 0x40) {
	                	hd = 7
	                	Native.setHdrService(hd-1);
                	}
                }
                source: {
                	if(Native.hdrAvailableProgram & 0x40) {
                    	if (pressed) return "../image/hd_7_press.png"
                    	return hd == 7 ? "../image/hd_7_select.png" : "../image/hd_7_w.png"
                    }
                    return "../image/hd_7_default.png"
                }
            }
            ImageButton {
                onClicked: {
	                if(Native.hdrAvailableProgram & 0x80) {
                		hd = 8
                		Native.setHdrService(hd-1);
                	}
                }
                source: {
                	if(Native.hdrAvailableProgram & 0x80) {
                    	if (pressed) return "../image/hd_8_press.png"
                    	return hd == 8 ? "../image/hd_8_select.png" : "../image/hd_8_w.png"
                    }
                    return "../image/hd_8_default.png"
                }
            }
        }
    }
}
