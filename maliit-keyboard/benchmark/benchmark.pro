TEMPLATE = app
TARGET = maliit-keyboard-benchmark
target.path = $$PREFIX/usr/bin

INCLUDEPATH += ../lib
LIBS += -L../lib -lmaliit-keyboard
SOURCES += main.cpp

QT += core
INSTALLS += target
