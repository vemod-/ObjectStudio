#-------------------------------------------------
#
# Project created by QtCreator 2011-10-10T11:29:08
#
#-------------------------------------------------

TARGET = VSTHost
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)

DEFINES += VSTHOST_LIBRARY

include(VSTHost.pri)

LIBS += -L../ -lPluginLoader
















