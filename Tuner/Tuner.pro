#-------------------------------------------------
#
# Project created by QtCreator 2017-07-20T07:44:35
#
#-------------------------------------------------

TARGET = Tuner

include($$PWD/../SoftSynthsIncludes.pri)
include($$PWD/../../QCanvas/QCanvas.pri)

LIBS += -lPitchTracker

INCLUDEPATH += ../PitchTracker

DEFINES += TUNER_LIBRARY

SOURCES += \
        ctuner.cpp \
    ctunerform.cpp \
    ctunerwidget.cpp

HEADERS += \
        ctuner.h \
    ctunerform.h \
    ctunerwidget.h

FORMS += \
    ctunerform.ui \
    ctunerwidget.ui

