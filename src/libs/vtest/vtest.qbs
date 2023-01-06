VLib {
    Depends { name: "Qt"; submodules: ["testlib"] }
    Depends { name: "VGeometryLib" }
    Depends { name: "VPatternDBLib" }
    Depends { name: "VLayoutLib" }
    Depends { name: "Tape" }
    Depends { name: "Valentina" }
    Depends { name: "Puzzle" }

    name: "VTestLib"
    files: [
        "abstracttest.cpp",
        "abstracttest.h",
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
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
