#-------------------------------------------------
#
# Project created by QtCreator 2011-10-19T15:27:45
#
#-------------------------------------------------

TARGET = Equalizer

include(../SoftSynthsIncludes.pri)

LIBS += -lSoftSynthsWidgets

INCLUDEPATH += ../../QCanvas
INCLUDEPATH += ../RtAudioBuffer
INCLUDEPATH += ../../SynthKnob
INCLUDEPATH += ../../SynthSlider

LIBS += -lWaveBank
INCLUDEPATH += ../wavebank

INCLUDEPATH += ../Chorus

SOURCES += cequalizerframe.cpp \
    cequalizerform.cpp \
    cequalizergraph.cpp

HEADERS += cequalizerframe.h \
    cequalizerform.h \
    cequalizergraph.h \
    ../Chorus/biquad.h

DEFINES += EQUALIZER_LIBRARY

SOURCES += cequalizer.cpp

HEADERS += cequalizer.h


FORMS += \
    cequalizerframe.ui \
    cequalizerform.ui \
    cequalizergraph.ui












