#-------------------------------------------------
#
# Project created by QtCreator 2017-06-14T23:22:11
#
#-------------------------------------------------

TARGET = SoftSynthsWidgets
DESTDIR = ../

include(../SoftSynthsFrameworks.pri)

DEFINES += SOFTSYNTHSWIDGETS_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD

macx {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
    } else {
        LIBS += -L../ -lSoftSynthsClasses
    }
}
ios {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        LIBS += -F$$PWD/../SoftSynthsClasses
    }
    contains(DEFINES,BUILD_WITH_STATIC) {
        LIBS += -L$$PWD/../ -lSoftSynthsClasses
    }
}
INCLUDEPATH += $$PWD/../SoftSynthsClasses
INCLUDEPATH += $$PWD/../../QGraphicsViewZoomer

include($$PWD/../../QSignalMenu/QSignalMenu.pri)
include($$PWD/../../QFadingWidget/QFadingWidget.pri)
include($$PWD/../../QiPhotoRubberband/QiPhotoRubberband.pri)
include($$PWD/../../EffectLabel/EffectLabel.pri)
include($$PWD/../../SynthPanel/QSynthPanel.pri)
include($$PWD/../../SynthKnob/QSynthKnob.pri)
include($$PWD/../../SynthSwitch/QSynthSwitch.pri)
include($$PWD/../../LCDLabel/QLCDLabel.pri)
include($$PWD/../../EventHandlers/EventHandlers.pri)
include($$PWD/../../QSynthCheckbox/QSynthCheckbox.pri)
include($$PWD/../../SynthSlider/QSynthSlider.pri)
include($$PWD/../../ToggleButton/QToggleButton.pri)
include($$PWD/../RtAudioBuffer/PeakMeter.pri)
include($$PWD/../../QsynthButtonPanel/QSynthButtonPanel.pri)
include($$PWD/../RtAudioBuffer/DesktopComponent.pri)

HEADERS += \
    ../../QGraphicsViewZoomer/qgraphicsviewzoomer.h \
    ../RtAudioBuffer/cthreadedfunction.h \
    ceditmenu.h \
    cparametersmenu.h \
    cprojectapp.h \
    crecentmenu.h \
    cresourceinitializer.h \
    ctimeline.h \
    ctimelineedit.h \
    ctimelineslider.h \
    cundomenu.h \
        ##softsynthswidgets.h \
    ../MacroBox/cmacroboxform.h

FORMS += \
    ../MacroBox/cmacroboxform.ui \
    ctimelineedit.ui

SOURCES += \
    ../MacroBox/cmacroboxform.cpp \
    ../RtAudioBuffer/cthreadedfunction.cpp \
    cprojectapp.cpp \
    #ctimeline.cpp \
    cresourceinitializer.cpp \
    ctimelineedit.cpp \
    ctimelineslider.cpp

