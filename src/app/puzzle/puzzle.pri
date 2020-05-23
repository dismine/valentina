# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/dialogs/vpdialogabout.cpp \
    $$PWD/main.cpp \
    $$PWD/puzzlecommands.cpp \
    $$PWD/puzzlemainwindow.cpp \
    $$PWD/puzzleapplication.cpp \
    $$PWD/vpiececarrouselpiecepreview.cpp \
    $$PWD/vpuzzlecommandline.cpp \
    $$PWD/vpiececarrousel.cpp \
    $$PWD/vpuzzlegraphicslayout.cpp \
    $$PWD/vpuzzlegraphicspiece.cpp \
    $$PWD/vpuzzlelayout.cpp \
    $$PWD/vpuzzlelayer.cpp \
    $$PWD/vpuzzlemaingraphicsview.cpp \
    $$PWD/vpuzzlemimedatapiece.cpp \
    $$PWD/vpuzzlepiece.cpp \
    $$PWD/vpuzzlesettings.cpp \
    $$PWD/xml/vplayoutfilereader.cpp \
    $$PWD/xml/vplayoutfilewriter.cpp \
    $$PWD/xml/vplayoutliterals.cpp \
    $$PWD/vpiececarrousellayer.cpp \
    $$PWD/vpiececarrouselpiece.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/dialogs/vpdialogabout.h \
    $$PWD/puzzlecommands.h \
    $$PWD/puzzlemainwindow.h \
    $$PWD/stable.h \
    $$PWD/puzzleapplication.h \
    $$PWD/vpiececarrouselpiecepreview.h \
    $$PWD/vpuzzlecommandline.h \
    $$PWD/vpiececarrousel.h \
    $$PWD/vpuzzlegraphicslayout.h \
    $$PWD/vpuzzlegraphicspiece.h \
    $$PWD/vpuzzlelayout.h \
    $$PWD/vpuzzlelayer.h \
    $$PWD/vpuzzlemaingraphicsview.h \
    $$PWD/vpuzzlemimedatapiece.h \
    $$PWD/vpuzzlepiece.h \
    $$PWD/vpuzzlesettings.h \
    $$PWD/xml/vplayoutfilereader.h \
    $$PWD/xml/vplayoutfilewriter.h \
    $$PWD/xml/vplayoutliterals.h \
    $$PWD/vpiececarrousellayer.h \
    $$PWD/vpiececarrouselpiece.h

FORMS += \
    $$PWD/dialogs/vpdialogabout.ui \
    $$PWD/puzzlemainwindow.ui \
    $$PWD/vpiececarrousel.ui
