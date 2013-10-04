#-------------------------------------------------
#
# Project created by QtCreator 2013-06-28T21:57:16
#
#-------------------------------------------------

TARGET = StereoSplitBox
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)
include(../../QCanvas/QCanvas.pri)

DEFINES += STEREOSPLITBOX_LIBRARY

include(../RtAudioBuffer/DesktopComponent.pri)

INCLUDEPATH += ../MacroBox

SOURCES += cstereosplitbox.cpp \
    ../MacroBox/cmacroboxform.cpp

HEADERS += cstereosplitbox.h \
    ../MacroBox/cmacroboxform.h

FORMS += \
    ../MacroBox/cmacroboxform.ui
