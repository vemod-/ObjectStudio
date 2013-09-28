#-------------------------------------------------
#
# Project created by QtCreator 2013-02-24T11:21:48
#
#-------------------------------------------------
TARGET = MIDIFile2Wave
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)

DEFINES += MIDIFILE2WAVE_LIBRARY

INCLUDEPATH += ../StereoMixer
INCLUDEPATH += ../MIDIFilePlayer
INCLUDEPATH += ../MIDIFileReader

##include(../../QSignalMenu/QSignalMenu.pri)
include(../../EffectLabel/EffectLabel.pri)
include(../VSTHost/VSTHost.pri)
include(../AudioUnitHost/AudioUnitHost.pri)
include(../RtAudioBuffer/PeakMeter.pri)
include(../../LCDLabel/QLCDLabel.pri)
include(../SF2Player/SF2Player.pri)
include(../../SynthSlider/QSynthSlider.pri)
include(../../SynthPanel/QSynthPanel.pri)

##LIBS += -L../AudioUnitHost/ -lAudioUnitHost
##INCLUDEPATH += ../AudioUnitHost
##LIBS += -L../VSTHost/ -lVSTHost
##INCLUDEPATH += ../VSTHost

SOURCES += cmidifile2wave.cpp \
    cdevicecontainer.cpp
HEADERS += cmidifile2wave.h \
    cdevicecontainer.h

HEADERS += ../StereoMixer/cmasterwidget.h

SOURCES += \
    ../StereoMixer/cmasterwidget.cpp

FORMS += ../StereoMixer/cmasterwidget.ui

HEADERS += \
    csf2channelwidget.h \
    cmixerwidget.h

SOURCES += \
    csf2channelwidget.cpp \
    cmixerwidget.cpp

FORMS += \
    csf2channelwidget.ui \
    cmixerwidget.ui

HEADERS += \
    ../StereoMixer/cstereomixer.h

SOURCES += \
    ../StereoMixer/cstereomixer.cpp

HEADERS += ../MIDIFileReader/cmidifilereader.h \
    ../MIDIFilePlayer/cmidifileplayer.h

SOURCES += ../MIDIFileReader/cmidifilereader.cpp \
    ../MIDIFilePlayer/cmidifileplayer.cpp

