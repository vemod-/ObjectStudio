#-------------------------------------------------
#
# Project created by QtCreator 2011-09-27T18:07:53
#
#-------------------------------------------------

TARGET = MIDIFilePlayer

include(../SoftSynthsIncludes.pri)

LIBS += -lMIDIFileReader
INCLUDEPATH += ../MIDIFileReader

DEFINES += MIDIFILEPLAYER_LIBRARY

SOURCES += cmidifileplayer.cpp

HEADERS += cmidifileplayer.h

