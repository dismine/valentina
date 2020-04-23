# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/puzzlecommands.cpp \
    $$PWD/puzzlemainwindow.cpp \
    $$PWD/puzzleapplication.cpp \
    $$PWD/vpuzzlecommandline.cpp \
    $$PWD/dialogs/dialogaboutpuzzle.cpp \
    $$PWD/vpiececarrousel.cpp \
    $$PWD/vpuzzlelayout.cpp \
    $$PWD/vpuzzlelayer.cpp \
    $$PWD/vpuzzlepiece.cpp \
    $$PWD/xml/layoutliterals.cpp \
    $$PWD/xml/vpuzzlelayoutfilewriter.cpp \
    $$PWD/xml/vpuzzlelayoutfilereader.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/puzzlecommands.h \
    $$PWD/puzzlemainwindow.h \
    $$PWD/stable.h \
    $$PWD/puzzleapplication.h \
    $$PWD/vpuzzlecommandline.h \
    $$PWD/dialogs/dialogaboutpuzzle.h \
    $$PWD/vpiececarrousel.h \
    $$PWD/vpuzzlelayout.h \
    $$PWD/vpuzzlelayer.h \
    $$PWD/vpuzzlepiece.h \
    $$PWD/xml/layoutliterals.h \
    $$PWD/xml/vpuzzlelayoutfilewriter.h \
    $$PWD/xml/vpuzzlelayoutfilereader.h

FORMS += \
    $$PWD/puzzlemainwindow.ui \
    $$PWD/dialogs/dialogaboutpuzzle.ui
