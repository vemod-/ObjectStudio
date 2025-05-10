!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/cuimap.ui

HEADERS += \
    $$PWD/cuimap.h

SOURCES += \
    $$PWD/cuimap.cpp

}
