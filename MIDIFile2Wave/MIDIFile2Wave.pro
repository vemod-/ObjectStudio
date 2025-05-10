#-------------------------------------------------
#
# Project created by QtCreator 2013-02-24T11:21:48
#
#-------------------------------------------------
TARGET = MIDIFile2Wave

include(../SoftSynthsIncludes.pri)

DEFINES += MIDIFILE2WAVE_LIBRARY

LIBS += -lStereoMixer
INCLUDEPATH += ../StereoMixer
LIBS += -lMIDIFileReader
INCLUDEPATH += ../MIDIFileReader
LIBS += -lMIDIFilePlayer
INCLUDEPATH += ../MIDIFilePlayer

LIBS += -lSoftSynthsWidgets
INCLUDEPATH += ../../SynthKnob
INCLUDEPATH += ../../LCDLabel
INCLUDEPATH += ../../QSignalMenu
INCLUDEPATH += ../RtAudioBuffer
INCLUDEPATH += ../../SynthPanel

include(../RtAudioBuffer/UIMap.pri)

INCLUDEPATH += $$PWD/../PluginLoader

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lPluginLoader
    }
}

SOURCES += cmidifile2wave.cpp ##\
    ##../StereoMixer/cmixerwidget.cpp \
    ##../StereoMixer/cstereomixer.cpp
HEADERS += cmidifile2wave.h ##\
    ##../StereoMixer/cmixerwidget.h \
    ##../StereoMixer/cstereomixer.h

##FORMS += \
    ##../StereoMixer/cmixerwidget.ui

##HEADERS += \
##    csf2channelwidget.h

##SOURCES += \
##    csf2channelwidget.cpp

##FORMS += \
##    csf2channelwidget.ui

RESOURCES += \
    midifile2waveresources.qrc

