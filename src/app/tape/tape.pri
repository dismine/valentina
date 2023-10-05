# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES  += \
    $$PWD/dialogs/dialogdimensioncustomnames.cpp \
    $$PWD/dialogs/dialogdimensionlabels.cpp \
    $$PWD/dialogs/dialogmeasurementscsvcolumns.cpp \
    $$PWD/dialogs/dialogrestrictdimension.cpp \
    $$PWD/main.cpp \
    $$PWD/tmainwindow.cpp \
    $$PWD/mapplication.cpp \
    $$PWD/dialogs/dialogabouttape.cpp \
    $$PWD/dialogs/dialognewmeasurements.cpp \
    $$PWD/dialogs/dialogmdatabase.cpp \
    $$PWD/vlitepattern.cpp \
    $$PWD/dialogs/dialogtapepreferences.cpp \
    $$PWD/dialogs/configpages/tapepreferencesconfigurationpage.cpp \
    $$PWD/vtapesettings.cpp \
    $$PWD/dialogs/dialogsetupmultisize.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS  += \
    $$PWD/dialogs/dialogdimensioncustomnames.h \
    $$PWD/dialogs/dialogdimensionlabels.h \
    $$PWD/dialogs/dialogmeasurementscsvcolumns.h \
    $$PWD/dialogs/dialogrestrictdimension.h \
    $$PWD/tmainwindow.h \
    $$PWD/stable.h \
    $$PWD/mapplication.h \
    $$PWD/dialogs/dialogabouttape.h \
    $$PWD/dialogs/dialognewmeasurements.h \
    $$PWD/dialogs/dialogmdatabase.h \
    $$PWD/version.h \
    $$PWD/vlitepattern.h \
    $$PWD/dialogs/dialogtapepreferences.h \
    $$PWD/dialogs/configpages/tapepreferencesconfigurationpage.h \
    $$PWD/vtapesettings.h \
    $$PWD/dialogs/dialogsetupmultisize.h

FORMS    += \
    $$PWD/dialogs/dialogdimensioncustomnames.ui \
    $$PWD/dialogs/dialogdimensionlabels.ui \
    $$PWD/dialogs/dialogmeasurementscsvcolumns.ui \
    $$PWD/dialogs/dialogrestrictdimension.ui \
    $$PWD/tmainwindow.ui \
    $$PWD/dialogs/dialogabouttape.ui \
    $$PWD/dialogs/dialognewmeasurements.ui \
    $$PWD/dialogs/dialogmdatabase.ui \
    $$PWD/dialogs/dialogtapepreferences.ui \
    $$PWD/dialogs/configpages/tapepreferencesconfigurationpage.ui \
    $$PWD/dialogs/dialogsetupmultisize.ui
