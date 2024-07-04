#include "QMLInterface.h"
#include "AppPreference.h"
#include "SleepThread.h"
#include "TCInput.h"
#include "DBusMsgDef.h"

#ifdef QT_DEBUG
#include <QGuiApplication>
#endif

extern int g_appID;

#define RELEASEDISPLAY	0x01
#define RELEASEAUDIO	0x02
#define RELEASETUNER	0x10

QMLInterface::QMLInterface(QObject* parent) : QObject(parent) {
    qDebug("Load preferece --------------");

    isPresetScan = false;

    for (int i = 0; i < 6; ++i) {
        AppPreference::getAMPresetData(i, &mwPresetData[i]);
    }


    for (int i = 0; i < 6; ++i) {
        AppPreference::getFMPresetData(i, &fmPresetData[i]);
    }
}


QMLInterface::~QMLInterface() {
    // TODO
}


void QMLInterface::connectDBusInterface(DBusInterface *interface) {
    dbus = interface;
    connect(dbus, SIGNAL(changedMode(const char*)), this, SLOT(onChangedMode(const char*)));
	connect(dbus, SIGNAL(releaseResources(int)), this, SLOT(onReleaseResources(int)));
    connect(dbus, SIGNAL(radioseekPIList(unsigned int*, unsigned int)), this, SLOT(onSeekPIList(unsigned int*, unsigned int)));

    connect(dbus, SIGNAL(changeType(void)), this, SLOT(onChangeType(void)));
	connect(dbus, SIGNAL(selectPreset(int)), this, SLOT(onSelectPreset(int)));
	connect(dbus, SIGNAL(tunerExtopen(void)), this, SLOT(onTunerExtOpen(void)));
	connect(dbus, SIGNAL(tunerExtclose(void)), this, SLOT(onTunerExtClose(void)));

    connect(dbus, SIGNAL(keyboardClickedEvent(int)), this, SLOT(onKeyboardClicked(int)));
}


void QMLInterface::connectRadioInterface(RadioInterface* interface) {
    radio = interface;
	connect(radio, SIGNAL(tunerOpened()), this, SLOT(onTunerOpened()));
	connect(radio, SIGNAL(tunerClosed(int)), this, SLOT(onTunerClosed(int)));
    connect(radio, SIGNAL(frequencyChanged(int)), this, SLOT(onFrequencyChanged(int)));
    connect(radio, SIGNAL(stationListChanged(int)), this, SLOT(onStationChanged(int)));

	connect(radio, SIGNAL(hdrTunerOpened()), this, SLOT(onHdrTunerOpened()));
	connect(radio, SIGNAL(hdrStationShortNameChanged(char*)), this, SLOT(onStationShortNameChanged(char*)));
	connect(radio, SIGNAL(hdrProgramTypeChanged(int)), this, SLOT(onProgramTypeChanged(int)));
	connect(radio, SIGNAL(hdrTitleChanged(char*)), this, SLOT(onMetaTitleChanged(char*)));
	connect(radio, SIGNAL(hdrArtistChanged(char*)), this, SLOT(onMetaArtistChanged(char*)));
	connect(radio, SIGNAL(hdrAlbumChanged(char*)), this, SLOT(onMetaAlbumChanged(char*)));

	connect(radio, SIGNAL(hdrStatusChanged(int)), this, SLOT(onHdrStatusChanged(int)));
	connect(radio, SIGNAL(hdrAvailableProgramChanged(int)), this, SLOT(onHdrAvailableProgramChanged(int)));
}

void QMLInterface::sendInitInfo(void) {

	int i;
	int amPreset[6];
	int fmPreset[6];
	for(i = 0; i < 6; i++)
	{
		amPreset[i] = mwPresetData[i].frequency;
	}
	for(i = 0; i < 6; i++)
	{
		fmPreset[i] = fmPresetData[i].frequency;
	}
	dbus->sendAMPresetList(amPreset, 6);
	dbus->sendFMPresetList(fmPreset, 6);
	dbus->sendRadioFrequency(radio->getCurrentFrequency());
}

void QMLInterface::backToApps() {
	dbus->changeMode("view", 0);
	emit hideApplication();
	radio->saveStatus();
	isKeyEnable = false;
}

