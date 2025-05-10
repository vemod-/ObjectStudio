#-------------------------------------------------
#
# Project created by QtCreator 2011-09-27T21:36:26
#
#-------------------------------------------------

TARGET = ToneGenerator

include(../SoftSynthsIncludes.pri)

LIBS += -lWaveBank

INCLUDEPATH += ../wavebank

DEFINES += TONEGENERATOR_LIBRARY

SOURCES += ctonegenerator.cpp

HEADERS += ctonegenerator.h

