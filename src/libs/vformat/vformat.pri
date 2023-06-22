# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/vdimensions.cpp \
    $$PWD/vmeasurements.cpp \
    $$PWD/vlabeltemplate.cpp \
    $$PWD/vpatternrecipe.cpp \
    $$PWD/vsinglelineoutlinechar.cpp \
    $$PWD/vwatermark.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/vdimensions.h \
    $$PWD/vmeasurements.h \
    $$PWD/stable.h \
    $$PWD/vlabeltemplate.h \
    $$PWD/vpatternrecipe.h \
    $$PWD/vsinglelineoutlinechar.h \
    $$PWD/vwatermark.h
