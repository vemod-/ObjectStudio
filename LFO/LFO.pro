#-------------------------------------------------
#
# Project created by QtCreator 2011-09-28T22:53:14
#
#-------------------------------------------------

TARGET = LFO

include(../SoftSynthsIncludes.pri)

LIBS += -lWaveBank
INCLUDEPATH += ../wavebank

DEFINES += LFO_LIBRARY

SOURCES += clfo.cpp

HEADERS += clfo.h

