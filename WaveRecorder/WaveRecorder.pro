#-------------------------------------------------
#
# Project created by QtCreator 2011-10-20T22:57:04
#
#-------------------------------------------------

TARGET = WaveRecorder
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)
include(../../QCanvas/QCanvas.pri)

LIBS += -lWaveGenerator

INCLUDEPATH += ../wavegenerator

SOURCES += cwaverecorderform.cpp \
    cwavelanes.cpp \
    cwaveeditcontrol.cpp \
    cwaveeditwidget.cpp

HEADERS += cwaverecorderform.h \
    cwavelanes.h \
    cwaveeditcontrol.h \
    cwaveeditwidget.h

DEFINES += WAVERECORDER_LIBRARY

SOURCES += cwaverecorder.cpp

HEADERS += cwaverecorder.h

FORMS += \
    cwaverecorderform.ui \
    cwavelanes.ui \
    cwaveeditcontrol.ui \
    cwaveeditwidget.ui













