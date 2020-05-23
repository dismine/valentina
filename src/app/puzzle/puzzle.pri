# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/dialogs/vpdialogabout.cpp \
    $$PWD/main.cpp \
    $$PWD/vpapplication.cpp \
    $$PWD/vpcarrousel.cpp \
    $$PWD/vpcommands.cpp \
    $$PWD/vpiececarrouselpiecepreview.cpp \
    $$PWD/vpmainwindow.cpp \
    $$PWD/vpuzzlecommandline.cpp \
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

*msvc*:SOURCES +=

HEADERS += \
    $$PWD/dialogs/vpdialogabout.h \
    $$PWD/vpapplication.h \
    $$PWD/vpcarrousel.h \
    $$PWD/vpcommands.h \
    $$PWD/vpiececarrouselpiecepreview.h \
    $$PWD/vpmainwindow.h \
    $$PWD/vpstable.h \
    $$PWD/vpuzzlecommandline.h \
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
    $$PWD/vpcarrousel.ui \
    $$PWD/vpmainwindow.ui

SOURCES += \
    $$PWD/vpstable.cpp
