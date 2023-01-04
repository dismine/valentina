VLib {
    name: "QMUParserLib"
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

    buildconfig.staticBuild: false

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
