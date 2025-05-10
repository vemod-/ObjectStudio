#-------------------------------------------------
#
# Project created by QtCreator 2011-09-26T11:10:10
#
#-------------------------------------------------

TARGET = SF2Generator
DESTDIR = ../

include(../SoftSynthsFrameworks.pri)

macx {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
        LIBS += -F$$PWD/../WaveBank
    } else {
        LIBS += -L../ -lSoftSynthsClasses
        LIBS += -L../ -lWaveBank
    }
}
ios {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
        LIBS += -F$$PWD/../WaveBank
    }
    contains(DEFINES,BUILD_WITH_STATIC) {
        LIBS += -L$$PWD/../ -lSoftSynthsClasses
        LIBS += -L$$PWD/../ -lWaveBank
    }
}

INCLUDEPATH += ../SoftSynthsClasses
INCLUDEPATH += ../WaveBank
INCLUDEPATH += ../../QDomLite

include(SF2Generator.pri)


