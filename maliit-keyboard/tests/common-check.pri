QMAKE_EXTRA_TARGETS += check
check.target = check

# This enables the maliit library for C++ code
CONFIG += maliit-plugins

qws {
    test_arguments += -qws
}

check.commands = \
    LD_LIBRARY_PATH=$$MALIIT_INSTALL_LIBS:$$[QT_INSTALL_LIBS]:$(LD_LIBRARY_PATH) \
    ./$$TARGET $$test_arguments

check.depends += $$TARGET

LIBS += ../common/libtests-common.a
POST_TARGETDEPS += ../common/libtests-common.a
INCLUDEPATH += ../common
