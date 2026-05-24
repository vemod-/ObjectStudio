TARGET = StereoContainer

include(../SoftSynthsIncludes.pri)

greaterThan(QT_MAJOR_VERSION, 4){
    QT +=  widgets
    lessThan(QT_MAJOR_VERSION, 6) {
        QT += macextras
    }
}

DEFINES += STEREOCONTAINER_LIBRARY

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

INCLUDEPATH += $$PWD/../PluginLoader
INCLUDEPATH += $$PWD/../../QSignalMenu

SOURCES += cstereocontainer.cpp \
    cstereocontainerbase.cpp

HEADERS +=  $$PWD/../../QSignalMenu/qsignalmenu.h \
            cstereocontainer.h \
            cstereocontainerbase.h