void QMLInterface::setBandFM(bool fm) {
    bandButtonEnable = false;
    if(fm == true) {
		dbus->sendRadioFM();
        radio->switchToFM();
    } else {
		dbus->sendRadioAM();
        radio->switchToAM();
    }
}

void QMLInterface::setLatestDab(bool dabValue) {
    radio->setDabState(dabValue);
}

void QMLInterface::setFrequencyInt(int freq) {
    radio->setFrequency(freq);
}


void QMLInterface::setFrequency(QString text) {
    float freq = text.toFloat();
    if (radio->getCurrentBand() == eRADIO_FM_BAND) {
		radio->setFrequency((int) (freq * 1000));
    } else {
		radio->setFrequency((int) freq);
    }
}


void QMLInterface::autoScan(bool addPreset) {
    isPresetScan = addPreset;

#ifdef QT_DEBUG
    if (radio->getCurrentBand() == eRADIO_FM_BAND) {
        fmPresetData[0].frequency = 101700;
        fmPresetData[0].name = "Sound of free";
        fmPresetData[1].frequency = 104500;
        fmPresetData[1].name = "EBS Educational broadcasting";

        AppPreference::setFMPresetData(0, fmPresetData[0].name, fmPresetData[0].frequency);
        AppPreference::setFMPresetData(1, fmPresetData[1].name, fmPresetData[1].frequency);
        emit stationChanged(2);

    } else {
		mwPresetData[0].frequency = 1143;
        mwPresetData[0].name = "RFK Free Korea Broadcasting";
        mwPresetData[1].frequency = 1530;
        mwPresetData[1].name = "AFN ThunderAM";

        AppPreference::setAMPresetData(0, mwPresetData[0].name, mwPresetData[0].frequency);
        AppPreference::setAMPresetData(1, mwPresetData[1].name, mwPresetData[1].frequency);
        emit stationChanged(2);
    }

#else
    radio->radioSeekList();
#endif
}


void QMLInterface::stopScan() {
    radio->radioSeekStop();
}


void QMLInterface::seekAutoUp() {
    radio->radioSeekAuto(true);
}


void QMLInterface::seekAutoDown() {
    radio->radioSeekAuto(false);
}


void QMLInterface::seekManualUp() {
    radio->radioSeekManual(true);
}


void QMLInterface::seekManualDown() {
    radio->radioSeekManual(false);
}

void QMLInterface::setHdrEnable(int onoff) {
	if(onoff != 0) {
		isHdrEnable = true;
	}
	else {
		isHdrEnable = false;
	}
	radio->setHdrEnable(onoff);
}

void QMLInterface::setHdrAudio(int audio) {
	radio->setHdrAudio(audio);
}

void QMLInterface::setHdrService(int service) {
	radio->setHdrService(service);
}

void QMLInterface::setHdrLotOpen(int service, int port) {
	radio->setHdrLotOpen(service, port);
}

void QMLInterface::setHdrLotFlush(int service, int port) {
	radio->setHdrLotFlush(service, port);
}

void QMLInterface::getHdrPsd(int prgno) {
	radio->getHdrPsd(prgno);
}

void QMLInterface::getHdrSis() {
	radio->getHdrSis();
}

void QMLInterface::getHdrSig() {
	radio->getHdrSig();
}

void QMLInterface::getHdrStatus() {
	radio->getHdrStatus();
}

void QMLInterface::getHdrAlert() {
	radio->getHdrAlert();
}

void QMLInterface::getHdrLot() {
	radio->getHdrLot();
}

QString QMLInterface::getProgramTypeMapping(int pty) const {
	QString type ="";
	switch (pty) {
		case 0:	 type = "None";		break;
		case 1:	 type = "News";		break;
		case 2:	 type = "Affairs";	break;
		case 3:	 type = "Info";		break;
		case 4:	 type = "Sport";	break;
		case 5:	 type = "Educate";	break;
		case 6:	 type = "Drama";	break;
		case 7:	 type = "Culture";	break;
		case 8:	 type = "Science";	break;
		case 9:	 type = "Talk";		break;
		case 10: type = "Pop";		break;
		case 11: type = "Rock";		break;
		case 12: type = "Easy";		break;
		case 13: type = "LightM";	break;
		case 14: type = "Classics";	break;
		case 15: type = "OtherM";	break;
		case 16: type = "Weather";	break;
		case 17: type = "Finance";	break;
		case 18: type = "Children";	break;
		case 19: type = "Social";	break;
		case 20: type = "Religion";	break;
		case 21: type = "PhoneIn";	break;
		case 22: type = "Travel";	break;
		case 23: type = "Leisure";	break;
		case 24: type = "Jazz";		break;
		case 25: type = "Country";	break;
		case 26: type = "NationM";	break;
		case 27: type = "Oldies";	break;
		case 28: type = "Folk";		break;
		case 29: type = "Document";	break;
		case 30: type = "TEST";		break;
		case 31: type = "Alarm!";	break;
		default: type = "";			break;
	}
	return type;
}

QVariantList QMLInterface::getStationList() const {
    QVariantList stations;

    const eRADIO_BAND_t band = radio->getCurrentBand();
    int count = radio->getStationCount(band);
    stRADIO_LIST_t* list = radio->getStationList(band);
    for (int i = 0; i < count; ++i) {
        stRADIO_LIST_t item = list[i];
        QVariantMap map;
        if (band == eRADIO_FM_BAND) {
			map["freq"] = QString("%1 MHz").arg(item.uiFreq * 0.001);
            map["name"] = QString::number(item.uiFreq * 0.001);
        } else {
            map["freq"] = QString("%1 KHz").arg(item.uiFreq);
        }
        map["frequency"] = item.uiFreq;
        stations.append(map);
    }

#ifdef QT_DEBUG
    const PresetType* preset = (band == eRADIO_FM_BAND) ? fmPresetData : mwPresetData;
    for (int i = 0; i < MAX_PRESET_COUNT; ++i) {
        if (preset[i].frequency > 0) {
            QVariantMap map;
            if (band == eRADIO_FM_BAND) {
                map["freq"] = QString("%1 MHz").arg(preset[i].frequency * 0.001);
                map["name"] = QString::number(preset[i].frequency * 0.001);
            } else {
            	map["freq"] = QString("%1 KHz").arg(preset[i].frequency);
            }
            map["frequency"] = preset[i].frequency;
            stations.append(map);
        }
    }
#endif

    return stations;
}


QVariantMap QMLInterface::getPresetData(int no) const {
    QVariantMap map;

    if (radio->getCurrentBand() == eRADIO_FM_BAND) {
		map["name"] = fmPresetData[no].name;
		map["freq"] = fmPresetData[no].frequency;
    } else {
		map["name"] = mwPresetData[no].name;
		map["freq"] = mwPresetData[no].frequency;
    }

    return map;
}


void QMLInterface::savePreset(int index) {
    eRADIO_BAND_t band = radio->getCurrentBand();
    PresetType* preset = (band == eRADIO_FM_BAND) ? fmPresetData : mwPresetData;

    preset[index].frequency = radio->getCurrentFrequency();
    if (band == eRADIO_FM_BAND) {
        preset[index].name = QString::number(radio->getCurrentFrequency() * 0.001);
        AppPreference::setFMPresetData(index, preset[index].name, preset[index].frequency);
    } else {
    	preset[index].name = QString::number(radio->getCurrentFrequency());
        AppPreference::setAMPresetData(index, preset[index].name, preset[index].frequency);
    }

    emit stationChanged(radio->getStationCount(band));
    emit presetNoChanged(index);
	dbus->sendPresetChanged(preset[index].frequency, index);
}


bool QMLInterface::isInvalidFrequency(QString text) const {
    float freq = text.toFloat();
    qDebug() << "freq valid check: " << text << " to " << freq;

    if (radio->getCurrentBand() == eRADIO_FM_BAND) {
         freq *= 1000;
        return ((int)freq < FM_INIT_FREQUENCY || (int)freq > FM_LIMIT_FREQUENCY || (((int)freq-FM_INIT_FREQUENCY)%FM_STEP_FREQUENCY != 0));
    }
	else {
		return ((int)freq < AM_INIT_FREQUENCY || (int)freq > AM_LIMIT_FREQUENCY || (((int)freq-AM_INIT_FREQUENCY)%AM_STEP_FREQUENCY != 0));
    }
}


