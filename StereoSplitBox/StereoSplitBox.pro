#-------------------------------------------------
#
# Project created by QtCreator 2013-06-28T21:57:16
#
#-------------------------------------------------

TARGET = StereoSplitBox

include(../SoftSynthsIncludes.pri)

DEFINES += STEREOSPLITBOX_LIBRARY

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

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lPluginLoader
        LIBS += -lz
    }
}

SOURCES += cstereosplitbox.cpp

HEADERS += cstereosplitbox.h

