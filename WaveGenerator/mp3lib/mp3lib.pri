!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_
}

CONFIG += c++17

SOURCES += \
    $$PWD/cmp3dec.cpp \
    $$PWD/mp3play.cpp \
    $$PWD/imp3layer.cpp \
    $$PWD/clayer1.cpp \
    $$PWD/clayer2.cpp \
    $$PWD/clayer3.cpp \
    $$PWD/l3huff.cpp \
    $$PWD/l3sf.cpp

HEADERS += \
    $$PWD/bstream.h \
    $$PWD/cmp3dec.h \
    $$PWD/mp3play.h \
    $$PWD/imp3layer.h \
    $$PWD/clayer1.h \
    $$PWD/clayer2.h \
    $$PWD/clayer3.h \
    $$PWD/sbt.h \
    $$PWD/l3alias.h \
    $$PWD/fdct.h \
    $$PWD/l3msis.h \
    $$PWD/l3huff.h \
    $$PWD/l3hybrid.h \
    $$PWD/imdct.h \
    $$PWD/l3sf.h \
    $$PWD/cwindowing.h
