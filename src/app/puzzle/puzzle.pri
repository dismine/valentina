# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/dialogs/vpdialogabout.cpp \
    $$PWD/main.cpp \
    $$PWD/vpapplication.cpp \
    $$PWD/vpcarrousel.cpp \
    $$PWD/vpcarrouselpiece.cpp \
    $$PWD/vpcarrouselpiecelist.cpp \
    $$PWD/vpcarrouselpiecepreview.cpp \
    $$PWD/vpcommandline.cpp \
    $$PWD/vpcommands.cpp \
    $$PWD/vpgraphicssheet.cpp \
    $$PWD/vpmainwindow.cpp \
    $$PWD/vpuzzlegraphicspiece.cpp \
    $$PWD/vpuzzlelayout.cpp \
    $$PWD/vpuzzlelayer.cpp \
    $$PWD/vpuzzlemaingraphicsview.cpp \
    $$PWD/vpuzzlemimedatapiece.cpp \
    $$PWD/vpuzzlepiece.cpp \
    $$PWD/vpuzzlesettings.cpp \
    $$PWD/xml/vplayoutfilereader.cpp \
    $$PWD/xml/vplayoutfilewriter.cpp \
    $$PWD/xml/vplayoutliterals.cpp

*msvc*:SOURCES +=

HEADERS += \
    $$PWD/dialogs/vpdialogabout.h \
    $$PWD/vpapplication.h \
    $$PWD/vpcarrousel.h \
    $$PWD/vpcarrouselpiece.h \
    $$PWD/vpcarrouselpiecelist.h \
    $$PWD/vpcarrouselpiecepreview.h \
    $$PWD/vpcommandline.h \
    $$PWD/vpcommands.h \
    $$PWD/vpgraphicssheet.h \
    $$PWD/vpmainwindow.h \
    $$PWD/vpstable.h \
    $$PWD/vpuzzlegraphicspiece.h \
    $$PWD/vpuzzlelayout.h \
    $$PWD/vpuzzlelayer.h \
    $$PWD/vpuzzlemaingraphicsview.h \
    $$PWD/vpuzzlemimedatapiece.h \
    $$PWD/vpuzzlepiece.h \
    $$PWD/vpuzzlesettings.h \
    $$PWD/xml/vplayoutfilereader.h \
    $$PWD/xml/vplayoutfilewriter.h \
    $$PWD/xml/vplayoutliterals.h

FORMS += \
    $$PWD/dialogs/vpdialogabout.ui \
    $$PWD/vpcarrousel.ui \
    $$PWD/vpmainwindow.ui

SOURCES += \
    $$PWD/vpstable.cpp
