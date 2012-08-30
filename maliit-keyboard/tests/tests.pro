TEMPLATE = subdirs
SUBDIRS = \
    common \
    editor \
    language-layout-switching \
    preedit-string \
    repeat-backspace \
    rendering \
    word-candidates \
    language-layout-loading \

CONFIG += ordered
QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive
