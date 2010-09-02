TEMPLATE = app
CONFIG += meegotouch

DEPENDPATH += .
INCLUDEPATH += 	. \

include(../common_check.pri)

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

HEADERS += ut_mimcorrectioncandidatewidget.h \

SOURCES += ut_mimcorrectioncandidatewidget.cpp \

