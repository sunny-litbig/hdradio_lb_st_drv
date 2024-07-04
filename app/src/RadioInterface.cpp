#include "RadioInterface.h"
#include "AppPreference.h"

#include <QGuiApplication>

extern int g_debug;

#define TC_RADIO_PRINTF(format, arg...) \
	if (g_debug) \
	{ \
        fprintf(stderr, "[TC RADIO] %s: " format "", __FUNCTION__, ##arg); \
    }

#define	RADIO_AUDIO_SAMPLERATE	44100	// or 48000
//#define	USE_EXTERNAL_CONTROL

static RadioInterface* self = nullptr;

void getNotificationCallBack(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode) {
    TC_RADIO_MANAGER->callbackNotification(notifyID, arg, pData, errorCode);
}

void getStationListCallBack(uint32 totalnum, void * list, int32 errorCode) {
    TC_RADIO_MANAGER->callbackStation(totalnum, list, errorCode);
}

int getPrecheckSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata) {
	int ret = -1;
	ret = TC_RADIO_MANAGER->precheckSeekQual(mod_mode, qdata);
	return ret;
}

int getCheckSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata) {
	int ret = -1;
	ret = TC_RADIO_MANAGER->checkSeekQual(mod_mode, qdata);
	return ret;
}

RadioInterface::RadioInterface() {
    self = this;

#ifndef QT_DEBUG
    tcradio_configOnGetNotificationCallBack(getNotificationCallBack);
    tcradio_configOnGetStationListCallBack(getStationListCallBack);

	tcradio_configOnPrecheckSeekQual(getPrecheckSeekQual);
	tcradio_configOnCheckSeekQual(getCheckSeekQual);
#endif

    fmFrequency = AppPreference::getLatestFMFrequency();
    mwFrequency = AppPreference::getLatestAMFrequency();
    latestDab = AppPreference::getLatestDab();

    if (AppPreference::getLatestBand()) {
        currentFrequency = fmFrequency;
        currentBand = eRADIO_FM_BAND;
    } else {
        currentFrequency = mwFrequency;
        currentBand = eRADIO_MW_BAND;
    }

#ifndef QT_DEBUG
    RET ret;

	isRadio = true;
    tcradiocui_setRadioConfig(&radioConf);
    ret = tcradio_init();
    if (ret != eRET_OK) {
        TC_RADIO_PRINTF("Failed to initialize radio: %d", ret);
        QGuiApplication::quit();
        return;
    }

    tcradio_setBandFreqConfig(eRADIO_FM_MODE, FM_INIT_FREQUENCY, FM_LIMIT_FREQUENCY, FM_STEP_FREQUENCY);
    tcradio_setBandFreqConfig(eRADIO_AM_MODE, AM_INIT_FREQUENCY, AM_LIMIT_FREQUENCY, AM_STEP_FREQUENCY);

    qDebug("Radio initialized");
#endif
}

RadioInterface::~RadioInterface() {
#ifndef QT_DEBUG
    tcradio_close();
    tcradio_deinit();
#endif
    self = nullptr;
}

int RadioInterface::setRadioOpen() {
	RET ret;

    fmFrequency = AppPreference::getLatestFMFrequency();
    mwFrequency = AppPreference::getLatestAMFrequency();
    latestDab = AppPreference::getLatestDab();

    if (AppPreference::getLatestBand()) {
        currentFrequency = fmFrequency;
        currentBand = eRADIO_FM_BAND;
    } else {
        currentFrequency = mwFrequency;
        currentBand = eRADIO_MW_BAND;
    }

	isRadio = true;
    tcradiocui_setRadioConfig(&radioConf);

	ret = tcradio_open(&radioConf);
    if (ret != eRET_OK) {
        TC_RADIO_PRINTF("Failed to open radio: %d", ret);
        QGuiApplication::quit();
        return ret;
    }
//	qDebug("Radio opened!!!");
	return ret;
}

int RadioInterface::setRadioOpen(bool radio) {
	RET ret;

    fmFrequency = AppPreference::getLatestFMFrequency();
    mwFrequency = AppPreference::getLatestAMFrequency();
    latestDab = AppPreference::getLatestDab();

    if (AppPreference::getLatestBand()) {
        currentFrequency = fmFrequency;
        currentBand = eRADIO_FM_BAND;
    } else {
        currentFrequency = mwFrequency;
        currentBand = eRADIO_MW_BAND;
    }

	if(radio == false)
	{
		isRadio = false;
	}
    tcradiocui_setRadioConfig(&radioConf);

	ret = tcradio_open(&radioConf);
    if (ret != eRET_OK) {
        TC_RADIO_PRINTF("Failed to open radio: %d", ret);
        QGuiApplication::quit();
        return ret;
    }
//	qDebug("Radio opened!!!");
	return ret;
}

int RadioInterface::setRadioClose() {
	RET ret;
	ret = tcradio_close();

    qDebug("setRadioClose : ========================>>>>>>>>>>>>>>>>>>>>>> Close Radio Function!!!!!!!!");
	emit tunerClosed(ret);
	return ret;
}

bool RadioInterface::getLatestDab() const {
    return latestDab;
}


RadioInterface* RadioInterface::getInterface() {
    return self;
}

eRADIO_BAND_t RadioInterface::getCurrentBand() const {
    return currentBand;
}

uint32 RadioInterface::getCurrentFrequency() const {
    return currentFrequency;
}

uint32 RadioInterface::getStationCount(int band) const {
    return band == eRADIO_FM_BAND ? fmStationCount : mwStationCount;
}

stRADIO_LIST_t* RadioInterface::getStationList(int band) const {
    return band == eRADIO_FM_BAND ? fmStationList : mwStationList;
}

void RadioInterface::saveStatus() {
    AppPreference::setLatestBand(currentBand == eRADIO_FM_BAND);
    AppPreference::setCurrentAMFrequency(mwFrequency);
    AppPreference::setCurrentFMFrequency(fmFrequency);
}

void RadioInterface::switchToAM() {
    if (currentBand == eRADIO_MW_BAND) {
        TC_RADIO_PRINTF("Already MW, ignore");
        return;
    }

    TC_RADIO_PRINTF("Set to MW");

    // Back up current frequency
    fmFrequency = currentFrequency;

    // Set new band and frequency
    currentBand = eRADIO_MW_BAND;
    currentFrequency = mwFrequency;
#ifndef QT_DEBUG
    tcradio_setTune(eRADIO_AM_MODE, currentFrequency, eRADIO_TUNE_NORMAL, eRADIO_ID_PRIMARY);
#endif
    emit frequencyChanged(currentFrequency);
}

void RadioInterface::switchToFM() {
    if (currentBand == eRADIO_FM_BAND) {
        TC_RADIO_PRINTF("Already FM, ignore");
        return;
    }

    TC_RADIO_PRINTF("Set to FM");

    // Back up current frequency
    mwFrequency = currentFrequency;

    // Set new band and frequency
    currentBand = eRADIO_FM_BAND;
    currentFrequency = fmFrequency;
#ifndef QT_DEBUG
    tcradio_setTune(eRADIO_FM_MODE, currentFrequency, eRADIO_TUNE_NORMAL, eRADIO_ID_PRIMARY);
#endif
    emit frequencyChanged(currentFrequency);
}

void RadioInterface::setRadioMute(int onoff) {
    TC_RADIO_PRINTF("Set radio Mute [%d]", onoff);

#ifndef QT_DEBUG
    tcradio_setAudio(onoff);
#endif
}

void RadioInterface::setRadioPCMOpen() {
    TC_RADIO_PRINTF("Set radio pcm open ");

#ifndef QT_DEBUG
	if(radioConf.fExtAppCtrl==0) {
		tcradio_setAudioDevice(&radioConf, ON);
	}
#endif
}

void RadioInterface::setRadioPCMClose() {
    TC_RADIO_PRINTF("Set radio pcm close ");

#ifndef QT_DEBUG
	if(radioConf.fExtAppCtrl==0) {
		tcradio_setAudioDevice(&radioConf, OFF);
	}
#endif
}

void RadioInterface::setDabState(bool dabValue)
{
    latestDab = dabValue;
    AppPreference::setCurrentDabState(latestDab);
}
void RadioInterface::setFrequency(int freq) {
    TC_RADIO_PRINTF("Set frequency - %d", freq);
#ifndef QT_DEBUG
	tcradio_setAudio(OFF);
	if(currentBand == eRADIO_FM_BAND) {
    	tcradio_setTune(eRADIO_FM_MODE, freq, eRADIO_TUNE_NORMAL, eRADIO_ID_PRIMARY);
	}
	else {
		tcradio_setTune(eRADIO_AM_MODE, freq, eRADIO_TUNE_NORMAL, eRADIO_ID_PRIMARY);
	}
    saveStatus();
#else
    currentFrequency = freq;
    if (currentBand == eRADIO_FM_BAND) {
		fmFrequency = freq;
    } else {
        mwFrequency = freq;
    }
    emit frequencyChanged(currentFrequency);
#endif
}

void RadioInterface::radioSeekManual(bool up) {
    TC_RADIO_PRINTF("Seek manual - %d", up);
#ifndef QT_DEBUG
    tcradio_setSeek(up ? eRADIO_SEEK_MAN_UP : eRADIO_SEEK_MAN_DOWN, NULL);
  #ifdef USE_HDRADIO
	stTC_HDR_PSD_FORM_t hdrPsd;
	stTC_HDR_SIS_t hdrSis;
	memset(hdrPsd.data, 0 , sizeof(hdrPsd.data));
	memset(hdrSis.shortName.text, 0, sizeof(hdrSis.shortName.text));
	emit hdrTitleChanged((char*)hdrPsd.data);
	emit hdrArtistChanged((char*)hdrPsd.data);
	emit hdrAlbumChanged((char*)hdrPsd.data);
	emit hdrStationShortNameChanged((char*)hdrSis.shortName.text);
	emit hdrProgramTypeChanged(0);
  #endif
	saveStatus();
#else
    currentFrequency += up ? 100 : -100;
    if (currentBand == eRADIO_FM_BAND) {
		fmFrequency = currentFrequency;
    } else {
        mwFrequency = currentFrequency;
    }
    emit frequencyChanged(currentFrequency);
#endif
}

void RadioInterface::radioSeekAuto(bool up) {
    TC_RADIO_PRINTF("Seek auto - %d", up);
#ifndef QT_DEBUG
    tcradio_setSeek(up ? eRADIO_SEEK_AUTO_UP : eRADIO_SEEK_AUTO_DOWN, NULL);
  #ifdef USE_HDRADIO
	stTC_HDR_PSD_FORM_t hdrPsd;
	stTC_HDR_SIS_t hdrSis;
	memset(hdrPsd.data, 0 , sizeof(hdrPsd.data));
	memset(hdrSis.shortName.text, 0, sizeof(hdrSis.shortName.text));
	emit hdrTitleChanged((char*)hdrPsd.data);
	emit hdrArtistChanged((char*)hdrPsd.data);
	emit hdrAlbumChanged((char*)hdrPsd.data);
	emit hdrStationShortNameChanged((char*)hdrSis.shortName.text);
	emit hdrProgramTypeChanged(0);
  #endif
#else
    currentFrequency += up ? 100 : -100;
    if (currentBand == eRADIO_FM_BAND) {
        fmFrequency = currentFrequency;
    } else {
   		mwFrequency = currentFrequency;
    }
    emit frequencyChanged(currentFrequency);	
#endif
}

void RadioInterface::radioSeekList() {
    TC_RADIO_PRINTF("seek list");
#ifndef QT_DEBUG
    tcradio_setSeek(eRADIO_SEEK_SCAN_STATION, NULL);
#endif
}

void RadioInterface::radioSeekStop() {
    TC_RADIO_PRINTF("Seek stop");
#ifndef QT_DEBUG
    tcradio_setSeek(eRADIO_SEEK_STOP, NULL);
#endif
}

void RadioInterface::radioSeekPIList(unsigned int *pilist, unsigned int cnt) {
    TC_RADIO_PRINTF("seek PI list");
	//TODO : malloc pilist array
	(void)cnt;
    tcradio_setSeek(eRADIO_SEEK_SCAN_PI, pilist);
}

void RadioInterface::getRadioFrequency(eRADIO_MOD_MODE_t *mod_mode, unsigned int *freq, int ntuner) {
    TC_RADIO_PRINTF("");
#ifndef QT_DEBUG
    tcradio_getTune(mod_mode, freq, ntuner);
#endif
}

void RadioInterface::getRadioQuiality(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t * qdata, unsigned int ntuner) {
    TC_RADIO_PRINTF("");
#ifndef QT_DEBUG
    tcradio_getQuality(mod_mode, qdata, ntuner);
#endif
}

void RadioInterface::setHdrEnable(int onoff) {
    TC_RADIO_PRINTF("");
	(void)onoff;
#ifndef QT_DEBUG
//    tcradio_setHdrEnable(onoff);
#endif
}
void RadioInterface::setHdrAudio(int audio) {
    TC_RADIO_PRINTF("");
	(void)audio;
#ifndef QT_DEBUG
//    tcradio_setHdrAudio(audio);
#endif
}

void RadioInterface::setHdrService(int service) {
    TC_RADIO_PRINTF("");
#ifndef QT_DEBUG
#ifdef USE_HDRADIO
    tcradio_setHdrProgram((unsigned int)service);
#else
	(void)service;
#endif
#endif
}

void RadioInterface::setHdrLotOpen(int service, int port) {
    TC_RADIO_PRINTF("");
	(void)service;
	(void)port;
#ifndef QT_DEBUG
//    tcradio_setHdrLotOpen(service, port);
#endif
}

void RadioInterface::setHdrLotFlush(int service, int port) {
    TC_RADIO_PRINTF("");
	(void)service;
	(void)port;
#ifndef QT_DEBUG
//    tcradio_setHdrLotFlush(service, port);
#endif
}

void RadioInterface::getHdrPsd(int prgno) {
    TC_RADIO_PRINTF("");
	(void)prgno;
#ifndef QT_DEBUG
//    tcradio_getHdrPsd(prgno);
#endif
}

void RadioInterface::getHdrSis() {
    TC_RADIO_PRINTF("");
#ifndef QT_DEBUG
//    tcradio_getHdrSis();
#endif
}

void RadioInterface::getHdrSig() {
    TC_RADIO_PRINTF("");
#ifndef QT_DEBUG
//    tcradio_getHdrSig();
#endif
}

void RadioInterface::getHdrStatus() {
    TC_RADIO_PRINTF("");
#ifndef QT_DEBUG
//    tcradio_getHdrStatus();
#endif
}

void RadioInterface::getHdrAlert() {
    TC_RADIO_PRINTF("");
#ifndef QT_DEBUG
//    tcradio_getHdrAlert();
#endif
}

void RadioInterface::getHdrLot() {
    TC_RADIO_PRINTF("");
#ifndef QT_DEBUG
//    tcradio_getHdrLot();
#endif
}

void RadioInterface::tcradiocui_setRadioConfig(stRADIO_CONFIG_t *config) {
    TC_RADIO_PRINTF("");
	config->area = eRADIO_CONF_AREA_NA;
	if(currentBand == eRADIO_MW_BAND) {
		config->initMode = eRADIO_AM_MODE;
	}
	else {
		config->initMode = eRADIO_FM_MODE;
	}
	config->initFreq = currentFrequency;

//	config->numTuners = eRADIO_CONF_TYPE_TRIPLE;
	config->numTuners = eRADIO_CONF_TYPE_DUAL;
//	config->numTuners = eRADIO_CONF_TYPE_SINGLE;

    config->fPhaseDiversity = NO;
	config->fIqOut = YES;
	config->audioSamplerate = RADIO_AUDIO_SAMPLERATE;
#ifdef USE_EXTERNAL_CONTROL
	config->fExtAppCtrl = 1;
#else
	if(isRadio)
	{
		config->fExtAppCtrl = 0;
	}
	else
	{
		config->fExtAppCtrl = 1;
	}
#endif
#ifdef USE_HDRADIO
	config->sdr = eRADIO_SDR_HD;
	if(config->numTuners == eRADIO_CONF_TYPE_DUAL) {
	//	config->hdType = eRADIO_HD_TYPE_HD1p0_MRC;
		config->hdType = eRADIO_HD_TYPE_HD1p5;
	}
	else if(config->numTuners == eRADIO_CONF_TYPE_TRIPLE) {
		config->hdType = eRADIO_HD_TYPE_HD1p5_MRC;
	}
	else {
		config->hdType = eRADIO_HD_TYPE_HD1p0;
	}
#else
	config->sdr = eRADIO_SDR_NONE;
#endif
}


void RadioInterface::callbackNotification(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode) {
    TC_RADIO_PRINTF("notifyID: %d, errorCode: %d", notifyID, errorCode);
#ifndef USE_HDRADIO
	(void)pData;
#endif

    switch (notifyID) {
    case eRADIO_NOTIFY_OPEN:
        if (errorCode == eRET_OK)
		{
            TC_RADIO_PRINTF("Nofity opened band: %d, freq: %d", arg[0], arg[1]);
#ifndef USE_HDRADIO
			emit tunerOpened();
#endif
        }
		else
		{
            TC_RADIO_PRINTF("Notify open failed: %d", errorCode);
        }
        break;

    case eRADIO_NOTIFY_SEEK_MODE:
        if (errorCode == eRET_OK) {
            TC_RADIO_PRINTF("Nofity seek band: %d, freq: %d, mode: %d", arg[0], arg[1], arg[2]);
	     currentFrequency = arg[1];
            if(arg[0] == eRADIO_FM_MODE)
                fmFrequency = currentFrequency = arg[1];
            else if(arg[0] == eRADIO_AM_MODE)
                mwFrequency = currentFrequency = arg[1];
            emit frequencyChanged(currentFrequency);

            if (arg[2] == 3 || arg[2] == 4 || arg[2] == 5) {
				if(arg[3] == eRADIO_TYPE1) {
					stRADIO_TYPE1_QUALITY_t qvalue;
					memcpy((void *)&qvalue, (void *)(arg+4), sizeof(qvalue));

	                if (arg[0] == eRADIO_AM_MODE) {
	                    TC_RADIO_PRINTF("RSSI: %d, SNR: %d, MOD: %d, OFFSET: %d", 
							qvalue.am.Rssi, qvalue.am.Snr, qvalue.am.Mod, qvalue.am.Offs);
	                } else {
	                    TC_RADIO_PRINTF("RSSI: %d, SNR: %d, DEV: %d, OFFSET: %d, PILOT: %d, MULTPATH: %d, USN: %d",
							qvalue.fm.Rssi, qvalue.fm.Snr, qvalue.fm.Dev, qvalue.fm.Offs, qvalue.fm.Pilot, qvalue.fm.Mpth, qvalue.fm.Usn);
	                }
				}
				else if(arg[3] == eRADIO_TYPE2) {
					stRADIO_TYPE2_QUALITY_t qvalue;
					memcpy((void *)&qvalue, (void *)(arg+4), sizeof(qvalue));
					
					if (arg[0] == eRADIO_AM_MODE) {
	                    TC_RADIO_PRINTF("STATUS: %xh, RSSI: %d, HFN: %d, COCH: %d, OFFSET: %d, BWTH: %d, MOD: %d",
							qvalue.am.Status, qvalue.am.Rssi, qvalue.am.Hfn, qvalue.am.Coch, qvalue.am.Offs, qvalue.am.Bwth, qvalue.am.Mod);
	                } else {
	                    TC_RADIO_PRINTF("STATUS: %xh, RSSI: %d, USN: %d, MULTPATH: %d, OFFSET: %d, BWTH: %d, MOD: %d",
							qvalue.fm.Status, qvalue.fm.Rssi, qvalue.fm.Usn, qvalue.fm.Mpth, qvalue.fm.Offs, qvalue.fm.Bwth, qvalue.fm.Mod);
	                }
				}
				else {
					stRADIO_TYPE0_QUALITY_t qvalue;
					memcpy((void *)&qvalue, (void *)(arg+4), sizeof(qvalue));
					
					if (arg[0] == eRADIO_AM_MODE) {
	                    TC_RADIO_PRINTF("RSSI: %d, MOD: %d, OFFSET: %d", qvalue.am.Rssi, qvalue.am.Mod, qvalue.am.Offs);
	                } else {
	                    TC_RADIO_PRINTF("RSSI: %d, MOD: %d, OFFSET: %d, HFN: %d, MULTPATH: %d, PILOT: %d", 
							qvalue.fm.Rssi, qvalue.fm.Mod, qvalue.fm.Offs, qvalue.fm.Hfn, qvalue.fm.Mpth, qvalue.fm.Pilot);
	                }
				}
            }

			if(arg[2] == 0) {
				if(arg[3] == 0) {
					RAPP_DBG("Not seek or Can't found radio station!!! [%d]\n", arg[3]);
				}
				else {
					RAPP_DBG("Found radio station!!! [%d]\n", arg[3]);
				}
			}
        } 
		else {
            TC_RADIO_PRINTF("Notify seek failed: %d, band: %d, freq: %d, mode: %d", errorCode, arg[0], arg[1], arg[2]);
        }
        break;

    case eRADIO_NOTIFY_TUNE:
        if (errorCode == eRET_OK) {
            TC_RADIO_PRINTF("Nofity frequency band: %d, freq: %d, mode: %d, ntuner: %d", arg[0], arg[1], arg[2], arg[3]);
			if(arg[3] == eRADIO_ID_PRIMARY) {
				unsigned int fChgBand=0, fChgFreq=0;

				if(arg[0] != currentBand) {
					fChgBand = 1;
				}

				if(arg[1] != currentFrequency) {
					fChgFreq = 1;
				}
				if(arg[0] == eRADIO_FM_MODE) {
					// If you use OIRT, more conditionals will be needed.
					currentBand = eRADIO_FM_BAND;
					fmFrequency = currentFrequency = arg[1];
				}
				else if(arg[0] == eRADIO_AM_MODE) {
					// If you use SW or LW, more conditionals will be needed.
					currentBand = eRADIO_MW_BAND;
					mwFrequency = currentFrequency = arg[1];
				}
				else {
					;
				}
				if(fChgBand || fChgFreq) {
#ifdef USE_HDRADIO
					stTC_HDR_PSD_FORM_t hdrPsd;
					stTC_HDR_SIS_t hdrSis;
					memset(hdrPsd.data, 0 , sizeof(hdrPsd.data));
					memset(hdrSis.shortName.text, 0, sizeof(hdrSis.shortName.text));
					emit hdrTitleChanged((char*)hdrPsd.data);
					emit hdrArtistChanged((char*)hdrPsd.data);
					emit hdrAlbumChanged((char*)hdrPsd.data);
					emit hdrStationShortNameChanged((char*)hdrSis.shortName.text);
					emit hdrProgramTypeChanged(0);
#endif
				}
			}
            emit frequencyChanged(currentFrequency);
        }
		else {
            TC_RADIO_PRINTF("Notify frequency failed: %d", errorCode);
        }
		tcradio_setAudio(ON);
        break;

#ifdef USE_HDRADIO
	/****************************/
	//	HD Radio Notifications	//
	/****************************/
		case eRADIO_HD_NOTIFY_OPEN:
			if(errorCode != eRET_OK) {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_OPEN : Error[%d]!\n", errorCode);
			}
			else {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_OPEN : [%d] Done!\n", arg[0]);
				emit hdrTunerOpened();
			}
			break;

		case eRADIO_HD_NOTIFY_AUDIO_MODE:
			if(errorCode != eRET_OK) {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_AUDIO_MODE : Error[%d]!\n", errorCode);
			}
			else {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_AUDIO_MODE : [%d] Done!\n", arg[0]);
			}
			break;
	
		case eRADIO_HD_NOTIFY_PROGRAM:
			if(errorCode != eRET_OK) {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_PROGRAM : Error[%d]!\n", errorCode);
			}
			else {
				currentProgram = arg[0];
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_PROGRAM : ID[%d], Program[%d] Done!\n", arg[0], arg[1]);
			}
			break;
			
		case eRADIO_HD_NOTIFY_PSD:
			if(errorCode != eRET_OK) {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_PSD : Error[%d]!\n", errorCode);
			}
			else {
				if(*(pData+0) != NULL) {
					unsigned int type = 0;
					stTC_HDR_PSD_t hdrPsd;
					memcpy((void*)&hdrPsd, *pData, sizeof(stTC_HDR_PSD_t));
					TC_RADIO_PRINTF("-----------------------------------------------\n");
					TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_PSD: HDR ID[%d], PN[%02d]\n", arg[0], arg[1]);
					for(type=eTC_HDR_PSD_TITLE; type<eTC_HDR_PSD_MAX; type++) {
						stTC_HDR_PSD_FORM_t *tempPsdData;
						switch(type) {
							case eTC_HDR_PSD_TITLE:						tempPsdData = &hdrPsd.title;
																		emit hdrTitleChanged((char*)hdrPsd.title.data);	break;
							case eTC_HDR_PSD_ARTIST:					tempPsdData = &hdrPsd.artist;
																		emit hdrArtistChanged((char*)hdrPsd.artist.data);	break;
							case eTC_HDR_PSD_ALBUM:						tempPsdData = &hdrPsd.album;
																		emit hdrAlbumChanged((char*)hdrPsd.album.data);	break;
							case eTC_HDR_PSD_GENRE:						tempPsdData = &hdrPsd.genre;					break;
							case eTC_HDR_PSD_COMMENT_LANGUAGE:			tempPsdData = &hdrPsd.comment.language;			break;
							case eTC_HDR_PSD_COMMENT_SHORT_CONTENT:		tempPsdData = &hdrPsd.comment.shortContent;		break;
							case eTC_HDR_PSD_COMMENT_ACTUAL_TEXT:		tempPsdData = &hdrPsd.comment.actualText;		break;
							case eTC_HDR_PSD_COMMERCIAL_PRICE_STRING:	tempPsdData = &hdrPsd.commercial.priceString;	break;
							case eTC_HDR_PSD_COMMERCIAL_VALID_UNTIL:	tempPsdData = &hdrPsd.commercial.validUntil;	break;
							case eTC_HDR_PSD_COMMERCIAL_CONTACT_URL:	tempPsdData = &hdrPsd.commercial.contactURL;	break;
							case eTC_HDR_PSD_COMMERCIAL_RECEIVED_AS:	tempPsdData = &hdrPsd.commercial.receivedAs;	break;
							case eTC_HDR_PSD_COMMERCIAL_SELLER_NAME:	tempPsdData = &hdrPsd.commercial.sellerName;	break;
							case eTC_HDR_PSD_COMMERCIAL_DESCRIPTION:	tempPsdData = &hdrPsd.commercial.description;	break;
						}
						if(tempPsdData->len > 0) {
							int i;
							TC_RADIO_PRINTF("PsdType[%d], CharType[0x%02x] DataLength[%d]\n", type, tempPsdData->charType, tempPsdData->len);
							TC_RADIO_PRINTF("");
							if(tempPsdData->len > 0) {
								for(i=0; i<(int)tempPsdData->len; i++) {
									printf("%c", tempPsdData->data[i]);
								}
								printf("\n");
							}
						}
					}
				}
			}
			break;

		case eRADIO_HD_NOTIFY_SIS:
			if(errorCode != eRET_OK) {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_SIS : Error[%d]!\n", errorCode);
			}
			else {
					if(*(pData+0) != NULL) {
					int i;
					stTC_HDR_SIS_t sisData;
					memcpy((void*)&sisData, *pData, sizeof(stTC_HDR_SIS_t));
						
					TC_RADIO_PRINTF("---------------------------------------------------------\n");
					TC_RADIO_PRINTF("Station Information Service Data: HDR ID[%d]\n", arg[0]);
					
					TC_RADIO_PRINTF("Station ID[%04xh]\n", sisData.stationID.all);
					TC_RADIO_PRINTF("Short Name Length[%d]\n", sisData.shortName.len);
					TC_RADIO_PRINTF("Short Name : ");
					if(sisData.shortName.len > 0) {
						emit hdrStationShortNameChanged((char*)sisData.shortName.text);
						for(i=0; i<(int)sisData.shortName.len; i++) {
							printf("%c", sisData.shortName.text[i]);
						}
					}
					printf("\n");

					TC_RADIO_PRINTF("Universal Name Length[%d], charType[%d], appendFm[%d]\n", sisData.universalName.len, sisData.universalName.charType, sisData.universalName.appendFm);
					TC_RADIO_PRINTF("Universal Name : ");
					if(sisData.universalName.len > 0) {
						for(i=0; i<(int)sisData.universalName.len; i++) {
							printf("%c", sisData.universalName.text[i]);
						}
					}
					printf("\n");
					
					TC_RADIO_PRINTF("Slogan Length[%d], charType[%d]\n", sisData.slogan.len, sisData.slogan.charType);
					TC_RADIO_PRINTF("Slogan : ");
					if(sisData.slogan.len > 0) {
						for(i=0; i<(int)sisData.slogan.len; i++) {
							printf("%c", sisData.slogan.text[i]);
						}
					}
					printf("\n");
				}
			}
			break;

		case eRADIO_HD_NOTIFY_SIGNAL_STATUS:
			if(errorCode != eRET_OK) {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_SIGNAL_STATUS : Error[%d]!\n", errorCode);
			}
			else {
				stTC_HDR_SIGNAL_STATUS_t hdrSigStatus;
				memcpy((void*)&hdrSigStatus, *(pData+0), sizeof(stTC_HDR_SIGNAL_STATUS_t));
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_SIGNAL_STATUS : ID[%d], ALL[%02xh], HDSIG[%d], HDAUD[%d], SIS[%d], SISOK[%d] Done!\n",
					hdrSigStatus.hdrID, hdrSigStatus.acqStatus, hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_SIGNAL, hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_AUDIO,
					hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_SIS, hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_SIS_OK);
				if(hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_SIGNAL && !(hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_AUDIO)) {
					emit hdrAvailableProgramChanged(0);
					emit hdrStatusChanged(1);
				}
				else if(hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_SIGNAL && hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_AUDIO) {
				#if 0
					int ret=0;
					unsigned char bitmask;
					ret = tchdr_getAvailablePrograms(&bitmask);
					if(ret == 0) {
						emit hdrAvailableProgramChanged((int)bitmask);
					}
				#else
					emit hdrAvailableProgramChanged(hdrSigStatus.pmap);
				#endif
					emit hdrStatusChanged(3);
				}
				else {
					emit hdrAvailableProgramChanged(0);
					emit hdrStatusChanged(0);
				}
			}
			break;

		case eRADIO_HD_NOTIFY_PTY:
			if(errorCode != eRET_OK) {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_PTY : Error[%d]!\n", errorCode);
			}
			else {
				TC_RADIO_PRINTF("eRADIO_HD_NOTIFY_PTY : ID[%d], Current ProgNum[%d], PTY[%d]\n", arg[0], arg[1], arg[2]);
				emit hdrProgramTypeChanged((int)arg[2]);
			}
			break;
#endif	// #define USE_HDRADIO

    default:
        TC_RADIO_PRINTF("Unknown notify: %d", notifyID);
        break;
    }
}

void RadioInterface::callbackStation(uint32 totalnum, void * list, int32 errorCode) {
    TC_RADIO_PRINTF("totalnum: %d, errorCode: %d", totalnum, errorCode);

    if (currentBand == eRADIO_FM_BAND) {
       fmStationCount = totalnum;
       fmStationList = (stRADIO_LIST_t*) list;
    } else {
		mwStationCount = totalnum;
        mwStationList = (stRADIO_LIST_t*) list;
    }
    emit stationListChanged(totalnum);

    for (uint32 i = 0; i < totalnum; ++i) {
		if (currentBand == eRADIO_FM_BAND) {	// FM
			if((fmStationList+i)->stQdata.type == eRADIO_TYPE1) {
				stRADIO_TYPE1_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&(fmStationList+i)->stQdata.qual), sizeof(qvalue));
				TC_RADIO_PRINTF("Station %d: %d MHz", i+1, (fmStationList+i)->uiFreq);
	            TC_RADIO_PRINTF("RSSI: %d, SNR: %d, DEV: %d, OFFSET: %d, PILOT: %d, MULTPATH: %d, USN: %d",
	                qvalue.fm.Rssi, qvalue.fm.Snr, qvalue.fm.Dev, qvalue.fm.Offs, qvalue.fm.Pilot, qvalue.fm.Mpth, qvalue.fm.Usn);
			}
			else if((fmStationList+i)->stQdata.type == eRADIO_TYPE2) {
				stRADIO_TYPE2_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&(fmStationList+i)->stQdata.qual), sizeof(qvalue));
				TC_RADIO_PRINTF("Station %d: %d MHz", i+1, (fmStationList+i)->uiFreq);
	            TC_RADIO_PRINTF("STATUS: %xh, RSSI: %d, USN: %d, MULTPATH: %d, OFFSET: %d, BWTH: %d, MOD: %d",
	                qvalue.fm.Status, qvalue.fm.Rssi, qvalue.fm.Usn, qvalue.fm.Mpth, qvalue.fm.Offs, qvalue.fm.Bwth, qvalue.fm.Mod);
			}
			else {
				stRADIO_TYPE0_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&(fmStationList+i)->stQdata.qual), sizeof(qvalue));
				TC_RADIO_PRINTF("Station %d: %d MHz", i+1, (fmStationList+i)->uiFreq);
	            TC_RADIO_PRINTF("RSSI: %d, MOD: %d, OFFSET: %d, HFN: %d, MULTPATH: %d, PILOT: %d",
	                qvalue.fm.Rssi, qvalue.fm.Mod, qvalue.fm.Offs, qvalue.fm.Hfn, qvalue.fm.Mpth, qvalue.fm.Pilot);
			}
		}
		else {	// AM
			if((mwStationList+i)->stQdata.type == eRADIO_TYPE1) {
				stRADIO_TYPE1_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&(mwStationList+i)->stQdata.qual), sizeof(qvalue));
				TC_RADIO_PRINTF("Station %d: %d KHz", i+1, (mwStationList+i)->uiFreq);
	            TC_RADIO_PRINTF("RSSI: %d, SNR: %d, MOD: %d, OFFSET: %d",
	                qvalue.am.Rssi, qvalue.am.Snr, qvalue.am.Mod, qvalue.am.Offs);
			}
			else if((mwStationList+i)->stQdata.type == eRADIO_TYPE2) {
				stRADIO_TYPE2_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&(mwStationList+i)->stQdata.qual), sizeof(qvalue));
				TC_RADIO_PRINTF("Station %d: %d KHz", i+1, (mwStationList+i)->uiFreq);
	            TC_RADIO_PRINTF("STATUS: %xh, RSSI: %d, HFN: %d, COCH: %d, OFFSET: %d, BWTH: %d, MOD: %d",
	                qvalue.am.Status, qvalue.am.Rssi, qvalue.am.Hfn, qvalue.am.Coch, qvalue.am.Offs, qvalue.am.Bwth, qvalue.am.Mod);
			}
			else {
				stRADIO_TYPE0_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&(mwStationList+i)->stQdata.qual), sizeof(qvalue));
				TC_RADIO_PRINTF("Station %d: %d KHz", i+1, (mwStationList+i)->uiFreq);
	            TC_RADIO_PRINTF("RSSI: %d, MOD: %d, OFFSET: %d", qvalue.am.Rssi, qvalue.am.Mod, qvalue.am.Offs);
			}
			
			
		}
    }
}

