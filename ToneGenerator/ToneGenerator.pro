#-------------------------------------------------
#
# Project created by QtCreator 2011-09-27T21:36:26
#
#-------------------------------------------------

TARGET = ToneGenerator
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)

LIBS += -L../ -lWaveBank
INCLUDEPATH += ../wavebank

DEFINES += TONEGENERATOR_LIBRARY

SOURCES += ctonegenerator.cpp

HEADERS += ctonegenerator.h

