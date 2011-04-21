include(../common_top.pri)

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
           $$STUBS_DIR/minputmethodhoststub.h \
           dummydriver_mkh.h \
           $$STUBS_DIR/mgconfitem_stub.h \
           $$STUBS_DIR/fakegconf.h \

# PopupFactory needs to be compiled to have a mocked functionality
# otherwise the unit test crashes
SOURCES += ut_mkeyboardhost.cpp \
           $$STUBS_DIR/minputmethodhoststub.cpp \
           dummydriver_mkh.cpp \
           $$STUBS_DIR/fakegconf.cpp \

target.files += $$TARGET \
                toolbar1.xml \
                toolbar2.xml \

include(../common_check.pri)
