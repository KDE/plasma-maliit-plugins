include(../common_top.pri)
TEMPLATE = app
CONFIG += QtTest meegotouch MImServer meegoimframework

contains(CONFIG, noreactionmap) {
} else {
    DEFINES += HAVE_REACTIONMAP
    CONFIG  += meegoreactionmap
}

DEPENDPATH += .
INCLUDEPATH += 	. \
                ../stubs/ \

LIBS += -L/usr/lib -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard
# Input
HEADERS += ut_symbolview.h \
           ../stubs/mgconfitem_stub.h \
           ../stubs/fakegconf.h \
           ../stubs/mreactionmaptester.h \
           $$WIDGETS_DIR/symbolview.h \

# PopupFactory needs to be compiled to have a mocked functionality
# otherwise the unit test crashes
SOURCES += ut_symbolview.cpp \
           ../stubs/fakegconf.cpp \
           $$WIDGETS_DIR/symbolview.cpp \

OTHER_FILES += testsymbols.xml

include(../common_check.pri)

TESTLAYOUTFILE = testsymbols.xml
TESTLAYOUTFILEPATH_LOCAL = $$PWD/$$TESTLAYOUTFILE
TESTLAYOUTFILEPATH_INSTALLED = $$target.path/$$TESTLAYOUTFILE

target.files += \
    $$TARGET \
    $$TESTLAYOUTFILE

DEFINES += TESTLAYOUTFILEPATH_LOCAL=\\\"$$TESTLAYOUTFILEPATH_LOCAL\\\" \
           TESTLAYOUTFILEPATH_INSTALLED=\\\"$$TESTLAYOUTFILEPATH_INSTALLED\\\"
