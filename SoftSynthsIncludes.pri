!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

include(SoftSynthsArcitecture.pri)

macx {
    TEMPLATE = lib
    CONFIG += plugin
    DEFINES += __MACOSX_CORE__
}
ios {
    TEMPLATE = lib
    contains(DEFINES,BUILD_WITH_STATIC) {
        message("BUILD_WITH_STATIC")
        CONFIG += staticlib
        CONFIG -= qios
        LIBS -= -lqios
    }
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        message("BUILD_WITH_FRAMEWORKS")
        CONFIG -= staticlib
        CONFIG += shared
        QTPLUGIN -= QIOSIntegrationPlugin
        NO_PKGCONFIG_QT_PLUGINS = 1

        TEMPLATE = lib
        CONFIG += shared
        QMAKE_FRAMEWORK_BUNDLE = 1
        QMAKE_LFLAGS += -dynamiclib

        QT -= gui widgets core
        #QMAKE_LFLAGS_PLUGIN += -dynamiclib -Wl#,-no_entry
        # Specifik arkitektur för device och simulator
        CONFIG(debug, debug|release) {
            QMAKE_CXXFLAGS += -g
        }
        # Arkitektur och bitcode för simulator/enhet
        simulator | iossimulator {
            QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
            QMAKE_CXXFLAGS += -arch x86_64 -arch arm64
            QMAKE_LFLAGS += -arch x86_64 -arch arm64
        }
        device {
            QMAKE_APPLE_DEVICE_ARCHS = arm64
            QMAKE_CXXFLAGS += -arch arm64
            QMAKE_LFLAGS += -arch arm64
        }
        LIBS -= -lqios
        QMAKE_CFLAGS += -fembed-bitcode
    }
    #LIBS -= -lqios
    # Länka mot CoreFoundation och andra standard-frameworks
    #LIBS += -framework Foundation -framework UIKit -framework CoreFoundation
}

macx {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
        message("BUILD_WITH_FRAMEWORKS 1")
    } else {
        LIBS += -L../ -lSoftSynthsClasses
    }
}
ios {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
        message("BUILD_WITH_FRAMEWORKS 1")
    }
    contains(DEFINES,BUILD_WITH_STATIC) {
        LIBS += -L../ -lSoftSynthsClasses
        LIBS += -L../ -lPlugInLoader
        INCLUDEPATH += ../PluginLoader
        message("BUILD_WITH_STATIC 1")
    }
}

INCLUDEPATH += ../SoftSynthsClasses
INCLUDEPATH += ../../QDomLite

DEFINES += devicelib=lib$${TARGET}
DEFINES += devicename=\\\"$${TARGET}\\\"
DEFINES += deviceclass=C$${TARGET}
DEFINES += headerfile=$$lower(\\\"c$${TARGET}.h\\\")

SOURCES += ../cdeviceclass.cpp
HEADERS += ../cdeviceclass.h

DESTDIR = ../

}

HEADERS +=


