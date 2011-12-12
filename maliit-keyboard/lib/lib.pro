include(../../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard
TEMPLATE = lib
QT += core gui
DEFINES += MALIIT_PLUGINS_DATA_DIR=\\\"$${MALIIT_PLUGINS_DATA_DIR}\\\"

ORG_MALIIT_DIR = "$${MALIIT_PLUGINS_DATA_DIR}/org/maliit"
DEFINES += MALIIT_KEYBOARD_IMAGES_DIR=\\\"$${ORG_MALIIT_DIR}/images\\\"
DEFINES += MALIIT_KEYBOARD_STYLES_DIR=\\\"$${ORG_MALIIT_DIR}/styles\\\"

include(renderer/renderer.pri)
include(models/models.pri)
include(glass/glass.pri)
include(logic/logic.pri)
include(parser/parser.pri)

target.path += $$PREFIX/usr/lib
languages.path = $$MALIIT_PLUGINS_DATA_DIR
languages.files = languages/
images.path = $$ORG_MALIIT_DIR
images.files = images/
styles.path = $$ORG_MALIIT_DIR
styles.files = styles/

INSTALLS += target languages images styles

