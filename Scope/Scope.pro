#-------------------------------------------------
#
# Project created by QtCreator 2011-09-30T15:23:26
#
#-------------------------------------------------

TARGET = Scope

include(../SoftSynthsIncludes.pri)
LIBS += -lSoftSynthsWidgets

LIBS += -lPitchTracker

INCLUDEPATH += ../../QCanvas
INCLUDEPATH += ../PitchTracker

DEFINES += SCOPE_LIBRARY

SOURCES += cscope.cpp \
    cscopeform.cpp \
    cscopecontrol.cpp

HEADERS += cscope.h \
    cscopeform.h \
    cscopecontrol.h

FORMS += \
    cscopeform.ui \
    cscopecontrol.ui










