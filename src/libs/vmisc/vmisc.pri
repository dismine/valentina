# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/def.cpp \
    $$PWD/dialogs/dialogaskcollectstatistic.cpp \
    $$PWD/svgfont/svgdef.cpp \
    $$PWD/svgfont/vsvgfont.cpp \
    $$PWD/svgfont/vsvgfontdatabase.cpp \
    $$PWD/svgfont/vsvgfontengine.cpp \
    $$PWD/svgfont/vsvgfontreader.cpp \
    $$PWD/svgfont/vsvgglyph.cpp \
    $$PWD/svgfont/vsvgpathtokenizer.cpp \
    $$PWD/testpath.cpp \
    $$PWD/theme/vapplicationstyle.cpp \
    $$PWD/theme/vtheme.cpp \
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
    $$PWD/dialogs/dialogselectlanguage.cpp

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
    $$PWD/bpstd/any.hpp \
    $$PWD/bpstd/chrono.hpp \
    $$PWD/bpstd/complex.hpp \
    $$PWD/bpstd/cstddef.hpp \
    $$PWD/bpstd/detail/config.hpp \
    $$PWD/bpstd/detail/enable_overload.hpp \
    $$PWD/bpstd/detail/invoke.hpp \
    $$PWD/bpstd/detail/move.hpp \
    $$PWD/bpstd/detail/nth_type.hpp \
    $$PWD/bpstd/detail/proxy_iterator.hpp \
    $$PWD/bpstd/detail/variant_base.hpp \
    $$PWD/bpstd/detail/variant_fwds.hpp \
    $$PWD/bpstd/detail/variant_traits.hpp \
    $$PWD/bpstd/detail/variant_union.hpp \
    $$PWD/bpstd/detail/variant_visitors.hpp \
    $$PWD/bpstd/exception.hpp \
    $$PWD/bpstd/functional.hpp \
    $$PWD/bpstd/iterator.hpp \
    $$PWD/bpstd/memory.hpp \
    $$PWD/bpstd/optional.hpp \
    $$PWD/bpstd/span.hpp \
    $$PWD/bpstd/string.hpp \
    $$PWD/bpstd/string_view.hpp \
    $$PWD/bpstd/tuple.hpp \
    $$PWD/bpstd/type_traits.hpp \
    $$PWD/bpstd/utility.hpp \
    $$PWD/bpstd/variant.hpp \
    $$PWD/compatibility.h \
    $$PWD/dialogs/dialogaskcollectstatistic.h \
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
    $$PWD/theme/vapplicationstyle.h \
    $$PWD/theme/vtheme.h \
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
    $$PWD/diagnostic.h \
    $$PWD/dialogs/dialogexporttocsv.h \
    $$PWD/customevents.h \
    $$PWD/defglobal.h \
    $$PWD/backport/qoverload.h \
    $$PWD/testvapplication.h \
    $$PWD/literals.h \
    $$PWD/qt_dispatch/qt_dispatch.h \
    $$PWD/vdatastreamenum.h \
    $$PWD/vmodifierkey.h \
    $$PWD/typedef.h \
    $$PWD/backport/qscopeguard.h \
    $$PWD/backport/text.h \
    $$PWD/dialogs/dialogselectlanguage.h \
    $$PWD/fpm/fixed.hpp \
    $$PWD/fpm/math.hpp

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
    $$PWD/dialogs/dialogselectlanguage.ui
