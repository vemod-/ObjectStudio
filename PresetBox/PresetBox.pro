#-------------------------------------------------
#
# Project created by QtCreator 2017-03-22T22:13:30
#
#-------------------------------------------------

TARGET = PresetBox

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

DEFINES += PRESETBOX_LIBRARY

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lPluginLoader
        LIBS += -lz
    }
}

SOURCES += cpresetbox.cpp ##\
    ##../MacroBox/cmacroboxform.cpp

HEADERS += cpresetbox.h ##\
    ##../MacroBox/cmacroboxform.h

##FORMS += ../MacroBox/cmacroboxform.ui
