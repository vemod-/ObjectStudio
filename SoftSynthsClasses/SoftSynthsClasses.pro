#-------------------------------------------------
#
# Project created by QtCreator 2011-09-26T08:47:24
#
#-------------------------------------------------

QT       += core gui
QT       -= network opengl sql svg xml xmlpatterns qt3support
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt x86
CONFIG -= x86_64 ppc64 ppc

TARGET = SoftSynthsClasses
TEMPLATE = lib

DESTDIR = ../

include(../../QDomLite/QDomLite.pri)

SOURCES += \
    cmidibuffer.cpp \
    ijack.cpp

HEADERS += softsynthsclasses.h \
    cmidibuffer.h \
    idevicebase.h \
    ijackbase.h \
    caudiobuffer.h \
    ijack.h \
    ihost.h \
    idevice.h \
    softsynthsdefines.h

HEADERS += \
    csounddevice.h

SOURCES += \
    csounddevice.cpp

HEADERS += \
    cfreqglider.h

SOURCES += \
    cfreqglider.cpp

HEADERS += \
    cpresets.h

SOURCES += \
    cpresets.cpp

HEADERS += \
    csoftsynthsform.h

SOURCES += \
    csoftsynthsform.cpp






