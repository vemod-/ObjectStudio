#-------------------------------------------------
#
# Project created by QtCreator 2011-10-02T14:54:32
#
#-------------------------------------------------

TARGET = Envelope
TEMPLATE = lib

include(../SoftSynthsIncludes.pri)
include(../../QCanvas/QCanvas.pri)

DEFINES += ENVELOPE_LIBRARY

SOURCES += cadsrwidget.cpp

HEADERS += cadsrwidget.h

SOURCES += cenvelope.cpp \
    cadsr.cpp \
    cadsrcontrol.cpp \
    cenevelopeform.cpp

HEADERS += cenvelope.h \
    cadsr.h \
    cadsrcontrol.h \
    cenevelopeform.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE9618BEA
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = Envelope.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

FORMS += \
    cadsrcontrol.ui \
    cenevelopeform.ui \
    cadsrwidget.ui












