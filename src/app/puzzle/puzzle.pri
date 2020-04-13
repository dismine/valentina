# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/puzzlecommands.cpp \
    $$PWD/puzzlemainwindow.cpp \
    $$PWD/puzzleapplication.cpp \
    $$PWD/vpuzzlecommandline.cpp \
    $$PWD/dialogs/dialogaboutpuzzle.cpp \
    $$PWD/vpiececarrousel.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/puzzlecommands.h \
    $$PWD/puzzlemainwindow.h \
    $$PWD/stable.h \
    $$PWD/puzzleapplication.h \
    $$PWD/vpuzzlecommandline.h \
    $$PWD/dialogs/dialogaboutpuzzle.h \
    $$PWD/vpiececarrousel.h

FORMS += \
    $$PWD/puzzlemainwindow.ui \
    $$PWD/dialogs/dialogaboutpuzzle.ui
