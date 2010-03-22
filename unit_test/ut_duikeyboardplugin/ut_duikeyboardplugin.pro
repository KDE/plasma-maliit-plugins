TEMPLATE = app
CONFIG += dui duiimframework
QT += testlib

DEPENDPATH += .
INCLUDEPATH += 	. \

OBJECTS_DIR = .obj
MOC_DIR = .moc

include(../common_check.pri)

LIBS += -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard

HEADERS += ut_duikeyboardplugin.h

SOURCES += ut_duikeyboardplugin.cpp

