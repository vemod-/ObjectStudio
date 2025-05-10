!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

greaterThan(QT_MAJOR_VERSION,5) {
    QT += core5compat
}

##LIBS += -lPluginLoader
LIBS += -lPluginLoader
INCLUDEPATH += ../SoftSynthsClasses

#LIBS += -framework AppKit

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../PluginLoader
INCLUDEPATH += $$PWD/../../QDomLite

##LIBS += -lSoftSynthsWidgets
INCLUDEPATH += $$PWD/../SoftSynthsClasses
INCLUDEPATH += $$PWD/../../QSignalMenu \
    $$PWD/../../QiPhotoRubberband \
    $$PWD/../../QFadingWidget \
    $$PWD/../../EffectLabel \
    $$PWD/../../SynthPanel \
    $$PWD/../../SynthKnob \
    $$PWD/../../SynthSwitch \
    $$PWD/../../LCDLabel \
    $$PWD/../../EventHandlers \
    $$PWD/../../QSynthCheckbox \
    $$PWD/../../ObjectComposerXML

##INCLUDEPATH += $$PWD/../../QiPhotoRubberband

##include($$PWD/../../QSignalMenu/QSignalMenu.pri)
##include($$PWD/../../QFadingWidget/QFadingWidget.pri)
##include($$PWD/../../QiPhotoRubberband/QiPhotoRubberband.pri)
##include($$PWD/../../EffectLabel/EffectLabel.pri)
##include($$PWD/../../SynthPanel/QSynthPanel.pri)
##include($$PWD/../../SynthKnob/QSynthKnob.pri)
##include($$PWD/../../SynthSwitch/QSynthSwitch.pri)
##include($$PWD/../../LCDLabel/QLCDLabel.pri)
##include($$PWD/../../EventHandlers/EventHandlers.pri)$$PWD/
##include($$PWD/../../QSynthCheckbox/QSynthCheckbox.pri)
include($$PWD/UIMap.pri)
include($$PWD/AutomationLane.pri)
##include($$PWD/../../quazip/quazip.pri)
include($$PWD/../../Projectpage/projectpage.pri)

SOURCES += $$PWD/cparameterscomponent.cpp \
    $$PWD/cknobcontrol.cpp \
    $$PWD/cjackscomponent.cpp \
    $$PWD/cdesktopcontainer.cpp \
    $$PWD/cdesktopcomponent.cpp \
    $$PWD/../../ObjectComposerXML/qmacsplitter.cpp

HEADERS += $$PWD/cparameterscomponent.h \
    $$PWD/cknobcontrol.h \
    $$PWD/cjackscomponent.h \
    $$PWD/cdesktopcontainer.h \
    $$PWD/cdesktopcomponent.h \
    $$PWD/../../ObjectComposerXML/qmacsplitter.h

FORMS += $$PWD/cparameterscomponent.ui \
    $$PWD/cknobcontrol.ui \
    $$PWD/cjackscomponent.ui \
    $$PWD/cdesktopcontainer.ui \
    $$PWD/cdesktopcomponent.ui

FORMS += \
    $$PWD/cparameterscontainer.ui \
    $$PWD/crackcontainer.ui

HEADERS += \
    $$PWD/cparameterscontainer.h \
    $$PWD/cconnectionhelper.h \
    $$PWD/crackcontainer.h

SOURCES += \
    $$PWD/cparameterscontainer.cpp \
    $$PWD/cconnectionhelper.cpp \
    $$PWD/crackcontainer.cpp

}

RESOURCES += \
    $$PWD/desktopresources.qrc

DISTFILES += \
    $$PWD/../../SynthPanel/Aluminium Verde Tile.bmp \
    $$PWD/../../SynthPanel/Brushed Aluminium 3 Tile.bmp \
    $$PWD/../../SynthPanel/Sky Aluminium Tile.bmp \
    $$PWD/../../SynthPanel/Stainless Aluminium Tile.bmp \
    $$PWD/../../SynthPanel/Brushed Aluminium Tile.bmp \
    $$PWD/../../SynthPanel/Aluminium Verde Tile.bmp \
    $$PWD/../../SynthPanel/Brushed Aluminium 3 Tile.bmp \
    $$PWD/../../SynthPanel/Sky Aluminium Tile.bmp \
    $$PWD/../../SynthPanel/Stainless Aluminium Tile.bmp \
    $$PWD/../../SynthPanel/Brushed Aluminium Tile.bmp
