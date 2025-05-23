#-------------------------------------------------
#
# Project created by QtCreator 2018-11-09T21:22:36
#
#-------------------------------------------------

TARGET = Harmonizer

DEFINES += HARMONIZER_LIBRARY

DEFINES += VOCODER_LIBRARY

LIBS += -lMIDI2CV
INCLUDEPATH += ../MIDI2CV

LIBS += -lPitchTracker
INCLUDEPATH += ../PitchTracker/

LIBS += -lPitchShifter
INCLUDEPATH += ../PitchShifter/

include(../SoftSynthsIncludes.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        charmonizer.cpp

HEADERS += \
        charmonizer.h

