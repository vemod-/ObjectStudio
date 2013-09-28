#-------------------------------------------------
#
# Project created by QtCreator 2011-09-27T00:09:49
#
#-------------------------------------------------
QT       += core gui
QT       -= network opengl sql svg xml xmlpatterns qt3support

CONFIG += qt x86
CONFIG -= x86_64 ppc64 ppc

INCLUDEPATH += /Developer/Extras/CoreAudio/PublicUtility

LIBS += -framework Cocoa -framework CoreAudio -framework AudioUnit -framework AudioToolbox -framework CoreAudioKit

##macx:LIBS += -framework AppKit -framework Carbon

TARGET = PluginLoader
TEMPLATE = lib

DESTDIR = ../

SOURCES += caddins.cpp \
    /Developer/Extras/CoreAudio/PublicUtility/CAComponentDescription.cpp \
    /Developer/Extras/CoreAudio/PublicUtility/CAComponent.cpp \
    /Developer/Extras/CoreAudio/PublicUtility/CAStreamBasicDescription.cpp


HEADERS += caddins.h \
    /Developer/Extras/CoreAudio/PublicUtility/CAComponentDescription.h \
    /Developer/Extras/CoreAudio/PublicUtility/CAComponent.h \
    /Developer/Extras/CoreAudio/PublicUtility/CAStreamBasicDescription.h \
    singlevstpluginlist.h \
    singleaupluginlist.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OBJECTIVE_SOURCES +=
