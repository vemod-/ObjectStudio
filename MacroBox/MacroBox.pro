#-------------------------------------------------
#
# Project created by QtCreator 2011-10-03T17:36:25
#
#-------------------------------------------------

TARGET = MacroBox
TEMPLATE = lib

DEFINES += MACROBOX_LIBRARY

include(../SoftSynthsIncludes.pri)
include(../../QCanvas/QCanvas.pri)

include(../RtAudioBuffer/DesktopComponent.pri)

SOURCES += cmacrobox.cpp \
    cmacroboxform.cpp

HEADERS += cmacrobox.h \
    cmacroboxform.h

FORMS += \
    cmacroboxform.ui






