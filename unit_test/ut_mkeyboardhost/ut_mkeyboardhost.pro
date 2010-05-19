TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework
DEPENDPATH += .
INCLUDEPATH += . \
        ../stubs/

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

CONFIG += duiimengine duiimenginewords link_pkgconfig
PKGCONFIG += gconf-2.0

# Input
HEADERS += ut_mkeyboardhost.h \
           minputcontextstubconnection.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \

SOURCES += ut_mkeyboardhost.cpp \
           minputcontextstubconnection.cpp \
           ../stubs/fakegconf.cpp \

include(../common_check.pri)
