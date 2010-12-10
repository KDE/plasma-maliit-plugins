include(../common_top.pri)

TEMPLATE = app
CONFIG += meegotouch
DEPENDPATH += .
INCLUDEPATH += . \

include(../common_check.pri)

# Input
HEADERS += ut_mimkeymodel.h \
           $$COMMON_DIR/mimkeymodel.h \

SOURCES += ut_mimkeymodel.cpp \
           $$COMMON_DIR/mimkeymodel.cpp \
           $$COMMON_DIR/keyevent.cpp \

