VLib {
    Depends { name: "Qt"; submodules: ["core", "widgets", "network"] }
    Depends { name: "VMiscLib" }
    Depends { name: "VLayoutLib" }

    name: "VNestingLib"
    files: [
        "vaccountclient.cpp",
        "vaccountclient.h",
        "vaccounttoken.cpp",
        "vaccounttoken.h",
        "vnestingaccountpage.cpp",
        "vnestingaccountpage.h",
        "vnestingaccountpage.ui",
        "vnestingoptionswidget.cpp",
        "vnestingoptionswidget.h",
        "vnestingoptionswidget.ui",
        "vcirculargauge.cpp",
        "vcirculargauge.h",
        "vnestingprogressdialog.cpp",
        "vnestingprogressdialog.h",
        "vnestingprogressdialog.ui",
        "vnestorclient.cpp",
        "vnestorclient.h",
    ]

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["network"] }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
