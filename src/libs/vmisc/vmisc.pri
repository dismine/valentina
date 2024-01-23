# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/def.cpp \
    $$PWD/dialogs/dialogaskcollectstatistic.cpp \
    $$PWD/dialogs/vshortcutdialog.cpp \
    $$PWD/svgfont/svgdef.cpp \
    $$PWD/svgfont/vsvgfont.cpp \
    $$PWD/svgfont/vsvgfontdatabase.cpp \
    $$PWD/svgfont/vsvgfontengine.cpp \
    $$PWD/svgfont/vsvgfontreader.cpp \
    $$PWD/svgfont/vsvgglyph.cpp \
    $$PWD/svgfont/vsvgpathtokenizer.cpp \
    $$PWD/testpath.cpp \
    $$PWD/theme/vapplicationstyle.cpp \
    $$PWD/theme/vmanuallayoutstyle.cpp \
    $$PWD/theme/vpatternpiecestyle.cpp \
    $$PWD/theme/vscenestylesheet.cpp \
    $$PWD/theme/vstylesheetstyle.cpp \
    $$PWD/theme/vtheme.cpp \
    $$PWD/theme/vtoolstyle.cpp \
    $$PWD/vabstractvalapplication.cpp \
    $$PWD/vabstractapplication.cpp \
    $$PWD/projectversion.cpp \
    $$PWD/vcommonsettings.cpp \
    $$PWD/vvalentinasettings.cpp \
    $$PWD/commandoptions.cpp \
    $$PWD/qxtcsvmodel.cpp \
    $$PWD/vtablesearch.cpp \
    $$PWD/dialogs/dialogexporttocsv.cpp \
    $$PWD/literals.cpp \
    $$PWD/vmodifierkey.cpp \
    $$PWD/dialogs/dialogselectlanguage.cpp \
    $$PWD/vabstractshortcutmanager.cpp \
    $$PWD/vtranslator.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

macx {
  HEADERS += $$PWD/theme/macutils.h
  OBJECTIVE_SOURCES += $$PWD/theme/macutils.mm
}

contains(DEFINES, APPIMAGE) {
    SOURCES += \
        $$PWD/binreloc.c \
        $$PWD/appimage.cpp
}

HEADERS += \
    $$PWD/compatibility.h \
    $$PWD/dialogs/dialogaskcollectstatistic.h \
    $$PWD/dialogs/vshortcutdialog.h \
    $$PWD/lambdaconstants.h \
    $$PWD/stable.h \
    $$PWD/def.h \
    $$PWD/svgfont/svgdef.h \
    $$PWD/svgfont/vsvgfont.h \
    $$PWD/svgfont/vsvgfont_p.h \
    $$PWD/svgfont/vsvgfontdatabase.h \
    $$PWD/svgfont/vsvgfontengine.h \
    $$PWD/svgfont/vsvgfontengine_p.h \
    $$PWD/svgfont/vsvgfontreader.h \
    $$PWD/svgfont/vsvgglyph.h \
    $$PWD/svgfont/vsvgglyph_p.h \
    $$PWD/svgfont/vsvgpathtokenizer.h \
    $$PWD/testpath.h \
    $$PWD/theme/themeDef.h \
    $$PWD/theme/vapplicationstyle.h \
    $$PWD/theme/vmanuallayoutstyle.h \
    $$PWD/theme/vpatternpiecestyle.h \
    $$PWD/theme/vscenestylesheet.h \
    $$PWD/theme/vstylesheetstyle.h \
    $$PWD/theme/vtheme.h \
    $$PWD/theme/vtoolstyle.h \
    $$PWD/vabstractvalapplication.h \
    $$PWD/vmath.h \
    $$PWD/vabstractapplication.h \
    $$PWD/projectversion.h \
    $$PWD/vcommonsettings.h \
    $$PWD/vvalentinasettings.h \
    $$PWD/debugbreak.h \
    $$PWD/vlockguard.h \
    $$PWD/vsysexits.h \
    $$PWD/commandoptions.h \
    $$PWD/qxtcsvmodel.h \
    $$PWD/vtablesearch.h \
    $$PWD/dialogs/dialogexporttocsv.h \
    $$PWD/customevents.h \
    $$PWD/defglobal.h \
    $$PWD/testvapplication.h \
    $$PWD/literals.h \
    $$PWD/qt_dispatch/qt_dispatch.h \
    $$PWD/vmodifierkey.h \
    $$PWD/typedef.h \
    $$PWD/dialogs/dialogselectlanguage.h \
    $$PWD/fpm/fixed.hpp \
    $$PWD/fpm/math.hpp \
    $$PWD/vabstractshortcutmanager.h \
    $$PWD/vtranslator.h

contains(DEFINES, APPIMAGE) {
    SOURCES += \
        $$PWD/binreloc.h \
        $$PWD/appimage.h
}

# Qt's versions
# 5.2.0, 5.2.1
# 5.3.0, 5.3.1, 5.3.2


contains(QT_VERSION, ^5\\.[0-2]\\.[0-2]$) { # Since Qt 5.3.0
    HEADERS +=

    SOURCES +=
}

FORMS    += \
    $$PWD/dialogs/dialogaskcollectstatistic.ui \
    $$PWD/dialogs/dialogexporttocsv.ui \
    $$PWD/dialogs/dialogselectlanguage.ui \
    $$PWD/dialogs/vshortcutdialog.ui
