#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QtCore/QObject>
#include <QTimer>

#include "DBusMsgDef.h"

#define TC_IF_MANAGER DBusInterface::getInterface()

class DBusInterface : public QObject
{
    Q_OBJECT

public:
    DBusInterface();
    ~DBusInterface();

    static DBusInterface* getInterface();

	void changeMode(const char* mode, int app);
	void releaseResourceDone(int resources);
	void seekPIList(unsigned int *pilist, unsigned int cnt);
	void sendAMPresetList(int *preset, int cnt);
	void sendFMPresetList(int *preset, int cnt);
	void sendPresetChanged(int freq, int index);
	void sendRadioAM();
	void sendRadioFM();
	void sendRadioHDR(int sts);
	void sendRadioFrequency(int freq);

public slots:
    void onKeyboardPressedEvent(int);
    void onKeyboardLongPressedEvent(int);
    void onKeyboardLongLongPressedEvent(int);
    void onKeyboardReleasedEvent(int);
    void onKeyboardClickedEvent(int);

	void onChangedMode(const char * mode);
	void onReleaseResources(int resources);

	void onChangeType(void);
	void onSelectPreset(int);
	void onTunerExtOpen(void);
	void onTunerExtClose(void);

signals:
    void keyboardPressedEvent(int);
    void keyboardLongPressedEvent(int);
    void keyboardLongLongPressedEvent(int);
    void keyboardReleasedEvent(int);
    void keyboardClickedEvent(int);

	void changedMode(const char*);
	void releaseResources(int);
	void radioseekPIList(unsigned int*, unsigned int);

	void changeType(void);
	void selectPreset(int);
	void tunerExtopen(void);
	void tunerExtclose(void);

private:
};

#endif // DBUSINTERFACE_H
