#-------------------------------------------------
#
# Project created by QtCreator 2011-09-18T22:59:50
#
#-------------------------------------------------
include(../SoftSynthsArcitecture.pri)

QT       += concurrent

TEMPLATE = app
DESTDIR    = ../
TARGET = objectstudio

ios {
    LIBS += \
        -framework QtCore \
        -framework QtGui \
        -framework QtWidgets \
        -framework QtConcurrent \
        -framework QtPrintSupport \
        -framework QtCore5Compat

    LIBS += -framework CoreMidi -framework CoreAudio -framework CoreFoundation
    LIBS += -framework AudioToolbox
    LIBS += -framework AVFoundation
}

macx {
    LIBS += -framework CoreMidi -framework CoreAudio -framework CoreFoundation
    LIBS += -framework AudioUnit -framework AudioToolbox -framework CoreAudioKit
}


macx {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
        LIBS += -F$$PWD/../PluginLoader
        LIBS += -F$$PWD/../WaveGenerator
        LIBS += -F$$PWD/../SoftSynthsWidgets
    } else {
        LIBS += -L../ -lSoftSynthsClasses
        LIBS += -L../ -lPluginLoader
        LIBS += -L../ -lWaveGenerator
        LIBS += -L../ -lSoftSynthsWidgets
    }
}

ios {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
        LIBS += -F$$PWD/../PluginLoader
        LIBS += -F$$PWD/../WaveGenerator
        LIBS += -F$$PWD/../SoftSynthsWidgets
    }
    contains(DEFINES,BUILD_WITH_STATIC) {
        LIBS += -L../ -lSoftSynthsClasses
        LIBS += -L../ -lPluginLoader
        LIBS += -L../ -lWaveGenerator
        LIBS += -L../ -lSoftSynthsWidgets
        LIBS += -L../ -lSF2Generator
        LIBS += -L../ -lWaveBank
        LIBS += -L../ -lMIDIFileReader
    }
}

INCLUDEPATH += ../SoftSynthsClasses

