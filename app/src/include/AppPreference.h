#ifndef APPPREFERENCE_H
#define APPPREFERENCE_H

#include <QSettings>

#include "PresetType.h"

class AppPreference {
public:
    // Setter

    static void setLatestBand(bool fm);
    static void setCurrentDabState(bool latestDab);
    static void setCurrentAMFrequency(int frequency);
    static void setCurrentFMFrequency(int frequency);

    static void setAMPresetData(int index, const QString name, int frequency);
    static void setFMPresetData(int index, const QString name, int frequency);


    // Getter

    static bool getLatestBand();
    static bool getLatestDab();
    static int getLatestAMFrequency();
    static int getLatestFMFrequency();

    static void getAMPresetData(int index, PresetType* out);
    static void getFMPresetData(int index, PresetType* out);
};

#endif // APPPREFERENCE_H
