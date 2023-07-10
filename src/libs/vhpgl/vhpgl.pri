# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/vhpglengine.cpp \
    $$PWD/vhpglpaintdevice.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/stable.h \
    $$PWD/vhpglengine.h \
    $$PWD/vhpglpaintdevice.h
