!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

DEFINES += QT_USE_QSTRINGBUILDER
DEFINES += QT_USE_FAST_CONCATENATION
DEFINES += QT_USE_FAST_OPERATOR_PLUS

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../WaveBank
INCLUDEPATH += $$PWD/../Envelope

HEADERS += \
    $$PWD/enabler/sfreader.h \
    $$PWD/enabler/sfnav.h \
    $$PWD/enabler/sflookup.h \
    $$PWD/enabler/sfenum.h \
    $$PWD/enabler/sfdata.h \
    $$PWD/enabler/riff.h \
    $$PWD/enabler/hydra.h \
    $$PWD/enabler/enab.h \
    $$PWD/csf2generator.h \
    $$PWD/csf2file.h \
    $$PWD/csfenvelope.h \
    $$PWD/../Envelope/cadsr.h

SOURCES += \
    $$PWD/enabler/sfreader.cpp \
    $$PWD/enabler/sfnav.cpp \
    $$PWD/enabler/hydra.cpp \
    $$PWD/enabler/enab.cpp \
    $$PWD/csf2generator.cpp \
    $$PWD/csf2file.cpp \
    $$PWD/../Envelope/cadsr.cpp
}

HEADERS += \
    $$PWD/csffilter.h \
    $$PWD/csflfo.h \
    $$PWD/csfoscillator.h

SOURCES += \
    $$PWD/csfoscillator.cpp

