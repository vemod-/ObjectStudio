!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

CONFIG += qt ##x86
##CONFIG -= x86_64 ppc64 ppc
CONFIG += c++17
CONFIG+=sdk_no_version_check
INCLUDEPATH += $$PWD

##DEFINES += MINIMP3_IMPLEMENTATION
##DEFINES += MINIMP3_NO_STDIO

##include($$PWD/mp3lib/mp3lib.pri)

##LIBS += -lavdevice
##LIBS += -lavfilter
##LIBS += -lpostproc

macx {
    contains(DEFINES,FFMPEGLIB) {
        LIBS += -L/usr/local/Cellar/ffmpeg/6.0/lib
        INCLUDEPATH += /usr/local/Cellar/ffmpeg/6.0/include
        LIBS += -lavformat
        LIBS += -lavcodec
        LIBS += -lswresample
        LIBS += -lavutil
    }
    contains(DEFINES,QTMMLIB) {
        QT += multimedia
    }
}

ios {
    contains(DEFINES,FFMPEGLIB) {
        QMAKE_FRAMEWORKPATH += $$QT_INSTALL_LIBS/ffmpeg
        LIBS += -framework libavformat
        LIBS += -framework libavcodec
        LIBS += -framework libswresample
        LIBS += -framework libavutil
    }
    contains(DEFINES,QTMMLIB) {
        #LIBS += -F/Users/thomasallin/Qt/6.8.1/ios/lib/QtMultimedia.framework
        #INCLUDEPATH += /Users/thomasallin/Qt/6.8.1/ios/lib/QtMultimedia.framework/Headers
        QT += multimedia
        #QMAKE_LFLAGS -= -framework ApplicationServices
        #LIBS += -framework AVFoundation \
        #        -framework CoreMedia \
        #        -framework CoreAudio \
        #        -framework AudioToolbox
    }
}

ios {
    contains(DEFINES,FFMPEGLIB) {
        INCLUDEPATH += /usr/local/Cellar/ffmpeg/6.0/include
    }
}

##LIBS += -lavprobe

##INCLUDEPATH += /usr/local/Cellar/ffmpeg/6.0/include/libavcodec
##INCLUDEPATH += /usr/local/Cellar/ffmpeg/6.0/include/libavformat
##INCLUDEPATH += /usr/local/Cellar/ffmpeg/6.0/include/libpostproc
##INCLUDEPATH += /usr/local/Cellar/ffmpeg/6.0/include/libavutil
##INCLUDEPATH += /usr/local/Cellar/ffmpeg/6.0/include/libavdevice
##INCLUDEPATH += /usr/local/Cellar/ffmpeg/6.0/include/libavprobe

SOURCES += $$PWD/cwavefile.cpp \
    $$PWD/iwavefile.cpp

HEADERS += $$PWD/cwavefile.h \
    $$PWD/iwavefile.h
}

contains(DEFINES,FFMPEGLIB) {

    HEADERS += \
        $$PWD/audiorw.h
        ##$$PWD/cminimp3.h \
        ##$$PWD/minimp3.h \
        ##$$PWD/minimp3_ex.h

    SOURCES += \
        $$PWD/audiorw.cpp
}
contains(DEFINES,QTMMLIB) {
    HEADERS += \
        $$PWD/qaudiorw.h
}

