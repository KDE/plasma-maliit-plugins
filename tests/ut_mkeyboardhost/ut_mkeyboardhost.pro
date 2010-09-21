TEMPLATE = app
CONFIG += meegotouch MImServer
DEPENDPATH += .
INCLUDEPATH += . \
        ../stubs/

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

CONFIG += meegoimengine meegoimenginewords link_pkgconfig
PKGCONFIG += gconf-2.0

# Input
HEADERS += ut_mkeyboardhost.h \
           minputcontextstubconnection.h \
           dummydriver_mkh.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \

SOURCES += ut_mkeyboardhost.cpp \
           minputcontextstubconnection.cpp \
           dummydriver_mkh.cpp \
           ../stubs/fakegconf.cpp \

target.files += $$TARGET \
                toolbar1.xml \
                toolbar2.xml \

include(../common_check.pri)
