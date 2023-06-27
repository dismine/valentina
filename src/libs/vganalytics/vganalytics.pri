HEADERS += \
    $$PWD/stable.h \
    $$PWD/def.h \
    $$PWD/vganalytics.h \
    $$PWD/vganalyticsworker.h

SOURCES += \
    $$PWD/vganalytics.cpp \
    $$PWD/vganalyticsworker.cpp

*msvc*:SOURCES += $$PWD/stable.cpp
