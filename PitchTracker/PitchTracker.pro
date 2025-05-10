#-------------------------------------------------
#
# Project created by QtCreator 2011-10-25T19:27:32
#
#-------------------------------------------------

TARGET = PitchTracker

include(../SoftSynthsIncludes.pri)

SOURCES += cpitchtrackerclass.cpp \
    cpitchdetect.cpp \
    cpitchdsp.cpp \
    cffttracker.cpp
HEADERS += cpitchtrackerclass.h \
    bcf2.h \
    cpitchdetect.h \
    cpitchdsp.h \
    ciirfilters.h \
    cffttracker.h

DEFINES += PITCHTRACKER_LIBRARY

SOURCES += cpitchtracker.cpp

HEADERS += cpitchtracker.h

