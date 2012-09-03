include(../config.pri)

TARGET = dummy
TEMPLATE = lib

languages.path = $$MALIIT_PLUGINS_DATA_DIR/languages
languages.files = languages/*.xml languages/*.dtd

debug {
    languages.files += languages/debug/showcase.xml
}

styles.path = $$MALIIT_KEYBOARD_DATA_DIR
styles.files = styles

INSTALLS += languages styles

QMAKE_EXTRA_TARGETS += check
check.target = check

check.commands = \
    xmllint --noout --dtdvalid \"$$IN_PWD/languages/VirtualKeyboardLayout.dtd\" \"$$IN_PWD/languages/\"*.xml
