VLib {
    name: "VGAnalyticsLib"
    files: [
        "def.h",
        "vganalytics.cpp",
        "vganalytics.h",
        "vganalyticsworker.cpp",
        "vganalyticsworker.h",
    ]
    Depends { name: "Qt"; submodules: ["core", "network", "gui"] }
    Depends { name: "VMiscLib"; }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
