include(../../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard
TEMPLATE = lib
QT += core gui

include(renderer/renderer.pri)
include(models/models.pri)
include(glass/glass.pri)
include(logic/logic.pri)
include(parser/parser.pri)

target.path += $$PREFIX/usr/lib
INSTALLS += target
DEFINES += MALIIT_PLUGINS_DATA_DIR=\\\"$$MALIIT_PLUGINS_DATA_DIR\\\"
