#-------------------------------------------------
#
# Project created by QtCreator 2011-10-20T22:57:04
#
#-------------------------------------------------

TARGET = WaveRecorder

include(../SoftSynthsIncludes.pri)
include(../../QFadingWidget/qfadingwidget.pri)
include(../../Projectpage/projectpage.pri)

LIBS += -lSoftSynthsWidgets
LIBS += -lEffectRack
INCLUDEPATH += ../../QCanvas \
../RtAudioBuffer \
../../SynthSlider \
../../SynthPanel \
../../ToggleButton \
../../QSynthCheckbox \
../../SynthKnob \
../../LCDLabel \
../../EffectLabel \
../../QSignalMenu \
../../QGraphicsViewZoomer \
../SoftSynthsWidgets \
../EffectRack \
../QFadingWidget \
../../ObjectComposerXML

LIBS += -lWaveGenerator
INCLUDEPATH += ../wavegenerator

LIBS += -lStereoMixer
INCLUDEPATH += ../StereoMixer

LIBS += -lPitchShifter
INCLUDEPATH += ../pitchshifter
INCLUDEPATH += ../pluginloader

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lPluginLoader
        LIBS += -L../ -lWaveBank

        LIBS += -L/usr/local/Cellar/ffmpeg/6.0/lib
        LIBS += -lavformat
        LIBS += -lavcodec
        LIBS += -lswresample
        LIBS += -lavutil
    }
}

ios {
    contains(DEFINES,FFMPEGLIB) {
        QMAKE_FRAMEWORKPATH += $$QT_INSTALL_LIBS/ffmpeg
        LIBS += -framework libavformat
        LIBS += -framework libavcodec
        LIBS += -framework libswresample
        LIBS += -framework libavutil
    }
}

SOURCES += cwaverecorderform.cpp \
    cwavelanes.cpp \
    cwaveeditcontrol.cpp \
    cwaveeditwidget.cpp \
    cwavelane.cpp \
    cwavetrack.cpp \
    ../../ObjectComposerXML/qmacsplitter.cpp

HEADERS += cwaverecorderform.h \
    cauploader.h \
    cwavedocument.h \
    cwavelanes.h \
    cwaveeditcontrol.h \
    cwaveeditwidget.h \
    cwavelane.h \
    cwavetrack.h \
    ../../ObjectComposerXML/qmacsplitter.h

DEFINES += WAVERECORDER_LIBRARY

SOURCES += cwaverecorder.cpp

HEADERS += cwaverecorder.h

FORMS += \
    cwaverecorderform.ui \
    cwavelanes.ui \
    cwaveeditcontrol.ui \
    cwaveeditwidget.ui












