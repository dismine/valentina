VDynamicLib {
    name: "QMUParserLib"
    version: "2.7.0"
    files: [
        "qmuparser.cpp",
        "qmuparsertokenreader.cpp",
        "qmuparsererror.cpp",
        "qmuparsercallback.cpp",
        "qmuparserbytecode.cpp",
        "qmuparserbase.cpp",
        "qmuparsertest.cpp",
        "qmutranslation.cpp",
        "qmuformulabase.cpp",
        "qmutokenparser.cpp",
        "qmudef.cpp",
        "qmuparser.h",
        "qmuparser_global.h",
        "qmuparsertokenreader.h",
        "qmuparsertoken.h",
        "qmuparserfixes.h",
        "qmuparsererror.h",
        "qmuparserdef.h",
        "qmuparsercallback.h",
        "qmuparserbytecode.h",
        "qmuparserbase.h",
        "qmuparsertest.h",
        "qmutranslation.h",
        "qmudef.h",
        "qmuformulabase.h",
        "qmutokenparser.h",
        "qmuparsercallback_p.h",
        "make_unique.h",
    ]

    cpp.defines: ["QMUPARSER_LIBRARY", 'QMUP_VERSION="' + product.version + '"']

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }

    Group {
        name: "win deploy"
        condition: qbs.targetOS.contains("windows")
        prefix: product.sourceDirectory + "/"
        files: [
            "LICENSE_BSD.txt"
        ]
        qbs.install: true
        qbs.installDir: buildconfig.installAppPath
    }
}