bool QMLInterface::isCurrentBandFM() const {
    eRADIO_BAND_t band = radio->getCurrentBand();
    return band == eRADIO_FM_BAND;
}

bool QMLInterface::latestDab() {
    return radio->getLatestDab();
}


int QMLInterface::frequency() const {
    return radio->getCurrentFrequency();
}


int QMLInterface::presetNo() const {
    eRADIO_BAND_t band = radio->getCurrentBand();
    const PresetType* preset = (band == eRADIO_FM_BAND) ? fmPresetData : mwPresetData;

    unsigned int freq = radio->getCurrentFrequency();
    for (int i = 0; i < MAX_PRESET_COUNT; ++i) {
        if (preset[i].frequency == freq) {
            return i;
        }
    }
    return -1;
}

int QMLInterface::hdrStatus() const {
	return mHdrStatus;
}

int QMLInterface::hdrAvailableProgram() const {
	return mHdrAvailableProgramBitMask;
}

bool QMLInterface::band() const {
    eRADIO_BAND_t band = radio->getCurrentBand();
    return band == eRADIO_FM_BAND;
}

bool QMLInterface::getBandEnable() const {
	return bandButtonEnable;
}

QString QMLInterface::staionShortName() const {
    return stationShortLabel;
}


QString QMLInterface::metaProgramType() const {
	if(mprogramType == "") {
		return "None";
	}
    return mprogramType;
}


QString QMLInterface::metaTitle() const {
    return mtitleLabel;
}


QString QMLInterface::metaArtist() const {
    return martistLabel;
}


QString QMLInterface::metaAlbum() const {
    return malbumLabel;
}


void QMLInterface::onChangedMode(const char* mode) {
	if(strcmp(mode, "view") == 0)
	{
		emit showApplication();
		isKeyEnable = true;
		dbus->changeMode("audioplay", g_appID);
	}
	else if(strcmp(mode, "audioplay") == 0)
	{
		emit showApplication();
		isKeyEnable = true;
		if(isRadioEnable == false)
		{
			radio->setRadioOpen();
		}
		if(isRadioEnable == true && isAudioOutput == false)
		{
			radio->setRadioPCMOpen();
			radio->setRadioMute(1);
		}
		isAudioOutput = true;
		dbus->sendRadioFrequency(radio->getCurrentFrequency());
	}
	else if(strcmp(mode, "audioplaybg") == 0)
	{
		if(isRadioEnable == false)
		{
			radio->setRadioOpen();
		}
		if(isRadioEnable == true && isAudioOutput == false)
		{
			radio->setRadioPCMOpen();
			radio->setRadioMute(1);
		}
		isAudioOutput = true;
	}
	if (radio->getCurrentBand() == eRADIO_FM_BAND)
	{
		dbus->sendRadioFM();
	}
	else
	{
		dbus->sendRadioAM();
	}
}

void QMLInterface::onReleaseResources(int resources) {
	switch(resources)
	{
		case RELEASEDISPLAY:
			radio->saveStatus();
			emit hideApplication();
			isKeyEnable = false;
			dbus->releaseResourceDone(RELEASEDISPLAY);
			break;
		case RELEASEAUDIO:
			radio->setRadioMute(0);
			radio->setRadioPCMClose();
			isAudioOutput = false;
			mHdrStatus = 0;
			dbus->sendRadioHDR(mHdrStatus);
			dbus->releaseResourceDone(RELEASEAUDIO);
			break;
		case (RELEASEDISPLAY | RELEASEAUDIO):
			emit hideApplication();
			radio->setRadioMute(0);
			radio->setRadioPCMClose();
			isKeyEnable = false;
			isAudioOutput = false;
			radio->saveStatus();
			mHdrStatus = 0;
			dbus->sendRadioHDR(mHdrStatus);
			dbus->releaseResourceDone(RELEASEDISPLAY | RELEASEAUDIO);
			break;
		case RELEASETUNER:
			radio->setRadioClose();
			mHdrStatus = 0;
			dbus->sendRadioHDR(mHdrStatus);
			break;
		case (RELEASETUNER | RELEASEDISPLAY):
			emit hideApplication();
			radio->setRadioClose();
			isKeyEnable = false;
			mHdrStatus = 0;
			dbus->sendRadioHDR(mHdrStatus);
			dbus->releaseResourceDone(RELEASEDISPLAY);
			radio->saveStatus();
			break;
		case (RELEASETUNER | RELEASEAUDIO):
			radio->setRadioMute(0);
			radio->setRadioPCMClose();
			radio->setRadioClose();
			isAudioOutput = false;
			mHdrStatus = 0;
			dbus->sendRadioHDR(mHdrStatus);
			dbus->releaseResourceDone(RELEASEAUDIO);
			break;
		case (RELEASETUNER | RELEASEDISPLAY | RELEASEAUDIO):
			emit hideApplication();
			radio->setRadioMute(0);
			radio->setRadioPCMClose();
			radio->setRadioClose();
			isKeyEnable = false;
			isAudioOutput = false;
			mHdrStatus = 0;
			dbus->sendRadioHDR(mHdrStatus);
			dbus->releaseResourceDone(RELEASEDISPLAY | RELEASEAUDIO);
			radio->saveStatus();
			break;
		default:
			break;
	}
}

