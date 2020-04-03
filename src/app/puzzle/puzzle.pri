# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
        $$PWD/main.cpp \
        $$PWD/puzzlemainwindow.cpp \
    $$PWD/puzzleapplication.cpp \
    $$PWD/vpuzzlecommandline.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
        $$PWD/puzzlemainwindow.h \
        $$PWD/stable.h \
    $$PWD/puzzleapplication.h \
    $$PWD/vpuzzlecommandline.h

FORMS += \
        $$PWD/puzzlemainwindow.ui
