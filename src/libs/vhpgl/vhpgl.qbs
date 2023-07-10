VLib {
    Depends { name: "Qt"; submodules: ["core"] }
    Depends { name: "VMiscLib"}
    Depends { name: "VFormatLib"}

    name: "VHPGLLib"

    files: [
        "vhpglengine.cpp",
        "vhpglengine.h",
        "vhpglpaintdevice.cpp",
        "vhpglpaintdevice.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
