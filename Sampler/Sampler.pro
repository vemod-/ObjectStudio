#-------------------------------------------------
#
# Project created by QtCreator 2011-10-25T22:20:57
#
#-------------------------------------------------

TARGET = Sampler

include(../SoftSynthsIncludes.pri)

LIBS += -lSoftSynthsWidgets
INCLUDEPATH += ../../QCanvas

LIBS += -lWaveGenerator
INCLUDEPATH += ../WaveGenerator

LIBS += -lWaveBank
INCLUDEPATH += ../WaveBank

LIBS += -lEnvelope
INCLUDEPATH += ../Envelope

LIBS += -lPitchTracker
INCLUDEPATH += ../PitchTracker

LIBS += -lWaveRecorder
INCLUDEPATH += ../WaveRecorder

macx {
    contains(DEFINES,MACXSTATICLIBS) {
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

SOURCES += csamplerform.cpp \
    cwavelayers.cpp \
    ##../Envelope/cadsrwidget.cpp \
    ##../Envelope/cadsrcontrol.cpp \
    ##../Envelope/cadsr.cpp \
    ckeylayoutcontrol.cpp \
    clayer.cpp \
    crange.cpp \
    ##../PitchTracker/cpitchtrackerclass.cpp \
    ##../WaveRecorder/cwaveeditwidget.cpp \
    ##../WaveRecorder/cwaveeditcontrol.cpp \
    ckeyrangescontrol.cpp \
    csamplergenerator.cpp \
    csamplerdevice.cpp \
    ckeylayerscontrol.cpp \
    cmidinoteedit.cpp
HEADERS += csamplerform.h \
    cwavelayers.h \
    ##../Envelope/cadsrwidget.h \
    ##../Envelope/cadsrcontrol.h \
    ##../Envelope/cadsr.h \
    ckeylayoutcontrol.h \
    clayer.h \
    crange.h \
    ##../PitchTracker/cpitchtrackerclass.h \
    ##../WaveRecorder/cwaveeditwidget.h \
    ##../WaveRecorder/cwaveeditcontrol.h \
    ckeyrangescontrol.h \
    csamplergenerator.h \
    csamplerdevice.h \
    ckeylayerscontrol.h \
    cmidinoteedit.h

DEFINES += SAMPLER_LIBRARY

SOURCES += csampler.cpp

HEADERS += csampler.h


FORMS += \
    csamplerform.ui \
    cwavelayers.ui \
    ##../Envelope/cadsrwidget.ui \
    ##../Envelope/cadsrcontrol.ui \
    ckeylayoutcontrol.ui \
    ##../WaveRecorder/cwaveeditwidget.ui \
    ##../WaveRecorder/cwaveeditcontrol.ui \
    ckeyrangescontrol.ui \
    ckeylayerscontrol.ui \
    cmidinoteedit.ui

##HEADERS += \
##    ../MIDI2CV/ccvdevice.h

##SOURCES += \
##    ../MIDI2CV/ccvdevice.cpp

RESOURCES += \
    Resources.qrc





























