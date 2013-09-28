#-------------------------------------------------
#
# Project created by QtCreator 2011-10-06T15:24:18
#
#-------------------------------------------------

TARGET = SF2Player
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)
include(../../SynthKnob/QSynthKnob.pri)
include(../../SynthPanel/QSynthPanel.pri)
##include(../SF2Generator/SF2Generator.pri)

LIBS += -L../ -lSF2Generator

INCLUDEPATH += ../SF2Generator
INCLUDEPATH += ../WaveBank

HEADERS += csf2player.h \
    csf2device.h \
    csf2playerform.h

SOURCES += csf2player.cpp \
    csf2device.cpp \
    csf2playerform.cpp

FORMS += csf2playerform.ui

##HEADERS += ../WaveBank/cwavebank.h

##SOURCES += ../WaveBank/cwavebank.cpp

DEFINES += SF2PLAYER_LIBRARY














