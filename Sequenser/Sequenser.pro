#-------------------------------------------------
#
# Project created by QtCreator 2011-10-07T09:35:25
#
#-------------------------------------------------

TARGET = Sequenser

DEFINES += SEQUENSER_LIBRARY

include(../SoftSynthsIncludes.pri)

LIBS += -lDrumMachine
INCLUDEPATH += ../drummachine

SOURCES += csequenser.cpp
HEADERS += csequenser.h

SOURCES += \
    csequenserform.cpp

HEADERS += \
    csequenserform.h

FORMS += \
    csequenserform.ui















