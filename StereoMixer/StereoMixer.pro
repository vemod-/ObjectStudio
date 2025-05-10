#-------------------------------------------------
#
# Project created by QtCreator 2013-01-21T00:36:07
#
#-------------------------------------------------
TARGET = StereoMixer

DEFINES += STEREOMIXER_LIBRARY

include(../SoftSynthsIncludes.pri)
LIBS += -lSoftSynthsWidgets
LIBS += -lEffectRack

INCLUDEPATH += ../../EffectLabel\
../../SynthKnob \
../../SynthSlider \
../../SynthPanel \
../../ToggleButton \
../../QSignalMenu \
../RtAudioBuffer \
../../LCDLabel \
../../QCanvas

INCLUDEPATH += ../PlugInLoader
INCLUDEPATH += ../Preamp
INCLUDEPATH += ../EffectRack

##include(../../EffectLabel/EffectLabel.pri)
##include(../../SynthKnob/QSynthKnob.pri)
##include(../../SynthSlider/QSynthSlider.pri)
##include(../../SynthPanel/QSynthPanel.pri)
##include(../../ToggleButton/QToggleButton.pri)

##include(../../QSignalMenu/QSignalMenu.pri)
##include(../RtAudioBuffer/PeakMeter.pri)
##include(../../LCDLabel/QLCDLabel.pri)

SOURCES += cstereomixer.cpp \
    cstereomixerform.cpp \
    csf2channelwidget.cpp \
    cmasterwidget.cpp \
    cmastervol.cpp \
    cchannelvol.cpp \
    cchanneleffects.cpp \
    cmixerwidget.cpp \
    cchanneleq.cpp \
    cchannelgain.cpp
HEADERS += cstereomixer.h \
    cstereomixerform.h \
    csf2channelwidget.h \
    cmasterwidget.h \
    cmastervol.h \
    cchannelvol.h \
    cchanneleffects.h \
    cmixerwidget.h \
    cchanneleq.h \
    cchannelgain.h

FORMS += \
    cstereomixerform.ui \
    csf2channelwidget.ui \
    cmasterwidget.ui \
    cmastervol.ui \
    cchannelvol.ui \
    cchanneleffects.ui \
    cmixerwidget.ui \
    cchanneleq.ui \
    cchannelgain.ui
