!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

greaterThan(QT_MAJOR_VERSION, 4){
        lessThan(QT_MAJOR_VERSION, 6){
                QT += macextras
        }
}

INCLUDEPATH += $$PWD

LIBS += -lSoftSynthsWidgets

INCLUDEPATH += $$PWD/../SoftSynthsWidgets
INCLUDEPATH += ../../SynthKnob
INCLUDEPATH += ../../QCanvas
INCLUDEPATH += ../MacroBox

##LIBS += -lMacroBox
##INCLUDEPATH += $$PWD/../MacroBox
INCLUDEPATH += ../RtAudioBuffer
INCLUDEPATH += ../PlugInLoader
INCLUDEPATH += ../../QiPhotoRubberband
INCLUDEPATH += ../../QSignalMenu
INCLUDEPATH += ../../SynthPanel

##include($$PWD/../RtAudioBuffer/DesktopComponent.pri)
macx {
    contains(DEFINES,MACXSTATICLIBS) {
        LIBS += -L../ -lPluginLoader
        LIBS += -lz
    }
}

HEADERS += \
    $$PWD/cpluginbox.h ##\
    ##$$PWD/../MacroBox/cmacroboxform.h

SOURCES += \
    $$PWD/cpluginbox.cpp ##\
    ##$$PWD/../MacroBox/cmacroboxform.cpp

##FORMS += \
##    $$PWD/../MacroBox/cmacroboxform.ui

}
