#-------------------------------------------------
#
# Project created by QtCreator 2017-03-19T00:57:05
#
#-------------------------------------------------

TARGET = ProgramBox

include(../SoftSynthsIncludes.pri)

LIBS += -lSoftSynthsWidgets

INCLUDEPATH += $$PWD/../SoftSynthsWidgets
INCLUDEPATH += ../../SynthKnob
INCLUDEPATH += ../../QCanvas
INCLUDEPATH += ../MacroBox

##LIBS += -lMacroBox
##INCLUDEPATH += ../MacroBox
INCLUDEPATH += ../RtAudioBuffer
INCLUDEPATH += ../PlugInLoader
INCLUDEPATH += ../../QiPhotoRubberband
INCLUDEPATH += ../../QSignalMenu
INCLUDEPATH += ../../SynthPanel

#LIBS += -lSoftSynthsWidgets
INCLUDEPATH += ../../ToggleButton
INCLUDEPATH += ../../QsynthButtonPanel

##include(../RtAudioBuffer/DesktopComponent.pri)
##include(../../QsynthButtonPanel/QSynthButtonPanel.pri)

DEFINES += PROGRAMBOX_LIBRARY

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lPluginLoader
        LIBS += -lz
    }
}

SOURCES += cprogrambox.cpp ##\
    ##../MacroBox/cmacroboxform.cpp

HEADERS += cprogrambox.h ##\
    ##../MacroBox/cmacroboxform.h

##FORMS += ../MacroBox/cmacroboxform.ui
