#ifndef PRESETTYPE_H
#define PRESETTYPE_H

#include <QtCore/QtCore>

typedef struct {
    QString name;
    unsigned int frequency;

} PresetType;

Q_DECLARE_METATYPE(PresetType)

#endif // PRESETTYPE_H
