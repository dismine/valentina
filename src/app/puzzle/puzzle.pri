# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/puzzlecommands.cpp \
    $$PWD/puzzlemainwindow.cpp \
    $$PWD/puzzleapplication.cpp \
    $$PWD/vpiececarrouselpiecepreview.cpp \
    $$PWD/vpuzzlecommandline.cpp \
    $$PWD/dialogs/dialogaboutpuzzle.cpp \
    $$PWD/vpiececarrousel.cpp \
    $$PWD/vpuzzlegraphicslayout.cpp \
    $$PWD/vpuzzlegraphicspiece.cpp \
    $$PWD/vpuzzlelayout.cpp \
    $$PWD/vpuzzlelayer.cpp \
    $$PWD/vpuzzlemaingraphicsscene.cpp \
    $$PWD/vpuzzlemaingraphicsview.cpp \
    $$PWD/vpuzzlemimedatapiece.cpp \
    $$PWD/vpuzzlepiece.cpp \
    $$PWD/vpuzzlesettings.cpp \
    $$PWD/xml/layoutliterals.cpp \
    $$PWD/xml/vpuzzlelayoutfilewriter.cpp \
    $$PWD/xml/vpuzzlelayoutfilereader.cpp \
    $$PWD/vpiececarrousellayer.cpp \
    $$PWD/vpiececarrouselpiece.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/puzzlecommands.h \
    $$PWD/puzzlemainwindow.h \
    $$PWD/stable.h \
    $$PWD/puzzleapplication.h \
    $$PWD/vpiececarrouselpiecepreview.h \
    $$PWD/vpuzzlecommandline.h \
    $$PWD/dialogs/dialogaboutpuzzle.h \
    $$PWD/vpiececarrousel.h \
    $$PWD/vpuzzlegraphicslayout.h \
    $$PWD/vpuzzlegraphicspiece.h \
    $$PWD/vpuzzlelayout.h \
    $$PWD/vpuzzlelayer.h \
    $$PWD/vpuzzlemaingraphicsscene.h \
    $$PWD/vpuzzlemaingraphicsview.h \
    $$PWD/vpuzzlemimedatapiece.h \
    $$PWD/vpuzzlepiece.h \
    $$PWD/vpuzzlesettings.h \
    $$PWD/xml/layoutliterals.h \
    $$PWD/xml/vpuzzlelayoutfilewriter.h \
    $$PWD/xml/vpuzzlelayoutfilereader.h \
    $$PWD/vpiececarrousellayer.h \
    $$PWD/vpiececarrouselpiece.h

FORMS += \
    $$PWD/puzzlemainwindow.ui \
    $$PWD/dialogs/dialogaboutpuzzle.ui
