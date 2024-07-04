#ifndef QMLINTERFACE_H
#define QMLINTERFACE_H

#include <QtCore/QObject>

#include "DBusInterface.h"
#include "RadioInterface.h"

#include "PresetType.h"


#define MAX_PRESET_COUNT    18


class QMLInterface : public QObject {
    Q_OBJECT

    Q_PROPERTY(int frequency READ frequency NOTIFY frequencyChanged)
    Q_PROPERTY(int presetNo READ presetNo NOTIFY presetNoChanged)
    Q_PROPERTY(int band READ band NOTIFY bandChanged)
//    Q_PROPERTY(int bandEnable READ bandEnable NOTIFY bandEnableChanged)

	Q_PROPERTY(QString staionShortName READ staionShortName NOTIFY stationShortNameChanged)
	Q_PROPERTY(QString metaProgramType READ metaProgramType NOTIFY metaProgramTypeChanged)
	Q_PROPERTY(QString metaTitle READ metaTitle NOTIFY metaTitleChanged)
    Q_PROPERTY(QString metaArtist READ metaArtist NOTIFY metaArtistChanged)
    Q_PROPERTY(QString metaAlbum READ metaAlbum NOTIFY metaAlbumChanged)

	Q_PROPERTY(int hdrStatus READ hdrStatus NOTIFY metaHdrStatusChanged)
	Q_PROPERTY(int hdrAvailableProgram READ hdrAvailableProgram NOTIFY metaHdrAvailableProgramChanged)

public:
    explicit QMLInterface(QObject* parent = 0);
    ~QMLInterface();

    void connectDBusInterface(DBusInterface*);
    void connectRadioInterface(RadioInterface*);
    void sendInitInfo(void);

	Q_INVOKABLE void backToApps(void);
    Q_INVOKABLE void setBandFM(bool);
    Q_INVOKABLE void setFrequencyInt(int);
    Q_INVOKABLE void setFrequency(QString);

    Q_INVOKABLE void autoScan(bool);
    Q_INVOKABLE void stopScan();

    Q_INVOKABLE void seekAutoUp();
    Q_INVOKABLE void seekAutoDown();
    Q_INVOKABLE void seekManualUp();
    Q_INVOKABLE void seekManualDown();

	Q_INVOKABLE void setHdrEnable(int onoff);
	Q_INVOKABLE void setHdrAudio(int audio);
	Q_INVOKABLE void setHdrService(int service);
	Q_INVOKABLE void setHdrLotOpen(int service, int port);
	Q_INVOKABLE void setHdrLotFlush(int service, int port);
	Q_INVOKABLE void getHdrPsd(int prgno);
	Q_INVOKABLE void getHdrSis();
	Q_INVOKABLE void getHdrSig();
	Q_INVOKABLE void getHdrStatus();
	Q_INVOKABLE void getHdrAlert();
	Q_INVOKABLE void getHdrLot();

    Q_INVOKABLE QVariantList getStationList() const;

    Q_INVOKABLE QVariantMap getPresetData(int) const;
    Q_INVOKABLE void savePreset(int);

    Q_INVOKABLE bool isInvalidFrequency(QString) const;
    Q_INVOKABLE bool isCurrentBandFM() const;
    Q_INVOKABLE bool latestDab();

    Q_INVOKABLE void setLatestDab(bool);
    Q_INVOKABLE void setPresetScan(bool);
    Q_INVOKABLE bool getPresetScan() const;
    Q_INVOKABLE bool getBandEnable() const;

    int frequency() const;
    int presetNo() const;
	int hdrStatus() const;
	int hdrAvailableProgram() const;

    bool band() const;

	QString staionShortName() const;
	QString metaProgramType() const;
	QString metaTitle() const;
    QString metaArtist() const;
    QString metaAlbum() const;

	QString getProgramTypeMapping(int) const;

public slots:
    // Slots for DBus
    void onChangedMode(const char* mode);
	void onReleaseResources(int);
	void onSeekPIList(unsigned int *, unsigned int);
	void onChangeType(void);
	void onSelectPreset(int idx);
	void onKeyboardClicked(int);
	void onStationShortNameChanged(char* shortname);
	void onProgramTypeChanged(int pty);
	void onMetaTitleChanged(char*);
	void onMetaArtistChanged(char*);
	void onMetaAlbumChanged(char*);

    // Slots for radio API
	void onTunerOpened(void);
	void onTunerClosed(int);
    void onFrequencyChanged(int);
	void onTunerExtOpen(void);
	void onTunerExtClose(void);
    void onStationChanged(int);
	void onHdrTunerOpened(void);
	void onHdrStatusChanged(int);
	void onHdrAvailableProgramChanged(int);


signals:
    void showApplication();
    void hideApplication();
	void moveToBack();
    void exitApplication();
	void sourceChange();
    void frequencyChanged(int freq);
    void presetNoChanged(int no);
    void stationChanged(int count);
    void bandChanged();
	void stationShortNameChanged(QString shortname);
	void metaProgramTypeChanged(QString mpty);
	void metaTitleChanged(QString mtitle);
    void metaArtistChanged(QString martist);
    void metaAlbumChanged(QString malbum);
	void metaHdrStatusChanged(int status);
	void metaHdrAvailableProgramChanged(int program);

private:
    void addStationToPreset(int);


private:
    DBusInterface* dbus;
    RadioInterface* radio;

	QString stationShortLabel;
    QString mtitleLabel;
    QString martistLabel;
    QString malbumLabel;
    QString mprogramType;

	int mHdrStatus = 0;
	int mHdrAvailableProgramBitMask = 0;
	bool isKeyEnable = false;
	bool isTunerClose = false;
	bool isRadioEnable = false;
	bool isAudioOutput = false;
	bool isHdrEnable = false;
    bool isPresetScan;
    bool latestDabValue=false;
    bool bandButtonEnable=true;

    PresetType mwPresetData[MAX_PRESET_COUNT];
    PresetType fmPresetData[MAX_PRESET_COUNT];
};

#endif // QMLINTERFACE_H
