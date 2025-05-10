#-------------------------------------------------
#
# Project created by QtCreator 2011-09-26T09:33:35
#
#-------------------------------------------------

TARGET = WaveGenerator
DESTDIR = ../

include(../SoftSynthsFrameworks.pri)

macx {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
    } else {
        LIBS += -L../ -lSoftSynthsClasses
    }
}
ios {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
    }
    contains(DEFINES,BUILD_WITH_STATIC) {
        LIBS += -L$$PWD/../ -lSoftSynthsClasses
    }
}
INCLUDEPATH += ../SoftSynthsClasses

include(WaveFile.pri)
include(../../QDomLite/QDomLite.pri)

SOURCES += cwavegenerator.cpp

HEADERS += cwavegenerator.h







