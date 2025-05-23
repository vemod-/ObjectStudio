#-------------------------------------------------
#
# Project created by QtCreator 2013-07-21T08:18:08
#
#-------------------------------------------------

TARGET = AudioUnitHost

INCLUDEPATH += ../VSTHost

include(../SoftSynthsIncludes.pri)

DEFINES += AUDIOUNITHOST_LIBRARY

include(AudioUnitHost.pri)

LIBS += -lPluginLoader

HEADERS += ../VSTHost/cmacwindow.h \
    ../VSTHost/macstrings.h \
    ../VSTHost/iaudiopluginhost.h \
    ../VSTHost/cvstform.h

SOURCES += \
    ../VSTHost/cvstform.cpp

OBJECTIVE_SOURCES += ../VSTHost/cmacwindow.mm \
    ../VSTHost/iaudiopluginhost.mm \
    ../VSTHost/macstrings.mm

FORMS += \
    ../VSTHost/cvstform.ui
