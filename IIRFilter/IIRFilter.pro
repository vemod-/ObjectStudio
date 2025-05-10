#-------------------------------------------------
#
# Project created by QtCreator 2017-09-02T23:00:37
#
#-------------------------------------------------

TARGET = IIRFilter

include(../SoftSynthsIncludes.pri)

DEFINES += IIRFILTER_LIBRARY

LIBS += -lPitchTracker
INCLUDEPATH += ../PitchTracker

SOURCES += \
        ciirfilter.cpp

HEADERS += \
        ciirfilter.h

