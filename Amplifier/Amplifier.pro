#-------------------------------------------------
#
# Project created by QtCreator 2011-09-26T20:52:29
#
#-------------------------------------------------

TARGET = Amplifier

include(../SoftSynthsIncludes.pri)

greaterThan(QT_MAJOR_VERSION, 4){
    QT +=  widgets
    lessThan(QT_MAJOR_VERSION, 6) {
        QT += macextras
    }
}


DEFINES += AMPLIFIER_LIBRARY

SOURCES += camplifier.cpp

HEADERS += camplifier.h

