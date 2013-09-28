INCLUDEPATH += $$PWD

include($$PWD/../SF2Generator/SF2Generator.pri)
include($$PWD/../../SynthKnob/QSynthKnob.pri)

HEADERS += $$PWD/csf2player.h \
    $$PWD/csf2device.h \
    $$PWD/csf2playerform.h


SOURCES += $$PWD/csf2player.cpp \
    $$PWD/csf2device.cpp \
    $$PWD/csf2playerform.cpp

FORMS += $$PWD/csf2playerform.ui

INCLUDEPATH += $$PWD/../WaveBank

HEADERS += $$PWD/../WaveBank/cwavebank.h

SOURCES += $$PWD/../WaveBank/cwavebank.cpp
