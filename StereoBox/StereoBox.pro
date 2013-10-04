#-------------------------------------------------
#
# Project created by QtCreator 2013-06-19T12:02:13
#
#-------------------------------------------------

TARGET = StereoBox
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)
include(../../QCanvas/QCanvas.pri)

INCLUDEPATH += ../MacroBox

include(../RtAudioBuffer/DesktopComponent.pri)

DEFINES += STEREOBOX_LIBRARY

SOURCES += \
    cstereobox.cpp \
    ../MacroBox/cmacroboxform.cpp

HEADERS += \
    cstereobox.h \
    ../MacroBox/cmacroboxform.h

FORMS += \
    ../MacroBox/cmacroboxform.ui
