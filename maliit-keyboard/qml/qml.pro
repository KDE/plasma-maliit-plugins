include(../config.pri)

TARGET = dummy
TEMPLATE = lib

qml.path = $$MALIIT_KEYBOARD_DATA_DIR
qml.files = *.qml

INSTALLS += qml
