VERSION = 0.1.0
TARGET = meego-keyboard-quick
TEMPLATE = lib
QT = core gui declarative

MEEGO_IM_PLUGINS_DIR=$${DESTDIR}/usr/lib/meego-im-plugins
MEEGO_KEYBOARD_QUICK_DIR=$${MEEGO_IM_PLUGINS_DIR}/meego-keyboard-qml
MEEGO_KEYBOARD_QUICK_FILE=$${MEEGO_KEYBOARD_QUICK_DIR}/meego-keyboard.qml
DEFINES += "MEEGO_KEYBOARD_QUICK_FILE=$${MEEGO_KEYBOARD_QUICK_FILE}"

CONFIG += \
    plugin \
    meegoimframework \
    meegoimquick \

SOURCES += \
    meegokeyboardquickplugin.cpp \

HEADERS += \
    meegokeyboardquickplugin.h \

qml.files = qml/meego-keyboard.qml qml/assets
qml.path = $${MEEGO_KEYBOARD_QUICK_DIR}

target.path += $${MEEGO_IM_PLUGINS_DIR}

INSTALLS += target qml
