TEMPLATE = app
CONFIG += QtTest meegotouch mimframework
QT += testlib

DEPENDPATH += .
INCLUDEPATH +=  . \
		../stubs/

OBJECTS_DIR = .obj
MOC_DIR = .moc

LIBS += -Wl,-rpath=/usr/lib/m-im-plugins/ -lmkeyboard


HEADERS += ut_mimtoolbar.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h


SOURCES += ut_mimtoolbar.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
