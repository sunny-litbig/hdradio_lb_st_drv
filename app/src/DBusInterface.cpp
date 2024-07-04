#include <dbus/dbus.h>
#include "DBusInterface.h"
#include "TCDBus.h"

extern int g_appID;
static DBusInterface* self = nullptr;

DBusInterface::DBusInterface() {
    self = this;
    qDebug("DBus initialized");
    InitilaizeTCDBusManager();

}

DBusInterface::~DBusInterface() {
    ReleaseTCDBusManager();
    self = nullptr;
}

DBusInterface* DBusInterface::getInterface() {
    return self;
}

void DBusInterface::changeMode(const char* mode, int app) {
    qDebug("changeMode in DBUS interface");
	SendDBusChangeMode(mode, app);
}

void DBusInterface::releaseResourceDone(int resources) {
    qDebug("releaseResourceDone in DBUS interface");
	SendDBusReleaseResourceDone(resources, g_appID);
}

void DBusInterface::seekPIList(unsigned int *piList, unsigned int cnt) {
    qDebug("seekPIList in DBUS interface");
	emit radioseekPIList(piList, cnt);
}

void DBusInterface::sendAMPresetList(int *preset, int cnt) {
    qDebug("send AM PresetList in DBUS interface");
	SendDBusAMPresetList(preset, cnt);
}

void DBusInterface::sendFMPresetList(int *preset, int cnt) {
    qDebug("send FM PresetList in DBUS interface");
	SendDBusFMPresetList(preset, cnt);
}

void DBusInterface::sendPresetChanged(int freq, int index) {
    qDebug("sendPresetChanged in DBUS interface");
	SendDBusPresetChanged(freq, index);
}

void DBusInterface::sendRadioAM() {
    qDebug("send AM in DBUS interface");
	SendDBusRadioAM();
}

void DBusInterface::sendRadioFM() {
    qDebug("send FM in DBUS interface");
	SendDBusRadioFM();
}

void DBusInterface::sendRadioHDR(int sts) {
    qDebug("send HDR in DBUS interface");
	SendDBusRadioHDR(sts);
}

void DBusInterface::sendRadioFrequency(int freq) {
    qDebug("sendRadioType in DBUS interface");
	SendDBusRadioFrequency(freq);
}

void DBusInterface::onKeyboardPressedEvent(int value) {
    qDebug("DBUS RECEIVED: Keyboard Pressed Event %d", value);
    emit keyboardPressedEvent(value);
}

void DBusInterface::onKeyboardLongPressedEvent(int value) {
    qDebug("DBUS RECEIVED: Keyboard Long Pressed Event %d", value);
    emit keyboardLongPressedEvent(value);
}

void DBusInterface::onKeyboardLongLongPressedEvent(int value) {
    qDebug("DBUS RECEIVED: Keyboard Long Long Pressed Event %d", value);
    emit keyboardLongLongPressedEvent(value);
}

void DBusInterface::onKeyboardReleasedEvent(int value) {
    qDebug("DBUS RECEIVED: Keyboard Release Event %d", value);
    emit keyboardReleasedEvent(value);
}

void DBusInterface::onKeyboardClickedEvent(int value) {
    qDebug("DBUS RECEIVED: Keyboard Clicked Event %d", value);
    emit keyboardClickedEvent(value);
}

void DBusInterface::onChangedMode(const char* mode)
{
	qDebug("DBUS RECEIVED: Changed : %s", mode);
	emit changedMode(mode);
}

void DBusInterface::onReleaseResources(int resources)
{
	qDebug("DBUS RECEIVED: Release Resource %d", resources);
	emit releaseResources(resources);
}

void DBusInterface::onChangeType(void)
{
	qDebug("DBUS RECEIVED: Change Radio Type");
	emit changeType();
}

void DBusInterface::onSelectPreset(int idx)
{
	qDebug("DBUS RECEIVED: Select Radio index : %d", idx);
	emit selectPreset(idx);
}

void DBusInterface::onTunerExtOpen(void)
{
	qDebug("DBUS RECEIVED: TunerOpen ");
	emit tunerExtopen();
}

void DBusInterface::onTunerExtClose(void)
{
	qDebug("DBUS RECEIVED: TunerOpen ");
	emit tunerExtclose();
}
