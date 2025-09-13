import qbs.Utilities

VLib {
    Depends { name: "VMiscLib" }
    Depends { name: "Qt"; submodules: ["core", "gui", "xml"] }

    name: "VDXFLib"
    files: [
        "vdxfengine.cpp",
        "vdxfpaintdevice.cpp",
        "dxiface.cpp",
        "dxfdef.cpp",
        "vdxfengine.h",
        "vdxfpaintdevice.h",
        "dxfdef.h",
        "dxiface.h",
    ]

    Group {
        name: "libdxfrw"
        prefix: "libdxfrw/"
        files: [
            "drw_base.cpp",
            "intern/drw_dbg.cpp",
            "intern/drw_textcodec.cpp",
            "intern/dxfreader.cpp",
            "intern/dxfwriter.cpp",
            "drw_classes.cpp",
            "drw_entities.cpp",
            "drw_header.cpp",
            "drw_objects.cpp",
            "libdxfrw.cpp",
            "drw_reserve.h",
            "intern/make_unique.h",
            "intern/drw_dbg.h",
            "intern/drw_textcodec.h",
            "intern/dxfreader.h",
            "intern/dxfwriter.h",
            "drw_base.h",
            "drw_classes.h",
            "drw_entities.h",
            "drw_header.h",
            "drw_interface.h",
            "drw_objects.h",
            "libdxfrw.h",
            "main_doc.h",
        ]
    }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
