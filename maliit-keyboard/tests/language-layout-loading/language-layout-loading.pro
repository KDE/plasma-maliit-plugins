include(../../config.pri)
include(../common-check.pri)

TOP_BUILDDIR = $${OUT_PWD}/../../..
TARGET = language-layout-loading
TEMPLATE = app
QT = core testlib

INCLUDEPATH += ../ ../../lib ../../
LIBS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB}
PRE_TARGETDEPS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB}

DEFINES += TEST_DATA_DIR=\\\"$$IN_PWD\\\"

HEADERS += \

SOURCES += \
    main.cpp \

include(../../word-prediction.pri)

