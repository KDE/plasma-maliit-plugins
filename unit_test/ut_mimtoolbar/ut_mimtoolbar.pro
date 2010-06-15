TEMPLATE = app
CONFIG += QtTest meegotouch meegoimframework
QT += testlib

DEPENDPATH += .
INCLUDEPATH +=  . \
		../stubs/

OBJECTS_DIR = .obj
MOC_DIR = .moc

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard


HEADERS += ut_mimtoolbar.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h


SOURCES += ut_mimtoolbar.cpp \
           ../stubs/fakegconf.cpp

target.files += \
           $$TARGET \
           testtoolbar.xml \

include(../common_check.pri)
