LAYOUTS_DATA = layouts/*.xml layouts/*.dtd
layouts_data.path = /usr/share/meegotouch/virtual-keyboard/layouts
layouts_data.files = $$LAYOUTS_DATA

INSTALLS += \
    layouts_data \

OTHER_FILES += $$LAYOUTS_DATA
