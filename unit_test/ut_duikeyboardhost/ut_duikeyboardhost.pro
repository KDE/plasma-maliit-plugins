TEMPLATE = app
CONFIG += QtTest Dui DuiImServer duiimframework
DEPENDPATH += .
INCLUDEPATH += . \
        ../stubs/

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard

CONFIG += duiimengine duiimenginewords link_pkgconfig
PKGCONFIG += gconf-2.0

# Input
HEADERS += ut_duikeyboardhost.h \
           duiinputcontextstubconnection.h \
           ../stubs/duigconfitem_stub.h \
           ../stubs/fakegconf.h
SOURCES += ut_duikeyboardhost.cpp \
           duiinputcontextstubconnection.cpp \
           ../stubs/fakegconf.cpp

include(../common_check.pri)
