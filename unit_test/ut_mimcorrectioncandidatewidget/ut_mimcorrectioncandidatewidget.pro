TEMPLATE = app
CONFIG += QtTest Dui duiimframework
QT += testlib

DEPENDPATH += .
INCLUDEPATH += 	. \

OBJECTS_DIR = .obj
MOC_DIR = .moc

include(../common_check.pri)

LIBS += -Wl,-rpath=/usr/lib/dui-im-plugins/ -lduikeyboard

HEADERS += ut_duiimcorrectioncandidatewidget.h \

SOURCES += ut_duiimcorrectioncandidatewidget.cpp \

