#-------------------------------------------------
#
# Project created by QtCreator 2011-10-04T23:16:49
#
#-------------------------------------------------

TARGET = DrumMachine

include(../SoftSynthsIncludes.pri)

LIBS += -lWaveGenerator
INCLUDEPATH += ../wavegenerator

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

SOURCES += cdrummachine.cpp
HEADERS += cdrummachine.h

DEFINES += DRUMMACHINE_LIBRARY

SOURCES += \
    cdrummachineform.cpp \
    cbeatframe.cpp \
    crepeatform.cpp \
    cinsertpatternform.cpp

HEADERS += \
    sequenserclasses.h \
    cdrummachineform.h \
    cbeatframe.h \
    crepeatform.h \
    cinsertpatternform.h

RESOURCES += \
    Sounds.qrc

FORMS += \
    cdrummachineform.ui \
    cbeatframe.ui \
    crepeatform.ui \
    cinsertpatternform.ui

























