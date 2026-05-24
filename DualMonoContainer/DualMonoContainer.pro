TARGET = DualMonoContainer

include(../SoftSynthsIncludes.pri)

greaterThan(QT_MAJOR_VERSION, 4){
    QT +=  widgets
    lessThan(QT_MAJOR_VERSION, 6) {
        QT += macextras
    }
}

DEFINES += DUALMONOCONTAINER_LIBRARY

DEFINES += DUALMONO

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
INCLUDEPATH += $$PWD/../StereoContainer

SOURCES += cdualmonocontainer.cpp \
    $$PWD/../StereoContainer/cstereocontainerbase.cpp

HEADERS +=  $$PWD/../../QSignalMenu/qsignalmenu.h \
            cdualmonocontainer.h \
            $$PWD/../StereoContainer/cstereocontainerbase.h
