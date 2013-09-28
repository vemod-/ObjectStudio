#-------------------------------------------------
#
# Project created by QtCreator 2011-09-26T11:10:10
#
#-------------------------------------------------

CONFIG += qt x86
CONFIG -= x86_64 ppc64 ppc

QT += core gui
QT -= network opengl sql svg xml xmlpatterns qt3support
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SF2Generator
TEMPLATE = lib

LIBS += -L../ -lSoftSynthsClasses
LIBS += -L../ -lWaveBank

INCLUDEPATH += ../SoftSynthsClasses
INCLUDEPATH += ../WaveBank
INCLUDEPATH += ../../QDomLite

include(SF2Generator.pri)

DESTDIR = ../

