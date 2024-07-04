TEMPLATE = app
DESTDIR = bin
TARGET = TcRadioApp

QT += qml quick widgets
CONFIG += c++11

QMAKE_STRIP = echo

PKGCONFIG += dbus-1 TcUtils
CONFIG += link_pkgconfig

INCLUDEPATH += \
    ./src/include \

SOURCES += \
    src/main.cpp \
    src/DBusMsgDefNames.c \
    src/TCDBus.cpp \
    src/QMLInterface.cpp \
    src/DBusInterface.cpp \
    src/RadioInterface.cpp \
    src/AppPreference.cpp \

HEADERS += \
    src/include/DBusMsgDef.h \
    src/include/TCDBus.h \
    src/include/QMLInterface.h \
    src/include/DBusInterface.h \
    src/include/RadioInterface.h \
    src/include/PresetType.h \
    src/include/AppPreference.h \
    src/include/SleepThread.h \

RESOURCES += \
    res/qml.qrc

equals(USE_HDRADIO, YES) {
    DEFINES += USE_HDRADIO
}

target.path = /usr/bin

INSTALLS += target

LIBS += -ltcradio

CONFIG(debug, debug|release) { 
INCLUDEPATH += \
        /usr/include/dbus-1.0 \
        /usr/lib/x86_64-linux-gnu/dbus-1.0/include
}else { 
    OBJECTS_DIR = build/release/obj
    MOC_DIR = build/release/moc
    UI_HEADERS_DIR = build/release/ui_header
    INCLUDEPATH += build/release/ui_header
}

QML_IMPORT_PATH =
QML_DESIGNER_IMPORT_PATH =
DEFINES += QT_DEPRECATED_WARNINGS

# Additional import path used to resolve QML modules in Qt Creator's code model
# Additional import path used to resolve QML modules just for Qt Quick Designer

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

