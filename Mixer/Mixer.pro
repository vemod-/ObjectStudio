#-------------------------------------------------
#
# Project created by QtCreator 2011-10-07T23:17:15
#
#-------------------------------------------------
TARGET = Mixer
TEMPLATE = lib

DEFINES += MIXER_LIBRARY

include(../SoftSynthsIncludes.pri)
include(../RtAudioBuffer/PeakMeter.pri)
include(../../SynthKnob/QSynthKnob.pri)
include(../../SynthSlider/QSynthSlider.pri)

SOURCES += cmixerframe.cpp \
    cmixerform.cpp

HEADERS += cmixerframe.h \
    cmixerform.h

SOURCES += cmixer.cpp

HEADERS += cmixer.h

FORMS += \
    cmixerframe.ui \
    cmixerform.ui

RESOURCES +=









