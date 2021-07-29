# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/dialogs/configpages/puzzlepreferencesconfigurationpage.cpp \
    $$PWD/dialogs/configpages/puzzlepreferencespathpage.cpp \
    $$PWD/dialogs/dialogpuzzlepreferences.cpp \
    $$PWD/dialogs/vpdialogabout.cpp \
    $$PWD/main.cpp \
    $$PWD/vpapplication.cpp \
    $$PWD/vpcarrousel.cpp \
    $$PWD/vpcarrouselpiece.cpp \
    $$PWD/vpcarrouselpiecelist.cpp \
    $$PWD/vpcommandline.cpp \
    $$PWD/vpcommands.cpp \
    $$PWD/vpexporter.cpp \
    $$PWD/vpgraphicspiece.cpp \
    $$PWD/vpgraphicssheet.cpp \
    $$PWD/vpgraphicstilegrid.cpp \
    $$PWD/vplayout.cpp \
    $$PWD/vplayoutsettings.cpp \
    $$PWD/vpmaingraphicsview.cpp \
    $$PWD/vpmainwindow.cpp \
    $$PWD/vpmimedatapiece.cpp \
    $$PWD/vppiece.cpp \
    $$PWD/vpsettings.cpp \
    $$PWD/vpsheet.cpp \
    $$PWD/vptilefactory.cpp \
    $$PWD/xml/vplayoutfilereader.cpp \
    $$PWD/xml/vplayoutfilewriter.cpp \
    $$PWD/xml/vplayoutliterals.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/dialogs/configpages/puzzlepreferencesconfigurationpage.h \
    $$PWD/dialogs/configpages/puzzlepreferencespathpage.h \
    $$PWD/dialogs/dialogpuzzlepreferences.h \
    $$PWD/dialogs/vpdialogabout.h \
    $$PWD/stable.h \
    $$PWD/vpapplication.h \
    $$PWD/vpcarrousel.h \
    $$PWD/vpcarrouselpiece.h \
    $$PWD/vpcarrouselpiecelist.h \
    $$PWD/vpcommandline.h \
    $$PWD/vpcommands.h \
    $$PWD/vpexporter.h \
    $$PWD/vpgraphicspiece.h \
    $$PWD/vpgraphicssheet.h \
    $$PWD/vpgraphicstilegrid.h \
    $$PWD/vplayout.h \
    $$PWD/vplayoutsettings.h \
    $$PWD/vpmaingraphicsview.h \
    $$PWD/vpmainwindow.h \
    $$PWD/vpmimedatapiece.h \
    $$PWD/vppiece.h \
    $$PWD/vpsettings.h \
    $$PWD/vpsheet.h \
    $$PWD/vptilefactory.h \
    $$PWD/xml/vplayoutfilereader.h \
    $$PWD/xml/vplayoutfilewriter.h \
    $$PWD/xml/vplayoutliterals.h

FORMS += \
    $$PWD/dialogs/configpages/puzzlepreferencesconfigurationpage.ui \
    $$PWD/dialogs/configpages/puzzlepreferencespathpage.ui \
    $$PWD/dialogs/dialogpuzzlepreferences.ui \
    $$PWD/dialogs/vpdialogabout.ui \
    $$PWD/vpcarrousel.ui \
    $$PWD/vpmainwindow.ui
