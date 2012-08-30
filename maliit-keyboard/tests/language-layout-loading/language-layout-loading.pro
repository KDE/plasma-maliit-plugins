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

QMAKE_EXTRA_TARGETS += xml_check
xml_check.commands = \
    xmllint --path "$$IN_PWD/../../data/languages" --noout --dtdvalid "$$IN_PWD/../../data/languages/VirtualKeyboardLayout.dtd" "$$IN_PWD/languages/*.xml"

check.depends += xml_check
