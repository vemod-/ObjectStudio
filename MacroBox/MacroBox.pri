!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

greaterThan(QT_MAJOR_VERSION, 4){
        lessThan(QT_MAJOR_VERSION, 6){
                QT += macextras
        }
}

INCLUDEPATH += $$PWD

LIBS += -lSoftSynthsWidgets

INCLUDEPATH += $$PWD/../PlugInLoader
INCLUDEPATH += $$PWD/../SoftSynthsWidgets
INCLUDEPATH += ../../QiPhotoRubberband
INCLUDEPATH += ../../QSignalMenu
INCLUDEPATH += ../../SynthPanel
INCLUDEPATH += ../../SynthKnob
INCLUDEPATH += ../../QCanvas

##include($$PWD/../RtAudioBuffer/DesktopComponent.pri)
##LIBS += -lSoftSynthsWidgets
INCLUDEPATH += $$PWD/../RtAudioBuffer

macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lPluginLoader
        LIBS += -lz
    }
}

SOURCES += $$PWD/cmacrobox.cpp

HEADERS += $$PWD/cmacrobox.h

}
