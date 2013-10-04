!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

SOURCES +=     $$PWD/MP3Play.cpp \
    $$PWD/windowb.c \
    $$PWD/window.c \
    $$PWD/sbtb.c \
    $$PWD/sbt.c \
    $$PWD/mp3dec.c \
    $$PWD/l3side.c \
    $$PWD/l3sf.c \
    $$PWD/l3quant.c \
    $$PWD/l3msis.c \
    $$PWD/l3init.c \
    $$PWD/l3hybrid.c \
    $$PWD/l3huff.c \
    $$PWD/l3dec.c \
    $$PWD/l3alias.c \
    $$PWD/l2init.c \
    $$PWD/l2dec.c \
    $$PWD/l1init.c \
    $$PWD/l1dec.c \
    $$PWD/imdct.c \
    $$PWD/fdct.c \
    $$PWD/bstream.c 

HEADERS +=     $$PWD/MP3Play.h \
    $$PWD/include/mp3dec.h \
    $$PWD/include/layer3.h \
    $$PWD/include/l3huff.h \
    $$PWD/include/bstream.h \
    $$PWD/debug.h 
}
