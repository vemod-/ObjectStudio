#-------------------------------------------------
#
# Project created by QtCreator 2011-10-04T09:10:15
#
#-------------------------------------------------

TARGET = PolyBox

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

LIBS+= -lMIDI2CV
INCLUDEPATH += ../MIDI2CV

##include(../RtAudioBuffer/DesktopComponent.pri)

##SOURCES += ../MIDI2CV/ccvdevice.cpp

##HEADERS += ../MIDI2CV/ccvdevice.h

DEFINES += POLYBOX_LIBRARY

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lPluginLoader
        LIBS += -lz
    }
}

SOURCES += cpolybox.cpp ##\
    ##../MacroBox/cmacroboxform.cpp

HEADERS += cpolybox.h ##\
    ##../MacroBox/cmacroboxform.h

##FORMS += ../MacroBox/cmacroboxform.ui




