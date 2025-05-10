macx {
    DEFINES += FFMPEGLIB
    #DEFINES += QTMMLIB

    QT_INSTALL_LIBS = /Users/thomasallin/Qt/6.8.1/macos/lib

    QMAKE_MACOSX_DEPLOYMENT_TARGET = 13.0
    #DEFINES += MACXSTATICLIBS
    #DEFINES += BUILD_WITH_FRAMEWORKS
}

ios {
    QMAKE_XSPEC = macx-ios-clang
    QMAKE_MAC_SDK = iphoneos

    #DEFINES += FFMPEGLIB
    #DEFINES += QTMMLIB
    QT_INSTALL_LIBS = /Users/thomasallin/Qt/6.8.1/ios/lib
    QMAKE_FRAMEWORKPATH += /Users/thomasallin/Qt/6.8.1/ios/lib
    #DEFINES += BUILD_WITH_FRAMEWORKS
    DEFINES += BUILD_WITH_STATIC
    #DEFINES += BUILD_WITH_DYNAMIC

    # Deployment target and device family
    QMAKE_IOS_DEPLOYMENT_TARGET = 16.0
    QMAKE_APPLE_TARGETED_DEVICE_FAMILY = 2 #ipad only
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        message("BUILD_WITH_FRAMEWORKS")
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

        #LIBS -= -lqios
        #QMAKE_LFLAGS -= -framework ApplicationServices

        # Bitcode (kan tas bort om onödigt)
        QMAKE_CFLAGS += -fembed-bitcode
    }
}

#DEFINES += QT_USE_QSTRINGBUILDER
DEFINES += QT_USE_FAST_CONCATENATION
#DEFINES += QT_USE_FAST_OPERATOR_PLUS
#DEFINES += _LIBCPP_HAS_NO_WIDE_CHARACTERS

CONFIG+=sdk_no_version_check

#QMAKE_OBJECTIVE_CFLAGS += -fobjc-arc

##QMAKE_CXXFLAGS -= -O2
##QMAKE_CXXFLAGS += -O3
##QMAKE_CXXFLAGS += -march=native
#QMAKE_CXXFLAGS += -mcpu=apple-m1

CONFIG += qt ##x86
##CONFIG -= x86_64 ppc64 ppc
CONFIG += c++17

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4){
    QT += widgets
    lessThan(QT_MAJOR_VERSION, 6) {
        QT += macextras
    }
}

greaterThan(QT_MAJOR_VERSION,5) {
    QT += core5compat
}

macx {
    LIBS += -framework AppKit
}

ios {
    LIBS += -framework UIKit
}

message("End of SoftSynthsArcitecture.pri")
