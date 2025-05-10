#-------------------------------------------------
#
# Project created by QtCreator 2011-10-07T23:17:15
#
#-------------------------------------------------
TARGET = Mixer

DEFINES += MIXER_LIBRARY

include(../SoftSynthsIncludes.pri)

LIBS += -lSoftSynthsWidgets
INCLUDEPATH += ../RtAudioBuffer \
../../SynthKnob \
../../SynthSlider \
../../ToggleButton \
../../QCanvas

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









