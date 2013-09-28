#-------------------------------------------------
#
# Project created by QtCreator 2013-01-21T00:36:07
#
#-------------------------------------------------
TARGET = StereoMixer
TEMPLATE = lib

DEFINES += STEREOMIXER_LIBRARY

include(../SoftSynthsIncludes.pri)
include(../../EffectLabel/EffectLabel.pri)
include(../../SynthKnob/QSynthKnob.pri)
include(../../SynthSlider/QSynthSlider.pri)
include(../../SynthPanel/QSynthPanel.pri)

include(../../QSignalMenu/QSignalMenu.pri)
include(../RtAudioBuffer/PeakMeter.pri)
include(../../LCDLabel/QLCDLabel.pri)

SOURCES += cstereomixer.cpp \
    cstereomixerform.cpp \
    cstereochannelwidget.cpp \
    cmasterwidget.cpp
HEADERS += cstereomixer.h \
    cstereomixerform.h \
    cstereochannelwidget.h \
    cmasterwidget.h

FORMS += \
    cstereomixerform.ui \
    cstereochannelwidget.ui \
    cmasterwidget.ui
