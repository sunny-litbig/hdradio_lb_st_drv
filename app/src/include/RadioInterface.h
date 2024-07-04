#ifndef RADIOINTERFACE_H
#define RADIOINTERFACE_H

#include <QtCore/QtCore>

#include "tcradio_api.h"

#ifdef USE_HDRADIO
#include "tcradio_hdr_if.h"
#endif

#define TC_RADIO_MANAGER RadioInterface::getInterface()

#define FM_INIT_FREQUENCY   87500
#define FM_LIMIT_FREQUENCY  107900
#define	FM_STEP_FREQUENCY	200

#define AM_INIT_FREQUENCY   530
#define AM_LIMIT_FREQUENCY  1710
#define	AM_STEP_FREQUENCY	10

typedef enum {
	eRADIO_FM_BAND	= 0,
	eRADIO_MW_BAND	= 1,
	eRADIO_MAX_BAND
}eRADIO_BAND_t;

class RadioInterface : public QObject {
    Q_OBJECT

public:
    RadioInterface();
    ~RadioInterface();

    static RadioInterface* getInterface();
	int setRadioOpen();
	int setRadioOpen(bool radio);
	int setRadioClose();
    bool getLatestDab() const;
    eRADIO_BAND_t getCurrentBand() const;
    uint32 getCurrentFrequency() const;
    uint32 getStationCount(int band) const;
    stRADIO_LIST_t* getStationList(int band) const;

    void saveStatus();

    void switchToAM();
    void switchToFM();
	void setRadioMute(int onoff);
	void setRadioPCMOpen(void);
	void setRadioPCMClose(void);

    void setDabState(bool);
    void setFrequency(int);
    void radioSeekManual(bool up);
    void radioSeekAuto(bool up);
    void radioSeekList();
    void radioSeekStop();
    void radioSeekPIList(unsigned int *pilist, unsigned int cnt);

    void getRadioFrequency(eRADIO_MOD_MODE_t *mod_mode, unsigned int *freq, int ntuner);
    void getRadioQuiality(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t * qdata, unsigned int ntuner);

	void setHdrEnable(int onoff);
	void setHdrAudio(int audio);
	void setHdrService(int service);
	void setHdrLotOpen(int service, int port);
	void setHdrLotFlush(int service, int port);
	void getHdrPsd(int prgno);
	void getHdrSis();
	void getHdrSig();
	void getHdrStatus();
	void getHdrAlert();
	void getHdrLot();

    void tcradiocui_setRadioConfig(stRADIO_CONFIG_t *config);
    void callbackNotification(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode);
    void callbackStation(uint32 totalnum, void * list, int32 errorCode);
	int precheckSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata);
	int checkSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata);

signals:
	void tunerOpened();
	void tunerClosed(int);
    void frequencyChanged(int);
    void stationListChanged(int);

#ifdef USE_HDRADIO
	void hdrTunerOpened();
	void hdrStatusChanged(int);
	void hdrAvailableProgramChanged(int);
	void hdrStationShortNameChanged(char*);
	void hdrProgramTypeChanged(int);
	void hdrTitleChanged(char*);
	void hdrArtistChanged(char*);
	void hdrAlbumChanged(char*);
#endif

private:
	uint32 currentProgram = 0;
#ifdef USE_HDRADIO
	stTC_HDR_CONF_t hdrAppConf;
#endif
    eRADIO_BAND_t currentBand = eRADIO_FM_BAND;
    uint32 currentFrequency = 0;
    uint32 fmFrequency = 0;
    uint32 mwFrequency = 0;
    bool latestDab;
    bool isRadio;
    stRADIO_CONFIG_t radioConf;

    int mwStationCount = 0;
    int fmStationCount = 0;
    stRADIO_LIST_t* mwStationList;
    stRADIO_LIST_t* fmStationList;
};

#endif // RADIOINTERFACE_H
