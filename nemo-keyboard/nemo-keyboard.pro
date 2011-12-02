include(../config.pri)

TEMPLATE = lib
TARGET = dummy

PLUGIN_FILE = qml/nemo-keyboard.qml
PLUGIN_ASSETS = qml/assets

plugin.files = $${PLUGIN_FILE}
plugin.CONFIG += no_check_exist
plugin.path = $${MALIIT_PLUGINS_DIR}

other_files.files = $${PLUGIN_ASSETS}/
other_files.CONFIG += no_check_exist
other_files.path = $${MALIIT_PLUGINS_DIR}

QMAKE_CLEAN += libdummy.so*
INSTALLS += plugin other_files


