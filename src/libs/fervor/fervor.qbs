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

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["network"] }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
