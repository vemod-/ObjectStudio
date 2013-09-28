#-------------------------------------------------
#
# Project created by QtCreator 2011-10-04T09:10:15
#
#-------------------------------------------------

TARGET = PolyBox
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)

INCLUDEPATH += ../MacroBox
INCLUDEPATH += ../MIDI2CV

include(../RtAudioBuffer/DesktopComponent.pri)

SOURCES += ../MIDI2CV/ccvdevice.cpp

HEADERS += ../MIDI2CV/ccvdevice.h

DEFINES += POLYBOX_LIBRARY

SOURCES += cpolybox.cpp \
    ../MacroBox/cmacroboxform.cpp

HEADERS += cpolybox.h \
    ../MacroBox/cmacroboxform.h

FORMS += ../MacroBox/cmacroboxform.ui




