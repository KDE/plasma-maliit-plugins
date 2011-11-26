include(../config.pri)

TARGET = maliit-keyboard
TEMPLATE = lib
QT = core gui declarative
LIBS += -L../maliit-keyboard -lmaliit-keyboard
INCLUDEPATH += ../maliit-keyboard

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
    maliitkeyboardplugin.h \
    inputmethod.h \

SOURCES += \
    maliitkeyboardplugin.cpp \
    inputmethod.cpp \

target.path += $${MALIIT_PLUGINS_DIR}
INSTALLS += target
