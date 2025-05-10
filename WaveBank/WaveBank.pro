#-------------------------------------------------
#
# Project created by QtCreator 2011-09-26T09:57:16
#
#-------------------------------------------------

TARGET = WaveBank
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
INCLUDEPATH += ../../QDomLite

HEADERS += cwavebank.h

SOURCES += \
    cwavebank.cpp
