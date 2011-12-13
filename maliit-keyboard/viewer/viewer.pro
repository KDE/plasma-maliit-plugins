TEMPLATE = app
TARGET = maliit-keyboard-viewer
target.path = $$PREFIX/usr/bin

INCLUDEPATH += ../lib ../
LIBS += -L../lib -lmaliit-keyboard -L../view -lmaliit-keyboard-view
SOURCES += main.cpp

QT += core gui
INSTALLS += target
