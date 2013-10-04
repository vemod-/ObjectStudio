!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

INCLUDEPATH += $$PWD
include($$PWD/../../QCanvas/QCanvas.pri)

SOURCES += $$PWD/cstereopeak.cpp \
    $$PWD/cpeakcontrol.cpp \
    $$PWD/cdbscale.cpp

HEADERS += $$PWD/cstereopeak.h \
    $$PWD/cpeakcontrol.h \
    $$PWD/cdbscale.h

FORMS += $$PWD/cstereopeak.ui \
    $$PWD/cpeakcontrol.ui \
    $$PWD/cdbscale.ui
}
