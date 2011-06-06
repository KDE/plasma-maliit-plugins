QMAKE_EXTRA_TARGETS += testlayouts
testlayouts.target = testlayouts

testlayouts.files = ../m-keyboard/layouts/*.xml
testlayouts.path = /usr/lib/meego-keyboard-tests/layouts/
testlayouts.depends = FORCE
INSTALLS += testlayouts

