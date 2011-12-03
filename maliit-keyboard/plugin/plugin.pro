include(../../config.pri)

TARGET = maliit-keyboard-plugin
TEMPLATE = lib
QT = core gui declarative
LIBS += -L../lib -lmaliit-keyboard
INCLUDEPATH += ../lib

MALIIT_KEYBOARD_DIR=$${MALIIT_PLUGINS_DIR}/maliit-keyboard

CONFIG += \
    plugin \

enable-legacy {
    CONFIG += meegoimframework
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += maliit-plugins-0.80
    # moc needs the include path
    INCLUDEPATH += $$system(pkg-config --cflags maliit-plugins-0.80 | tr \' \' \'\\n\' | grep ^-I | cut -d I -f 2-)
}


HEADERS += \
    plugin.h \
    inputmethod.h \

SOURCES += \
    plugin.cpp \
    inputmethod.cpp \

target.path += $${MALIIT_PLUGINS_DIR}
INSTALLS += target
