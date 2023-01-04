VLib {
    Depends { name: "Qt"; submodules: ["widgets", "network", "printsupport", "xml"] }
    Depends { name: "VMiscLib" }

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
