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
           minputmethodhoststub.h \
           dummydriver_mkh.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \
           $$WIDGETS_DIR/popupfactory.h \

# PopupFactory needs to be compiled to have a mocked functionality
# otherwise the unit test crashes
SOURCES += ut_mkeyboardhost.cpp \
           minputmethodhoststub.cpp \
           dummydriver_mkh.cpp \
           ../stubs/fakegconf.cpp \
           $$WIDGETS_DIR/popupfactory.cpp \

target.files += $$TARGET \
                toolbar1.xml \
                toolbar2.xml \

include(../common_check.pri)
