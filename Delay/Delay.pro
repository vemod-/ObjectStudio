#-------------------------------------------------
#
# Project created by QtCreator 2011-09-28T22:42:31
#
#-------------------------------------------------

TARGET = Delay

include(../SoftSynthsIncludes.pri)

LIBS += -lWaveBank
INCLUDEPATH += ../wavebank

DEFINES += DELAY_LIBRARY

SOURCES += cdelay.cpp

HEADERS += cdelay.h


