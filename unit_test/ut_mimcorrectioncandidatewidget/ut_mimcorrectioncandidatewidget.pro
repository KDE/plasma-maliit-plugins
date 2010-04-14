TEMPLATE = app
CONFIG += QtTest meegotouch mimframework
QT += testlib

DEPENDPATH += .
INCLUDEPATH += 	. \

OBJECTS_DIR = .obj
MOC_DIR = .moc

include(../common_check.pri)

LIBS += -Wl,-rpath=/usr/lib/m-im-plugins/ -lmkeyboard

HEADERS += ut_mimcorrectioncandidatewidget.h \

SOURCES += ut_mimcorrectioncandidatewidget.cpp \

