#-------------------------------------------------
#
# Project created by QtCreator 2011-10-06T15:24:18
#
#-------------------------------------------------

TARGET = SF2Player

include(../SoftSynthsIncludes.pri)

LIBS += -lSoftSynthsWidgets
INCLUDEPATH += ../../SynthKnob \
../../SynthPanel \
../../EffectLabel \
../../ToggleButton \
../../LCDLabel \
../../QSignalMenu

LIBS += -lSF2Generator

INCLUDEPATH += ../SF2Generator
INCLUDEPATH += ../Envelope
INCLUDEPATH += ../WaveBank

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lWaveBank
    }
}

HEADERS += csf2player.h \
    csf2device.h \
    csf2playerform.h

SOURCES += csf2player.cpp \
    csf2device.cpp \
    csf2playerform.cpp

FORMS += csf2playerform.ui

DEFINES += SF2PLAYER_LIBRARY














