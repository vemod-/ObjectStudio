!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

greaterThan(QT_MAJOR_VERSION, 4): QT += macextras

INCLUDEPATH += /Developer/Extras/CoreAudio/PublicUtility
INCLUDEPATH += $$PWD/../PluginLoader
INCLUDEPATH += $$PWD/../VSTHost
INCLUDEPATH += $$PWD/../../AudioUnit
INCLUDEPATH += $$PWD

LIBS += -framework Cocoa -framework Carbon -framework CoreAudio -framework AudioUnit -framework AudioToolbox -framework CoreAudioKit

SOURCES += \
    ../../../../../../Developer/Extras/CoreAudio/PublicUtility/CAComponentDescription.cpp \
    ../../../../../../Developer/Extras/CoreAudio/PublicUtility/CAComponent.cpp \
    ../../../../../../Developer/Extras/CoreAudio/PublicUtility/CAStreamBasicDescription.cpp \
    $$PWD/caudiounithost.cpp

HEADERS += \
    ../../../../../../Developer/Extras/CoreAudio/PublicUtility/CAComponentDescription.h \
    ../../../../../../Developer/Extras/CoreAudio/PublicUtility/CAComponent.h \
    ../../../../../../Developer/Extras/CoreAudio/PublicUtility/CAStreamBasicDescription.h \
    $$PWD/../../AudioUnit/caudiounitclass.h \
    $$PWD/caudiounithost.h

OBJECTIVE_SOURCES += \
    $$PWD/../../AudioUnit/caudiounitclass.mm
}
