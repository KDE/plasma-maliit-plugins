TEMPLATE = app
CONFIG += QtTest meegotouch meegoimframework
QT += testlib

DEPENDPATH += .
INCLUDEPATH += 	. \

OBJECTS_DIR = .obj
MOC_DIR = .moc

include(../common_check.pri)

LIBS += -Wl,-rpath=/usr/lib/meego-im-plugins/ -lmeego-keyboard

HEADERS += ut_mimcorrectioncandidatewidget.h \

SOURCES += ut_mimcorrectioncandidatewidget.cpp \

