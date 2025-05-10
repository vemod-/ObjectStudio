#TEMPLATE = lib
DEFINES += OBJECTCOMPOSER_LIBRARY


TARGET = ObjectComposer

include(../SoftSynthsIncludes.pri)
include(../../ObjectComposerXML/ObjectComposerXML.pri)

INCLUDEPATH += $$PWD

SOURCES += \
    cobjectcomposer.cpp \
    cobjectcomposerform.cpp

HEADERS += \
    cobjectcomposer.h \
    cobjectcomposerform.h

FORMS += \
    cobjectcomposerform.ui

