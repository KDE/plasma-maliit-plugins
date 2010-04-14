TEMPLATE = app
CONFIG += QtTest Dui duiimframework
QT += testlib

DEPENDPATH += .
INCLUDEPATH +=  . \
		../stubs/

OBJECTS_DIR = .obj
MOC_DIR = .moc

LIBS += -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard


HEADERS += ut_duiimtoolbar.h \
           ../stubs/duigconfitem_stub.h \
           ../stubs/fakegconf.h


SOURCES += ut_duiimtoolbar.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
