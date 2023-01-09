VLib {
    Depends { name: "Qt"; submodules: ["xml"] }
    Depends { name: "IFCLib" }

    name: "VFormatLib"
    files: [
        "vdimensions.cpp",
        "vmeasurements.cpp",
        "vlabeltemplate.cpp",
        "vpatternrecipe.cpp",
        "vwatermark.cpp",
        "vdimensions.h",
        "vmeasurements.h",
        "vlabeltemplate.h",
        "vpatternrecipe.h",
        "vwatermark.h",
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
        Depends { name: "Qt"; submodules: ["xml"] }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
