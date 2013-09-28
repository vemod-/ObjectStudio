#-------------------------------------------------
#
# Project created by QtCreator 2011-10-19T15:27:45
#
#-------------------------------------------------

TARGET = Equalizer
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)
include(../RtAudioBuffer/PeakMeter.pri)
include(../../SynthKnob/QSynthKnob.pri)
include(../../SynthSlider/QSynthSlider.pri)

LIBS += -L../ -lWaveBank
INCLUDEPATH += ../wavebank

INCLUDEPATH += ../chorus

SOURCES += cequalizerframe.cpp \
    cequalizerform.cpp \
    cequalizergraph.cpp

HEADERS += cequalizerframe.h \
    cequalizerform.h \
    cequalizergraph.h

DEFINES += EQUALIZER_LIBRARY

SOURCES += cequalizer.cpp

HEADERS += cequalizer.h


FORMS += \
    cequalizerframe.ui \
    cequalizerform.ui \
    cequalizergraph.ui












