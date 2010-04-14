IMAGES_DATA = theme/*.svg
images_data.path = /usr/share/dui/virtual-keyboard/images
images_data.files = $$IMAGES_DATA

CSS_DATA = theme/*.css
css_data.path = /usr/share/dui/virtual-keyboard/css
css_data.files = $$CSS_DATA

INSTALLS += \
    images_data \
    css_data \