ios {
    message(start forcing libs to load)
    EXCLUDED_LIBS += libSoftSynthsClasses.a
    EXCLUDED_LIBS += libSoftSynthsWidgets.a
    EXCLUDED_LIBS += libPluginLoader.a
    EXCLUDED_LIBS += libWaveBank.a
    EXCLUDED_LIBS += libWaveGenerator.a
    EXCLUDED_LIBS += libSF2Generator.a
    EXCLUDED_LIBS += libMIDIFileReader.a

    PLUGIN_HEADER = $$PWD/generated_plugin_registration.h
    PLUGIN_LIBS = $$files($$OUT_PWD/../lib*.a)

    for(lib, PLUGIN_LIBS) {
        LIB_NAME = $$basename(lib)
            !contains(EXCLUDED_LIBS, $$basename(lib)) {
            # Ta bort "lib"-prefixet för korrekt länkning
            LIB_NAME = $$replace(LIB_NAME, "^lib", "")
            LIB_NAME = $$replace(LIB_NAME, "\\.a$", "")
            LIBS += -L$$PWD/.. -l$${LIB_NAME}
        }
    }
    message("Linked libraries: $$LIBS")

    pound = $$system(printf $$system_quote(\43))

    # Rensa befintlig header-fil och skriv standardhuvud
    system(echo \"$${pound}ifndef CIOSPLUGINREGISTRATION_H\" > $$PLUGIN_HEADER)
    system(echo \"$${pound}define CIOSPLUGINREGISTRATION_H\" >> $$PLUGIN_HEADER)
    system(echo \"\" >> $$PLUGIN_HEADER)

    # Generera extern deklaration för varje bibliotek
    for(lib, PLUGIN_LIBS) {
        !contains(EXCLUDED_LIBS, $$basename(lib)) {
            LIB_NAME = $$basename(lib)
            LIB_NAME = $$replace(LIB_NAME, "\\.a$", "")
            FUNC_NAME = $$replace(LIB_NAME, "^lib", "registerPlugin_lib")
            system(echo \"extern \\\"C\\\" void $${FUNC_NAME}();\" >> $$PLUGIN_HEADER)
        }
    }

    # Skapa funktionskroppen för registrering
    system(echo \"\" >> $$PLUGIN_HEADER)
    system(echo \"void IOSPluginRegistration() {\" >> $$PLUGIN_HEADER)

    for(lib, PLUGIN_LIBS) {
        !contains(EXCLUDED_LIBS, $$basename(lib)) {
            LIB_NAME = $$basename(lib)
            LIB_NAME = $$replace(LIB_NAME, "\\.a$", "")
            FUNC_NAME = $$replace(LIB_NAME, "^lib", "registerPlugin_lib")
            system(echo \"    $${FUNC_NAME}();\" >> $$PLUGIN_HEADER)
        }
    }

    # Avsluta header-filen
    system(echo \"}\" >> $$PLUGIN_HEADER)
    system(echo \"\" >> $$PLUGIN_HEADER)
    system(echo \"$${pound}endif // CIOSPLUGINREGISTRATION_H\" >> $$PLUGIN_HEADER)
}

INCLUDEPATH += $$PWD/../../QDomLite

include($$PWD/../../Projectpage/projectpage.pri)
include($$PWD/../../QFadingWidget/qfadingwidget.pri)

##include(../RtAudioBuffer/DesktopComponent.pri)
##include(../WaveGenerator/WaveFile.pri)
##LIBS += -lMacroBox
INCLUDEPATH += ../PlugInLoader

INCLUDEPATH += ../SoftSynthsWidgets
INCLUDEPATH += ../../QiPhotoRubberband
INCLUDEPATH += ../../QSignalMenu
INCLUDEPATH += ../../SynthPanel
INCLUDEPATH += ../../SynthKnob
INCLUDEPATH += ../../QCanvas

INCLUDEPATH += ../WaveGenerator

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L/usr/local/Cellar/ffmpeg/6.0/lib
        LIBS += -lavformat
        LIBS += -lavcodec
        LIBS += -lswresample
        LIBS += -lavutil
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
        QT += multimedia
        #LIBS += -F /Users/thomasallin/Qt/6.8.1/ios/lib/ -framework QtMultimedia
        #LIBS += -F /Users/thomasallin/Qt/6.8.1/ios/lib/ -framework QtMultimediaWidgets
        #LIBS += -framework /Users/thomasallin/Qt/6.8.1/ios/lib/QtMultimedia
        #QMAKE_LFLAGS -= -framework ApplicationServices
        #LIBS += -framework AVFoundation
        #LIBS += -framework CoreMedia
        #LIBS +=  -framework CoreAudio
        #LIBS += -framework AudioToolbox
    }
}

macx {
    DEFINES += __MACOSX_CORE__
}
ios  {
    DEFINES += __MINIAUDIO__
    DEFINES += __NOMIDI__
}

DEFINES += _UNICODE

SOURCES += main.cpp\
cthreadedfunction.cpp \
mainwindow.cpp \
corebuffer.cpp \
cpresetsform.cpp
!contains(DEFINES,__NOMIDI__) {
    SOURCES +=  ../../../../../../Library/Developer/Library/rtmidi-6.0.0/RtMidi.cpp
}
macx {
    SOURCES +=    ../../../../../../Library/Developer/Library/rtaudio-6.0.1/RtAudio.cpp
}
ios {
    OBJECTIVE_SOURCES += \
    miniaudio.mm
}

OBJECTIVE_SOURCES +=     ccaffeine.mm

HEADERS  += mainwindow.h \
corebuffer.h \
ccaffeine.h \
cpresetsform.h \
cthreadedfunction.h
!contains(DEFINES,__NOMIDI__) {
    HEADERS += ../../../../../../Library/Developer/Library/rtmidi-6.0.0/RtMidi.h
}
macx {
    HEADERS  +=    ../../../../../../Library/Developer/Library/rtaudio-6.0.1/RtAudio.h
}
ios {
    HEADERS  +=    miniaudio.h
}

FORMS    += mainwindow.ui \
    cpresetsform.ui

RESOURCES += Resources.qrc

DISTFILES += \
    busyindicator.qml \
    ocgrey.png \
    ocicon.png \
    ocicon.icns

macx {
    QMAKE_INFO_PLIST = Info.plist

    OUT_PWD_APP_PATH = $$OUT_PWD/../objectstudio.app

    # Skapa MacOS-mappen om den inte finns
    QMAKE_POST_LINK += mkdir -p $$OUT_PWD_APP_PATH/Contents/MacOS/; \
        cp -R $$OUT_PWD/../*.dylib $$OUT_PWD_APP_PATH/Contents/MacOS/;

    files = $$files($$OUT_PWD_APP_PATH/Contents/MacOS/*.dylib)
    for (file,files):QMAKE_POST_LINK += chmod +w $$file;

    # Hantera Qt-frameworks
    QMAKE_POST_LINK += mkdir -p $$OUT_PWD_APP_PATH/Contents/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtCore.framework $$OUT_PWD_APP_PATH/Contents/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtGui.framework $$OUT_PWD_APP_PATH/Contents/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtWidgets.framework $$OUT_PWD_APP_PATH/Contents/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtConcurrent.framework $$OUT_PWD_APP_PATH/Contents/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtPrintSupport.framework $$OUT_PWD_APP_PATH/Contents/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtCore5Compat.framework $$OUT_PWD_APP_PATH/Contents/Frameworks/;

    # Kopiera FFMpeg-bibliotek
    QMAKE_POST_LINK += cp -R -L $$QT_INSTALL_LIBS/libavformat.dylib $$OUT_PWD_APP_PATH/Contents/MacOS/;
    QMAKE_POST_LINK += cp -R -L $$QT_INSTALL_LIBS/libavcodec.dylib $$OUT_PWD_APP_PATH/Contents/MacOS/;
    QMAKE_POST_LINK += cp -R -L $$QT_INSTALL_LIBS/libswresample.dylib $$OUT_PWD_APP_PATH/Contents/MacOS/;
    QMAKE_POST_LINK += cp -R -L $$QT_INSTALL_LIBS/libavutil.dylib $$OUT_PWD_APP_PATH/Contents/MacOS/;

    # Kopiera z-bibliotek
    QMAKE_POST_LINK += cp -R -L /Applications/Audacity.app/Contents/Frameworks/libz.dylib $$OUT_PWD_APP_PATH/Contents/MacOS/;
}

ios {
    OUT_PWD_APP_PATH = $$OUT_PWD/Debug-iphonesimulator/objectstudio.app

    QMAKE_POST_LINK += mkdir -p $$OUT_PWD_APP_PATH/Frameworks/; \
        cp -R $$OUT_PWD/../*.a $$OUT_PWD_APP_PATH/Frameworks/; \
        cp -R $$OUT_PWD/../*.framework $$OUT_PWD_APP_PATH/Frameworks/;

    QMAKE_POST_LINK += \ #mkdir -p $$OUT_PWD_APP_PATH/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtCore.framework $$OUT_PWD_APP_PATH/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtGui.framework $$OUT_PWD_APP_PATH/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtWidgets.framework $$OUT_PWD_APP_PATH/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtConcurrent.framework $$OUT_PWD_APP_PATH/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtPrintSupport.framework $$OUT_PWD_APP_PATH/Frameworks/; \
        cp -R -L $$QT_INSTALL_LIBS/QtCore5Compat.framework $$OUT_PWD_APP_PATH/Frameworks/;

    contains(DEFINES,FFMPEGLIB) {

        # Kopiera FFMpeg-bibliotek
        QMAKE_POST_LINK += cp -R -L $$QT_INSTALL_LIBS/ffmpeg/libavformat.framework $$OUT_PWD_APP_PATH/Frameworks/;
        QMAKE_POST_LINK += cp -R -L $$QT_INSTALL_LIBS/ffmpeg/libavcodec.framework $$OUT_PWD_APP_PATH/Frameworks/;
        QMAKE_POST_LINK += cp -R -L $$QT_INSTALL_LIBS/ffmpeg/libswresample.framework $$OUT_PWD_APP_PATH/Frameworks/;
        QMAKE_POST_LINK += cp -R -L $$QT_INSTALL_LIBS/ffmpeg/libavutil.framework $$OUT_PWD_APP_PATH/Frameworks/;
    }
    contains(DEFINES,QTMMLIB) {
        QMAKE_POST_LINK += \ #mkdir -p $$OUT_PWD_APP_PATH/Frameworks/; \
            cp -R -L $$QT_INSTALL_LIBS/QtMultimedia.framework $$OUT_PWD_APP_PATH/Frameworks/;
    }
    # Kopiera z-bibliotek
    QMAKE_POST_LINK += cp -R -L /Applications/Audacity.app/Contents/Frameworks/libz.dylib $$OUT_PWD_APP_PATH/Frameworks/;

    QMAKE_RPATHDIR =
    #QMAKE_LFLAGS += -Wl,-rpath,@executable_path/Frameworks
    QMAKE_LFLAGS += -Wl,-rpath,@executable_path/../Frameworks
}

ICON = ocicon.icns
