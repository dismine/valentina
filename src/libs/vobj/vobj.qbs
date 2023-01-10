VLib {
    Depends { name: "Qt"; submodules: ["gui"] }

    name: "VObjLib"
    files: [
        "vobjengine.cpp",
        "vobjpaintdevice.cpp",
        "delaunay.cpp",
        "predicates.cpp",
        "vobjengine.h",
        "delaunay.h",
        "vobjpaintdevice.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
