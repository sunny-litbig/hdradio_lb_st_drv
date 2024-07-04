#include "AppPreference.h"

#include <QGuiApplication>

#ifdef QT_DEBUG
#define PREF_PATH       "radio/preference.ini"
#else
//#define PREF_PATH       "/usr/share/tc-radio/preference.ini"
#define PREF_PATH       "/home/root/.telechips/radio/preference.ini"
#endif

#define LATEST_DAB      "latestDab"
#define LATEST_BAND     "latestBand"
#define AM_FREQUENCY    "mwFrequency"
#define FM_FREQUENCY    "fmFrequency"
#define PRESET_AM_NAME  "amPresetName_%1"
#define PRESET_AM_FREQ  "amPresetFreq_%1"
#define PRESET_FM_NAME  "fmPresetName_%1"
#define PRESET_FM_FREQ  "fmPresetFreq_%1"

void AppPreference::setLatestBand(bool fm) {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    pref.setValue(LATEST_BAND, fm);
    pref.sync();
    system("sync");
}
void AppPreference::setCurrentDabState(bool latestDab){
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    pref.setValue(LATEST_DAB, latestDab);
    pref.sync();
    system("sync");
}
void AppPreference::setCurrentAMFrequency(int frequency) {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    pref.setValue(AM_FREQUENCY, frequency);
    pref.sync();
    system("sync");
}


void AppPreference::setCurrentFMFrequency(int frequency) {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    pref.setValue(FM_FREQUENCY, frequency);
    pref.sync();
    system("sync");
    //QProcess::execute("sync");
}


void AppPreference::setAMPresetData(int index, const QString name, int frequency) {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    QString nameKey = QString(PRESET_AM_NAME).arg(index);
    QString freqKey = QString(PRESET_AM_FREQ).arg(index);
    pref.setValue(nameKey, name);
    pref.setValue(freqKey, frequency);
    pref.sync();
    system("sync");
}


void AppPreference::setFMPresetData(int index, const QString name, int frequency) {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    QString nameKey = QString(PRESET_FM_NAME).arg(index);
    QString freqKey = QString(PRESET_FM_FREQ).arg(index);
    pref.setValue(nameKey, name);
    pref.setValue(freqKey, frequency);
    pref.sync();
    system("sync");
}


bool AppPreference::getLatestBand() {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    return pref.value(LATEST_BAND, true).toBool();
}


bool AppPreference::getLatestDab() {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    return pref.value(LATEST_DAB, true).toBool();
}



int AppPreference::getLatestAMFrequency() {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    return pref.value(AM_FREQUENCY, 530).toInt();
}


int AppPreference::getLatestFMFrequency() {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    return pref.value(FM_FREQUENCY, 87500).toInt();
}


void AppPreference::getAMPresetData(int index, PresetType *out) {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    QString nameKey = QString(PRESET_AM_NAME).arg(index);
    QString freqKey = QString(PRESET_AM_FREQ).arg(index);
    out->name = pref.value(nameKey, "").toString();
    out->frequency = pref.value(freqKey, 0).toInt();
}


void AppPreference::getFMPresetData(int index, PresetType *out) {
    QSettings pref(PREF_PATH, QSettings::IniFormat);
    QString nameKey = QString(PRESET_FM_NAME).arg(index);
    QString freqKey = QString(PRESET_FM_FREQ).arg(index);
    out->name = pref.value(nameKey, "").toString();
    out->frequency = pref.value(freqKey, 0).toInt();
}