void QMLInterface::onSeekPIList(unsigned int* pilist, unsigned int cnt) {
    qDebug("SET PI LIST RECEIVED");
	radio->radioSeekPIList(pilist, cnt);
}

void QMLInterface::onChangeType(void) {
    qDebug("OnClicked Source Change");
    emit sourceChange();
}

void QMLInterface::onSelectPreset(int idx) {
    qDebug("OnClicked Preset ");
    PresetType* preset = (radio->getCurrentBand() == eRADIO_FM_BAND) ? fmPresetData : mwPresetData;
	setFrequencyInt(preset[idx].frequency);
}

void QMLInterface::onTunerExtOpen(void) {
    qDebug("Tuner External Open from DAB");
	radio->setRadioOpen(false);
}

void QMLInterface::onTunerExtClose(void) {
    qDebug("Tuner External Close from DAB");
	radio->setRadioClose();
}

void QMLInterface::onKeyboardClicked(int key)
{
    qDebug("KEY RECEIVED: %d", key);
#ifdef USE_TC_KEY
	if (isKeyEnable)
	{
		if (key == g_knobKeys[TCKeyScan])
		{
    		if (isPresetScan == false) {
        		isPresetScan = true;
			}
    		radio->radioSeekList();
		}
		else if (key == g_knobKeys[TCKeyUp])
		{
    		radio->radioSeekManual(true);
		}
		else if (key == g_knobKeys[TCKeyDown])
		{
    		radio->radioSeekManual(false);
		}
		else if (key == g_knobKeys[TCKeyPower])
		{
			dbus->changeMode("idle", g_appID);
			emit hideApplication();
			radio->setRadioMute(0);
			radio->setRadioPCMClose();
			radio->saveStatus();
			isKeyEnable = false;
			isAudioOutput = false;
		}
		else if (key == g_knobKeys[TCKeyHome])
		{
			dbus->changeMode("home", 0);
			emit hideApplication();
			radio->saveStatus();
			isKeyEnable = false;
		}
		else if (key == g_knobKeys[TCKeyBack])
		{
			emit moveToBack();
		}
	}
	else
	{
		if (key == g_knobKeys[TCKeyRadio])
		{
    		dbus->changeMode("view", g_appID);
		}
		else if (key == g_knobKeys[TCKeyPower])
		{
			if(isAudioOutput == true)
			{
				radio->setRadioMute(0);
				radio->setRadioPCMClose();
				isAudioOutput = false;
				radio->saveStatus();
				dbus->changeMode("idle", g_appID);
			}
		}
	}
#endif
}

void QMLInterface::onTunerOpened(void) {
    qDebug("TUNER OPEN SIGNAL RECEIVED ");
	isRadioEnable = true;
	if(isTunerClose == true)
	{
		radio->setRadioClose();
	}
	else
	{
		if(isAudioOutput == true)
		{
			radio->setRadioPCMOpen();
			radio->setRadioMute(1);
		}
	}
}

void QMLInterface::onTunerClosed(int ret) {
    qDebug("TUNER CLOSED SIGNAL RECEIVED: %d", ret);
	if(ret != eRET_OK)
	{
		isTunerClose = true;
	}
	else
	{
		isRadioEnable = false;
		isTunerClose = false;
		dbus->releaseResourceDone(RELEASETUNER);
	}
}

