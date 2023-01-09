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

    Group {
        name: "Precompiled headers"
        condition: buildconfig.enablePCH
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
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
