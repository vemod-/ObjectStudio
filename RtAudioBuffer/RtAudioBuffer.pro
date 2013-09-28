#-------------------------------------------------
#
# Project created by QtCreator 2011-09-18T22:59:50
#
#-------------------------------------------------

QT       += core gui
QT       -= network opengl sql svg xml xmlpatterns qt3support
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt x86
CONFIG -= x86_64 ppc64 ppc

LIBS += -framework CoreMidi -framework CoreAudio ##-framework Carbon
LIBS += -L../ -lSoftSynthsClasses

INCLUDEPATH += ../SoftSynthsClasses

include(../RtAudioBuffer/DesktopComponent.pri)
include(../WaveGenerator/WaveFile.pri)
include(PeakMeter.pri)
include(../../EventHandlers/EventHandlers.pri)

DEFINES += __MACOSX_CORE__

SOURCES += main.cpp\
        mainwindow.cpp \
    ../../../../../../Developer/Library/rtaudio-4.0.11/RtAudio.cpp \
    ../../../../../../Developer/Library/rtmidi-2.0.1/RtMidi.cpp \
    corebuffer.cpp

HEADERS  += mainwindow.h \
    ../../../../../../Developer/Library/rtaudio-4.0.11/RtAudio.h \
    ../../../../../../Developer/Library/rtmidi-2.0.1/RtMidi.h \
    corebuffer.h

FORMS    += mainwindow.ui

TARGET = objectstudio
TEMPLATE = app
DESTDIR    = ../

RESOURCES += Resources.qrc





























































































