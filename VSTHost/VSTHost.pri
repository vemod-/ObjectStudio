!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

greaterThan(QT_MAJOR_VERSION, 4): QT += macextras

LIBS += -framework Cocoa -framework Carbon
macx:LIBS += -framework AppKit -framework Carbon ## -framework CoreGraphics

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../SoftSynthClasses
INCLUDEPATH += $$PWD/../PluginLoader
include($$PWD/../../EventHandlers/EventHandlers.pri)
include($$PWD/../../QSignalMenu/QSignalMenu.pri)

SOURCES += $$PWD/cvsthost.cpp \
    $$PWD/cvstform.cpp
OBJECTIVE_SOURCES += $$PWD/cmacwindow.mm \
    $$PWD/cvsthostclass.mm \
    $$PWD/macstrings.mm \
    $$PWD/iaudiopluginhost.mm

HEADERS += $$PWD/cvsthost.h \
    $$PWD/cvsthostclass.h \
    $$PWD/aeffectx.h \
    $$PWD/AEffect.h \
    $$PWD/cmacwindow.h \
    $$PWD/macstrings.h \
    $$PWD/cvstform.h \
    $$PWD/iaudiopluginhost.h

FORMS += $$PWD/cvstform.ui
}

