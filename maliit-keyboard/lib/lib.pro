include(../../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard
TEMPLATE = lib
QT += core gui
DEFINES += MALIIT_PLUGINS_DATA_DIR=\\\"$$MALIIT_PLUGINS_DATA_DIR\\\"

include(renderer/renderer.pri)
include(models/models.pri)
include(glass/glass.pri)
include(logic/logic.pri)
include(parser/parser.pri)
include(languages/languages.pri)

target.path += $$PREFIX/usr/lib
languages.path = $$MALIIT_PLUGINS_DATA_DIR/languages

INSTALLS += target languages

