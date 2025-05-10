#-------------------------------------------------
#
# Project created by QtCreator 2011-09-26T08:47:24
#
#-------------------------------------------------

TARGET = SoftSynthsClasses
DESTDIR = ../

include(../SoftSynthsFrameworks.pri)

include(../../QDomLite/QDomLite.pri)

macx {
    LIBS += -L ../ -lPlugInLoader
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../PluginLoader.framework
    } else {
        LIBS += -L ../ -lPlugInLoader
    }
}
ios {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -framework PluginLoader
    }
    contains(DEFINES,BUILD_WITH_STATIC) {
        LIBS += $$PWD/../ -lPluginLoader
    }
}

INCLUDEPATH += ../PluginLoader

SOURCES += \
    ijack.cpp \
    cpitchtextconvert.cpp \
    cpresets.cpp \
    csoftsynthsform.cpp \
    caudiobuffer.cpp \
    csounddevice.cpp \
    idevicebase.cpp \
    ihost.cpp \
    imidiparser.cpp \
    isoundgenerator.cpp \
    cdevicelist.cpp \
    cprogrambank.cpp \
    cfileparameter.cpp \
    cdevicecontainer.cpp

HEADERS += \##softsynthsclasses.h \
    cautomationplayer.h \
    cmidibuffer.h \
    idevicebase.h \
    ijackbase.h \
    caudiobuffer.h \
    ijack.h \
    ihost.h \
    idevice.h \
    softsynthsdefines.h \
    isoundgenerator.h \
    csinglemap.h \
    cringbuffer.h \
    cpitchtextconvert.h \
    csimplebuffer.h \
    cfloatbuffer.h \
    cmseccounter.h \
    imidiparser.h \
    cdevicelist.h \
    cjackcollection.h \
    csounddevice.h \
    cfreqglider.h \
    cpresets.h \
    csoftsynthsform.h \
    cparameter.h \
    cprogrambank.h \
    cfileidentifier.h \
    cfileparameter.h \
    cdevicecontainer.h \
    csyncbuffer.h \
    cfastcircularbuffer.h \
    cspectralwindow.h \
    cfft.h \
    cvoltagemodulator.h

OBJECTIVE_SOURCES += \
    idevice.mm




