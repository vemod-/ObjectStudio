#-------------------------------------------------
#
# Project created by QtCreator 2013-06-19T12:02:13
#
#-------------------------------------------------

TARGET = StereoBox

include(../SoftSynthsIncludes.pri)

LIBS += -lSoftSynthsWidgets

INCLUDEPATH += $$PWD/../SoftSynthsWidgets
INCLUDEPATH += ../../SynthKnob
INCLUDEPATH += ../../QCanvas
INCLUDEPATH += ../MacroBox

INCLUDEPATH += ../RtAudioBuffer
INCLUDEPATH += ../PlugInLoader
INCLUDEPATH += ../../QiPhotoRubberband
INCLUDEPATH += ../../QSignalMenu
INCLUDEPATH += ../../SynthPanel

DEFINES += STEREOBOX_LIBRARY

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lPluginLoader
        LIBS += -lz
    }
}

SOURCES += \
    cstereobox.cpp ##\

HEADERS += \
    cstereobox.h ##\
