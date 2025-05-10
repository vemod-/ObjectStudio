!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

greaterThan(QT_MAJOR_VERSION, 4){
    QT += widgets
    lessThan(QT_MAJOR_VERSION, 6) {
        QT += macextras
    }
}

#QMAKE_OBJECTIVE_CFLAGS += -fobjc-arc

macx {
    LIBS += -framework AppKit
}

ios {
    LIBS += -framwork UIKit
}

#!defined(__x86_64): LIBS += -framework Carbon

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../SoftSynthsClasses
INCLUDEPATH += $$PWD/../PluginLoader
INCLUDEPATH += /Library/Developer/SDKs/VST_SDK/VST3_SDK
INCLUDEPATH += /Library/Developer/SDKs/VST_SDK/VST2_SDK/public.sdk/source/vst2.x

include($$PWD/../../EventHandlers/EventHandlers.pri)
include($$PWD/../../QSignalMenu/QSignalMenu.pri)

SOURCES += $$PWD/cvsthost.cpp \
    $$PWD/cvstform.cpp
OBJECTIVE_SOURCES += $$PWD/cmacwindow.mm \
    $$PWD/cvsthostclass.mm \
    $$PWD/macstrings.mm \
    $$PWD/iaudiopluginhost.mm

HEADERS += $$PWD/cvsthost.h \
    $$PWD/cvsthostclass.h \
    ##$$PWD/aeffectx.h \
    ##$$PWD/AEffect.h \
    ##/Library/Developer/SDKs/VST_SDK/VST2_SDK/public.sdk/source/vst2.x/audioeffect.h \
    $$PWD/cmacwindow.h \
    $$PWD/macstrings.h \
    $$PWD/cvstform.h \
    $$PWD/../SoftSynthsClasses/imidiparser.h \
    $$PWD/iaudiopluginhost.h \

FORMS += $$PWD/cvstform.ui
}

##HEADERS += \
##    $$PWD/vstfxstore.h

