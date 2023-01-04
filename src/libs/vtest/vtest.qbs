VLib {
    Depends { name: "Qt"; submodules: ["testlib", "gui", "printsupport"] }
    Depends { name: "VGeometryLib" }
    Depends { name: "VLayoutLib" }
    Depends { name: "IFCLib" }

    name: "VTestLib"
    files: [
        "abstracttest.cpp",
        "abstracttest.h",
    ]

    Group {
        name: "Precompiled headers"
        files: {
            var files = ["stable.h"];
            if (qbs.targetOS.contains("windows"))
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
