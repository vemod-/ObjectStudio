#-------------------------------------------------
#
# Project created by QtCreator 2017-02-07T22:51:57
#
#-------------------------------------------------

TARGET = Preamp

include(../SoftSynthsIncludes.pri)

DEFINES += PREAMP_LIBRARY

SOURCES += cpreamp.cpp

HEADERS += cpreamp.h \
    filters.h \
    c3bandfilter.h \
    campsimulator.h

