LIBS += -L $$PWD/../ -lPluginLoader
macx:LIBS += -framework AppKit -framework Carbon

include($$PWD/../../SynthKnob/QSynthKnob.pri)

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../PluginLoader
INCLUDEPATH += $$PWD/../../QDomLite
INCLUDEPATH += $$PWD/../../QiPhotoRubberband

include($$PWD/../../QSignalMenu/QSignalMenu.pri)
include($$PWD/../../QiPhotoRubberband/QiPhotoRubberband.pri)
include($$PWD/../../EffectLabel/EffectLabel.pri)
include($$PWD/../../SynthPanel/QSynthPanel.pri)
include($$PWD/../../LCDLabel/QLCDLabel.pri)

SOURCES += $$PWD/cparameterscomponent.cpp \
    $$PWD/cknobcontrol.cpp \
    $$PWD/cjackscomponent.cpp \
    $$PWD/cdevicelist.cpp \
    $$PWD/cdesktopcontainer.cpp \
    $$PWD/cdesktopcomponent.cpp

HEADERS += $$PWD/cparameterscomponent.h \
    $$PWD/cknobcontrol.h \
    $$PWD/cjackscomponent.h \
    $$PWD/cdevicelist.h \
    $$PWD/cdesktopcontainer.h \
    $$PWD/cdesktopcomponent.h

FORMS += $$PWD/cparameterscomponent.ui \
    $$PWD/cknobcontrol.ui \
    $$PWD/cjackscomponent.ui \
    $$PWD/cdesktopcontainer.ui \
    $$PWD/cdesktopcomponent.ui
