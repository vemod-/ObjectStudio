#-------------------------------------------------
#
# Project created by QtCreator 2017-07-20T10:31:16
#
#-------------------------------------------------

TARGET = AutoTune

include(../SoftSynthsIncludes.pri)

LIBS += -lPitchTracker
LIBS += -lPitchShifter

INCLUDEPATH += ../../QCanvas
INCLUDEPATH += ../PitchTracker
INCLUDEPATH += ../PitchShifter

DEFINES += AUTOTUNE_LIBRARY

SOURCES += \
        cautotune.cpp

HEADERS += \
        cautotune.h

