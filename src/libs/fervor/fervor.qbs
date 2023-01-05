VLib {
    Depends { name: "Qt"; submodules: ["widgets", "network"] }
    Depends { name: "IFCLib" }

    name: "FervorLib"
    files: [
        "fvupdatewindow.cpp",
        "fvupdater.cpp",
        "fvavailableupdate.cpp",
        "fvupdatewindow.h",
        "fvupdater.h",
        "fvavailableupdate.h",
        "fvupdatewindow.ui",
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
        Depends { name: "Qt"; submodules: ["network"] }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
