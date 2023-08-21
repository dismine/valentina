# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/libdxfrw/drw_base.cpp \
    $$PWD/vdxfengine.cpp \
    $$PWD/vdxfpaintdevice.cpp \
    $$PWD/libdxfrw/intern/drw_dbg.cpp \
    $$PWD/libdxfrw/intern/drw_textcodec.cpp \
    $$PWD/libdxfrw/intern/dxfreader.cpp \
    $$PWD/libdxfrw/intern/dxfwriter.cpp \
    $$PWD/libdxfrw/intern/drw_cptable932.h \
    $$PWD/libdxfrw/intern/drw_cptable936.h \
    $$PWD/libdxfrw/intern/drw_cptable949.h \
    $$PWD/libdxfrw/intern/drw_cptable950.h \
    $$PWD/libdxfrw/intern/drw_cptables.h \
    $$PWD/libdxfrw/drw_classes.cpp \
    $$PWD/libdxfrw/drw_entities.cpp \
    $$PWD/libdxfrw/drw_header.cpp \
    $$PWD/libdxfrw/drw_objects.cpp \
    $$PWD/libdxfrw/libdxfrw.cpp \
    $$PWD/dxiface.cpp \
    $$PWD/dxfdef.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/libdxfrw/drw_reserve.h \
    $$PWD/libdxfrw/intern/make_unique.h \
    $$PWD/stable.h \
    $$PWD/vdxfengine.h \
    $$PWD/vdxfpaintdevice.h \
    $$PWD/dxfdef.h \
    $$PWD/libdxfrw/intern/drw_dbg.h \
    $$PWD/libdxfrw/intern/drw_textcodec.h \
    $$PWD/libdxfrw/intern/dxfreader.h \
    $$PWD/libdxfrw/intern/dxfwriter.h \
    $$PWD/libdxfrw/drw_base.h \
    $$PWD/libdxfrw/drw_classes.h \
    $$PWD/libdxfrw/drw_entities.h \
    $$PWD/libdxfrw/drw_header.h \
    $$PWD/libdxfrw/drw_interface.h \
    $$PWD/libdxfrw/drw_objects.h \
    $$PWD/libdxfrw/libdxfrw.h \
    $$PWD/libdxfrw/main_doc.h \
    $$PWD/dxiface.h
