#-------------------------------------------------
#
# Project created by QtCreator 2011-09-26T09:33:35
#
#-------------------------------------------------

QT += core gui
QT -= network opengl sql svg xml xmlpatterns qt3support
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt x86
CONFIG -= x86_64 ppc64 ppc

TARGET = WaveGenerator
TEMPLATE = lib

LIBS += -L../ -lSoftSynthsClasses
INCLUDEPATH += ../SoftSynthsClasses

include(WaveFile.pri)

DESTDIR = ../

SOURCES += cwavegenerator.cpp

HEADERS += cwavegenerator.h







