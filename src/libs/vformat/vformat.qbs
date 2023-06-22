VLib {
    Depends { name: "Qt"; submodules: ["xml"] }
    Depends { name: "IFCLib" }

    name: "VFormatLib"
    files: [
        "vdimensions.cpp",
        "vmeasurements.cpp",
        "vlabeltemplate.cpp",
        "vpatternrecipe.cpp",
        "vsinglelineoutlinechar.cpp",
        "vsinglelineoutlinechar.h",
        "vwatermark.cpp",
        "vdimensions.h",
        "vmeasurements.h",
        "vlabeltemplate.h",
        "vpatternrecipe.h",
        "vwatermark.h",
    ]

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["xml"] }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
