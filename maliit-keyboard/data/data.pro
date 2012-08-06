include(../config.pri)

TARGET = dummy
TEMPLATE = lib

languages.path = $$MALIIT_PLUGINS_DATA_DIR
languages.files = languages/

styles.path = $$MALIIT_KEYBOARD_DATA_DIR
styles.files = styles

INSTALLS += languages styles

QMAKE_EXTRA_TARGETS += check
check.target = check

check.commands = \
    xmllint --noout --dtdvalid languages/VirtualKeyboardLayout.dtd languages/*.xml
