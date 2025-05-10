#-------------------------------------------------
#
# Project created by QtCreator 2017-08-12T09:30:35
#
#-------------------------------------------------

TARGET = Vocoder

DEFINES += VOCODER_LIBRARY

LIBS += -lMIDI2CV
INCLUDEPATH += ../MIDI2CV

LIBS += -lPitchTracker
INCLUDEPATH += ../PitchTracker/

LIBS += -lPitchShifter
INCLUDEPATH += ../PitchShifter/

include(../SoftSynthsIncludes.pri)

SOURCES += \
        cvocoder.cpp

HEADERS += \
        cvocoder.h