void QMLInterface::onFrequencyChanged(int freq) {
    qDebug("FREQUENCY CHANGED SIGNAL RECEIVED: %d", freq);
    int preset = presetNo();
    
    emit frequencyChanged(freq);
    emit presetNoChanged(preset);
    emit bandChanged();
 //   emit bandEnableChanged();
    bandButtonEnable = true;
	dbus->sendRadioFrequency(freq);
}


void QMLInterface::onStationChanged(int count) {
    qDebug("STATION CHANGED SIGNAL RECEIVED: %d", count);

    if (isPresetScan) {
        isPresetScan = false;
        addStationToPreset(count);
    }
    emit stationChanged(count);
}

void QMLInterface::onStationShortNameChanged(char* shortname) {
	qDebug("STATION SHORT NAME CHANGED SIGNAL RECEIVED: %s", shortname);
    stationShortLabel = QString::fromLocal8Bit((const char*) shortname);
	emit stationShortNameChanged(stationShortLabel);
}


void QMLInterface::onProgramTypeChanged(int pty) {
    qDebug("PROGRAM TYPE CHANGED SIGNAL RECEIVED: %d", pty);
    mprogramType = getProgramTypeMapping(pty);
    emit metaProgramTypeChanged(mprogramType);
}


void QMLInterface::onMetaTitleChanged(char* mtitle) {
    qDebug("META TITLE CHANGED SIGNAL RECEIVED: %s", mtitle);
    mtitleLabel = QString::fromLocal8Bit((const char*) mtitle);
    emit metaTitleChanged(mtitleLabel);
}


void QMLInterface::onMetaArtistChanged(char* martist) {
    qDebug("META ARTIST CHANGED SIGNAL RECEIVED: %s", martist);
    martistLabel = QString::fromLocal8Bit((const char*) martist);
    emit metaArtistChanged(martistLabel);
}


void QMLInterface::onMetaAlbumChanged(char* malbum) {
    qDebug("META ALBUM CHANGED SIGNAL RECEIVED: %s", malbum);
    malbumLabel = QString::fromLocal8Bit((const char*) malbum);
    emit metaAlbumChanged(malbumLabel);
}

void QMLInterface::onHdrTunerOpened(void) {
    qDebug("HDR TUNER OPEN SIGNAL RECEIVED");
	isRadioEnable = true;
	if(isTunerClose == true)
	{
		radio->setRadioClose();
	}
	else
	{
		if(isAudioOutput == true)
		{
			radio->setRadioPCMOpen();
			radio->setRadioMute(1);
		}
	}

}

void QMLInterface::onHdrStatusChanged(int sts) {
    qDebug("HD Radio Status CHANGED SIGNAL RECEIVED: %d", sts);
	mHdrStatus = sts;
    emit metaHdrStatusChanged(mHdrStatus);
	dbus->sendRadioHDR(mHdrStatus);
}

void QMLInterface::onHdrAvailableProgramChanged(int programBitMask) {
    qDebug("HD Radio Available Program CHANGED SIGNAL RECEIVED: %d", programBitMask);
	mHdrAvailableProgramBitMask = programBitMask;
    emit metaHdrAvailableProgramChanged(mHdrAvailableProgramBitMask);
}

void QMLInterface::addStationToPreset(int count) {
    eRADIO_BAND_t band = radio->getCurrentBand();
    PresetType* preset = (band == eRADIO_FM_BAND) ? fmPresetData : mwPresetData;
    stRADIO_LIST_t* list = radio->getStationList(band);

    for (int i = 0; i < count; ++i) {
	 if(count > MAX_PRESET_COUNT){
	      break;
	 }
         preset[i].frequency = list[i].uiFreq;
         if (band == eRADIO_FM_BAND) {
			preset[i].name = QString::number(list[i].uiFreq * 0.001);
			AppPreference::setFMPresetData(i, preset[i].name, preset[i].frequency);
         }else {
			preset[i].name = QString::number(list[i].uiFreq);
			AppPreference::setAMPresetData(i, preset[i].name, preset[i].frequency);
          }
   }
}
void QMLInterface::setPresetScan(bool addPreset){
    isPresetScan = addPreset;
}

bool QMLInterface::getPresetScan() const{
    return isPresetScan;
}
