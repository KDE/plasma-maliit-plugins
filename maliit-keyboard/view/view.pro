include(../../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard-view
TEMPLATE = lib
QT += core gui
DEFINES += MALIIT_PLUGINS_DATA_DIR=\\\"$${MALIIT_PLUGINS_DATA_DIR}\\\"

ORG_MALIIT_DIR = "$${MALIIT_PLUGINS_DATA_DIR}/org/maliit"
DEFINES += MALIIT_KEYBOARD_IMAGES_DIR=\\\"$${ORG_MALIIT_DIR}/images\\\"
DEFINES += MALIIT_KEYBOARD_STYLES_DIR=\\\"$${ORG_MALIIT_DIR}/styles\\\"

INCLUDEPATH=../lib

include(renderer/renderer.pri)
include(glass/glass.pri)

target.path += $$PREFIX/usr/lib
INSTALLS += target

