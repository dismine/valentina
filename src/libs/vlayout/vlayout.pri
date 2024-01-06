# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

# Suport subdirectories. Just better project code tree.
include(dialogs/dialogs.pri)

HEADERS += \
    $$PWD/stable.h \
    $$PWD/vlayoutexporter.h \
    $$PWD/vlayoutgenerator.h \
    $$PWD/vlayoutdef.h \
    $$PWD/vlayoutpaper.h \
    $$PWD/vlayoutpaper_p.h \
    $$PWD/vbank.h \
    $$PWD/vcontour.h \
    $$PWD/vcontour_p.h \
    $$PWD/vbestsquare.h \
    $$PWD/vlayoutpoint.h \
    $$PWD/vposition.h \
    $$PWD/vrawlayout.h \ 
    $$PWD/vprintlayout.h \
    $$PWD/vsapoint.h \
    $$PWD/vtextmanager.h \
    $$PWD/vposter.h \
    $$PWD/vgraphicsfillitem.h \
    $$PWD/vabstractpiece.h \
    $$PWD/vabstractpiece_p.h \
    $$PWD/vlayoutpiece.h \
    $$PWD/vlayoutpiece_p.h \
    $$PWD/vlayoutpiecepath.h \
    $$PWD/vlayoutpiecepath_p.h \
    $$PWD/vbestsquare_p.h \
    $$PWD/vrawsapoint.h \
    $$PWD/vboundary.h \
    $$PWD/vfoldline.h

SOURCES += \
    $$PWD/vlayoutexporter.cpp \
    $$PWD/vlayoutgenerator.cpp \
    $$PWD/vlayoutpaper.cpp \
    $$PWD/vbank.cpp \
    $$PWD/vcontour.cpp \
    $$PWD/vbestsquare.cpp \
    $$PWD/vlayoutpoint.cpp \
    $$PWD/vposition.cpp \
    $$PWD/vrawlayout.cpp \
    $$PWD/vsapoint.cpp \
    $$PWD/vprintlayout.cpp \
    $$PWD/vtextmanager.cpp \
    $$PWD/vposter.cpp \
    $$PWD/vgraphicsfillitem.cpp \
    $$PWD/vabstractpiece.cpp \
    $$PWD/vlayoutpiece.cpp \
    $$PWD/vlayoutpiecepath.cpp \
    $$PWD/vrawsapoint.cpp \
    $$PWD/vboundary.cpp \
    $$PWD/vfoldline.cpp

*msvc*:SOURCES += $$PWD/stable.cpp
