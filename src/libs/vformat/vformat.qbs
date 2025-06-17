VLib {
    Depends { name: "Qt"; submodules: ["gui", "xml"] }
    Depends { name: "IFCLib" }

    name: "VFormatLib"
    files: [
        "knownmeasurements/vknownmeasurement.h",
        "knownmeasurements/vknownmeasurements.cpp",
        "knownmeasurements/vknownmeasurements.h",
        "knownmeasurements/vknownmeasurements_p.h",
        "knownmeasurements/vknownmeasurementsdocument.cpp",
        "knownmeasurements/vknownmeasurementsdocument.h",
        "vdimensions.cpp",
        "knownmeasurements/vknownmeasurementsdatabase.cpp",
        "knownmeasurements/vknownmeasurementsdatabase.h",
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
        Depends { name: "Qt"; submodules: ["gui", "xml"] }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
