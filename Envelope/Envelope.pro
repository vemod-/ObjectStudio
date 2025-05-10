#-------------------------------------------------
#
# Project created by QtCreator 2011-10-02T14:54:32
#
#-------------------------------------------------

TARGET = Envelope

include(../SoftSynthsIncludes.pri)

LIBS += -lSoftSynthsWidgets
INCLUDEPATH += ../../QCanvas

DEFINES += ENVELOPE_LIBRARY

SOURCES += cadsrwidget.cpp \
    cenvelopeform.cpp

HEADERS += cadsrwidget.h \
    cenvelopeform.h

SOURCES += cenvelope.cpp \
    cadsr.cpp \
    cadsrcontrol.cpp

HEADERS += cenvelope.h \
    cadsr.h \
    cadsrcontrol.h

FORMS += \
    cadsrcontrol.ui \
    cadsrwidget.ui \
    cenvelopeform.ui












