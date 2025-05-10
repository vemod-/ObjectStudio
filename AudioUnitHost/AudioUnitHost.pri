!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

greaterThan(QT_MAJOR_VERSION, 4){
        lessThan(QT_MAJOR_VERSION, 6){
                QT += macextras
        }
}

#QMAKE_OBJECTIVE_CFLAGS += -fobjc-arc

##include($$PWD/../../QSignalMenu/QSignalMenu.pri)

INCLUDEPATH += /Library/Developer/Extras/CoreAudio/PublicUtility
INCLUDEPATH += $$PWD/../PluginLoader
INCLUDEPATH += $$PWD/../VSTHost
INCLUDEPATH += $$PWD/../../AudioUnit
INCLUDEPATH += $$PWD

macx {
    LIBS += -framework AppKit
}

ios {
    LIBS += -framwork UIKit
}

ios {
    LIBS += -framework CoreAudio -framework CoreFoundation
    LIBS += -framework AudioToolbox
    LIBS += -framework AVFoundation
}

macx {
    LIBS += -framework CoreAudio -framework CoreFoundation
    LIBS += -framework AudioUnit -framework AudioToolbox -framework CoreAudioKit
}

LIBS += -lSoftSynthsWidgets
INCLUDEPATH += $$PWD/../../QSignalMenu

SOURCES += \
    /Library/Developer/Extras/CoreAudio/PublicUtility/CAComponentDescription.cpp \
    /Library/Developer/Extras/CoreAudio/PublicUtility/CAComponent.cpp \
    $$PWD/caudiounithost.cpp

macx {
SOURCES += \
    /Library/Developer/Extras/CoreAudio/PublicUtility/CAStreamBasicDescription.cpp
}

HEADERS += \
    /Library/Developer/Extras/CoreAudio/PublicUtility/CAComponentDescription.h \
    /Library/Developer/Extras/CoreAudio/PublicUtility/CAComponent.h \
    $$PWD/../../AudioUnit/caudiounitclass.h \
    $$PWD/caudiounithost.h

macx {
HEADERS += \
    /Library/Developer/Extras/CoreAudio/PublicUtility/CAStreamBasicDescription.h
}

OBJECTIVE_SOURCES += \
    $$PWD/../../AudioUnit/caudiounitclass.mm

ios {
CONFIG += objcpp
##DEFINES += __OBJC__

OBJECTIVE_HEADERS += \
    $$PWD/caudiounitobjcwrapper.h

OBJECTIVE_SOURCES += \
    $$PWD/caudiounitobjcwrapper.mm
}

}
