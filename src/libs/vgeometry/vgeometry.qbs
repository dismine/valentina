VLib {
    Depends { name: "Qt"; submodules: ["gui", "concurrent"] }
    Depends { name: "VMiscLib"}
    Depends { name: "IFCLib"}

    name: "VGeometryLib"
    files: [
        "vgeometrydef.cpp",
        "vgobject.cpp",
        "vabstractcurve.cpp",
        "varc.cpp",
        "vlayoutplacelabel.cpp",
        "vpointf.cpp",
        "vspline.cpp",
        "vsplinepath.cpp",
        "vsplinepoint.cpp",
        "vellipticalarc.cpp",
        "vcubicbezier.cpp",
        "vabstractcubicbezier.cpp",
        "vabstractcubicbezierpath.cpp",
        "vcubicbezierpath.cpp",
        "vabstractarc.cpp",
        "vabstractbezier.cpp",
        "vplacelabelitem.cpp",
        "vgobject.h",
        "vgobject_p.h",
        "vabstractcurve.h",
        "varc.h",
        "varc_p.h",
        "vlayoutplacelabel.h",
        "vpointf.h",
        "vpointf_p.h",
        "vspline.h",
        "vspline_p.h",
        "vsplinepath.h",
        "vsplinepath_p.h",
        "vsplinepoint.h",
        "vsplinepoint_p.h",
        "vgeometrydef.h",
        "vellipticalarc.h",
        "vellipticalarc_p.h",
        "vabstractcurve_p.h",
        "vcubicbezier.h",
        "vcubicbezier_p.h",
        "vabstractcubicbezier.h",
        "vabstractcubicbezierpath.h",
        "vcubicbezierpath.h",
        "vcubicbezierpath_p.h",
        "vabstractarc.h",
        "vabstractarc_p.h",
        "vabstractbezier.h",
        "vplacelabelitem.h",
        "vplacelabelitem_p.h"
    ]

    Group {
        name: "Precompiled headers"
        files: {
            var files = ["stable.h"];
            if (qbs.toolchain.contains("msvc"))
                files.push("stable.cpp")
            return files;
        }
        fileTags: ["cpp_pch_src"]
    }

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["gui"] }
        Depends { name: "VMiscLib"}
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
