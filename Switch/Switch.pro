TARGET = Switch

include(../SoftSynthsIncludes.pri)

greaterThan(QT_MAJOR_VERSION, 4){
    QT +=  widgets
    lessThan(QT_MAJOR_VERSION, 6) {
        QT += macextras
    }
}


DEFINES += SWITCH_LIBRARY

SOURCES += \
    cswitch.cpp

HEADERS += \
    cswitch.h

