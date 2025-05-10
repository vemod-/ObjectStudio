#-------------------------------------------------
#
# Project created by QtCreator 2015-06-16T07:42:56
#
#-------------------------------------------------

TARGET = SampleAndHold

include(../SoftSynthsIncludes.pri)

LIBS += -lWaveBank
INCLUDEPATH += ../wavebank

DEFINES += SAMPLEANDHOLD_LIBRARY

SOURCES += csampleandhold.cpp

HEADERS += csampleandhold.h

