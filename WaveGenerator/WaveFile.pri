!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

INCLUDEPATH += $$PWD

include($$PWD/mp3lib/mp3lib.pri)

SOURCES += $$PWD/cwavefile.cpp \
    $$PWD/iwavefile.cpp

HEADERS += $$PWD/cwavefile.h \
    $$PWD/iwavefile.h
}
