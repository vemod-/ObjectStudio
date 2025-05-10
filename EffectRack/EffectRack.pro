TARGET = EffectRack

include(../SoftSynthsIncludes.pri)

DEFINES += EFFECTRACK_LIBRARY

INCLUDEPATH += $$PWD

LIBS += -lPlugInLoader
INCLUDEPATH += ../PluginLoader

LIBS += -lSoftSynthsWidgets
INCLUDEPATH += $$PWD/../SoftSynthsWidgets

INCLUDEPATH += $$PWD/../../QSignalMenu
INCLUDEPATH += $$PWD/../RtAudioBuffer
include($$PWD/../../EventHandlers/EventHandlers.pri)

SOURCES += \
    ceffectrack.cpp

HEADERS += \
    ceffectrack.h