int RadioInterface::precheckSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata)
{
	int32 ret=-1, rssi=0;
	(void)mod_mode;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));
		
		if(currentBand == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 20)
			{
				ret = 0;
			}
		}
		else if(currentBand == eRADIO_MW_BAND) {
			rssi = (int32)temp_qdata.am.Rssi;
			if(rssi >= 38)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}
	}
	else if(qdata.type == eRADIO_TYPE2)
	{
		stRADIO_TYPE2_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));
		
		if(currentBand == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 280)
			{
				ret = 0;
			}
		}
		else if(currentBand == eRADIO_MW_BAND){
			rssi = (int32)temp_qdata.am.Rssi;
			if(rssi >= 630)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}
	}

	return ret;
}

int RadioInterface::checkSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata)
{
	int32 ret=-1;
	int32 rssi=0, snr=0, offs=0;
	uint32 usn=0, mpth=0, noise=0;
	(void)mod_mode;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));
		
		if(currentBand == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			snr = (int32)temp_qdata.fm.Snr;
			offs = (int32)temp_qdata.fm.Offs;
			if(rssi >= 20 && snr > 4 && offs > -6 && offs < 6)
			{
				ret = 0;
			}
		}
		else if(currentBand == eRADIO_MW_BAND) {
			rssi = (int32)temp_qdata.am.Rssi;
			snr = (int32)temp_qdata.am.Snr;
			offs = (int32)temp_qdata.am.Offs;
			if(rssi >= 38 && snr > 6 && offs > -6 && offs < 6)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}	
	}
	else if(qdata.type == eRADIO_TYPE2)
	{
		stRADIO_TYPE2_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));
		
		if(currentBand == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			offs = (int32)temp_qdata.fm.Offs;
			usn = temp_qdata.fm.Usn;
			mpth = temp_qdata.fm.Mpth;
			if(rssi >= 280 && offs > -100 && offs < 100 && usn < 120 && mpth < 200)
			{
				ret = 0;
			}
		}
		else if(currentBand == eRADIO_MW_BAND) {
			rssi = (int32)temp_qdata.am.Rssi;
			offs = (int32)temp_qdata.am.Offs;
			noise = temp_qdata.am.Hfn;
			if(rssi >= 630 && offs > -50 && offs < 50 && noise < 100)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}
	}

	return ret;
}
