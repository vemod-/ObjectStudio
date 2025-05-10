include(SoftSynthsArcitecture.pri)

TEMPLATE = lib
macx {
    CONFIG += plugin
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        CONFIG += lib_bundle
    }
    contains(DEFINES,MACXSTATICLIBS) {
        CONFIG += staticlib
    }
}
ios {
    contains(DEFINES,BUILD_WITH_FRAMEWORKS) {
        #CONFIG += staticlib # since you're probably building a shared library by default
        #CONFIG -= staticlib
        CONFIG += framework
        CONFIG += lib_bundle
        FRAMEWORK_HEADERS.version = Versions
        FRAMEWORK_HEADERS.files = $${HEADERS}
        FRAMEWORK_HEADERS.path = Headers
        QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
        # If we're static but a bundle, we want to copy files to a
        # framework directory.
        CONFIG(staticlib, shared|staticlib) {
            message("Adding copy for static framework")
            # (using QMAKE_EXTRA_TARGET will be executed before
            # linking, which is too early).
            QMAKE_POST_LINK += mkdir -p ../$${TARGET}.framework/Headers && \
                $$QMAKE_COPY $$PWD/*.h ../$${TARGET}.framework/Headers && \
                $$QMAKE_COPY $$OUT_PWD/../lib$${TARGET}.a ../$${TARGET}.framework/$${TARGET} && \
                $$QMAKE_RANLIB -s ../$${TARGET}.framework/$${TARGET}
        }
    }
    contains(DEFINES,BUILD_WITH_STATIC) {
        CONFIG += staticlib
    }
}
