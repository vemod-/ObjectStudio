INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../WaveBank

HEADERS += \
    $$PWD/enabler/win_mem.h \
    $$PWD/enabler/sfreader.h \
    $$PWD/enabler/sfnav.h \
    $$PWD/enabler/sflookup.h \
    $$PWD/enabler/sfenum.h \
    $$PWD/enabler/sfdetect.h \
    $$PWD/enabler/sfdata.h \
    $$PWD/enabler/riff.h \
    $$PWD/enabler/omega.h \
    $$PWD/enabler/hydra.h \
    $$PWD/enabler/enab.h \
    $$PWD/enabler/emuerrs.h \
    $$PWD/enabler/datatype.h \
    $$PWD/csf2generator.h \
    $$PWD/csf2file.h

SOURCES += \
    $$PWD/enabler/win_mem.cpp \
    $$PWD/enabler/sfreader.cpp \
    $$PWD/enabler/sfnav.cpp \
    $$PWD/enabler/sflookup.cpp \
    $$PWD/enabler/sfdetect.cpp \
    $$PWD/enabler/riff.cpp \
    $$PWD/enabler/omega.cpp \
    $$PWD/enabler/hydra.cpp \
    $$PWD/enabler/enab.cpp \
    $$PWD/csf2generator.cpp \
    $$PWD/csf2file.cpp

