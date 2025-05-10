!contains(PROFILES,$$_FILE_){
PROFILES+=$$_FILE_

INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/cautomationlane.cpp

HEADERS += \
    $$PWD/cautomationlane.h

FORMS += \
    $$PWD/cautomationlane.ui
}
