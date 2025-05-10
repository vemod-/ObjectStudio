#-------------------------------------------------
#
# Project created by QtCreator 2017-10-18T09:22:26
#
#-------------------------------------------------

TARGET = Spectrum

DEFINES += SPECTRUM_LIBRARY

include(../SoftSynthsIncludes.pri)
LIBS += -lSoftSynthsWidgets

INCLUDEPATH += ../../QCanvas
INCLUDEPATH += ../Scope

SOURCES += \
    ../Scope/cspectrumcontrol.cpp \
    cspectrum.cpp \
    cspectrumform.cpp

HEADERS += \
    ../Scope/cspectrumcontrol.h \
        cspectrum.h \
    cspectrumform.h

FORMS += \
    ../Scope/cspectrumcontrol.ui \
    cspectrumform.ui
