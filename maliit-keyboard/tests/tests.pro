TEMPLATE = subdirs
SUBDIRS = \
    common \
    language-layout-switching \
    preedit-string \
    repeat-backspace \
    rendering \
    word-candidates \

CONFIG += ordered
QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive
