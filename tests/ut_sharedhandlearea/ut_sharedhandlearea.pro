TEMPLATE = app
CONFIG += meegotouch meegoreactionmap

DEPENDPATH += .
INCLUDEPATH +=  . \
		../stubs/

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard


HEADERS += ut_sharedhandlearea.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \
           ../stubs/mreactionmaptester.h \


SOURCES += ut_sharedhandlearea.cpp \
           ../stubs/fakegconf.cpp \

target.files += \
           $$TARGET \

include(../common_check.pri)
