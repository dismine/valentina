VLib {
    Depends { name: "Qt"; submodules: ["gui", "xml", "svg", "xmlpatterns", "concurrent"] }
    Depends { name: "VMiscLib" }

    name: "IFCLib"
    files: [
        "ifcdef.h",
        "ifcdef.cpp",
    ]

    Group {
        name: "exception"
        prefix: "exception/"
        files: [
            "vexceptionobjecterror.h",
            "vexceptionemptyparameter.h",
            "vexceptionconversionerror.h",
            "vexceptionbadid.h",
            "vexception.h",
            "vexceptionterminatedposition.h",
            "vexceptionwrongid.h",
            "vexceptionundo.h",
            "vexceptioninvalidnotch.h",
            "vexceptioninvalidhistory.h",
            "vexceptionobjecterror.cpp",
            "vexceptionemptyparameter.cpp",
            "vexceptionconversionerror.cpp",
            "vexceptionbadid.cpp",
            "vexception.cpp",
            "vexceptionterminatedposition.cpp",
            "vexceptionwrongid.cpp",
            "vexceptionundo.cpp",
            "vexceptioninvalidnotch.cpp",
            "vexceptioninvalidhistory.cpp", 
        ]
    }

    Group {
        name: "xml"
        prefix: "xml/"
        files: [
            "utils.h",
            "vabstractconverter.h",
            "vbackgroundpatternimage.h",
            "vdomdocument.h",
            "vlayoutconverter.h",
            "vpatternconverter.h",
            "vpatternimage.h",
            "vtoolrecord.h",
            "vabstractpattern.h",
            "vvstconverter.h",
            "/vvitconverter.h",
            "/vabstractmconverter.h",
            "vlabeltemplateconverter.h",
            "vwatermarkconverter.h",
            "utils.cpp",
            "vabstractconverter.cpp",
            "vbackgroundpatternimage.cpp",
            "vdomdocument.cpp",
            "vlayoutconverter.cpp",
            "vpatternconverter.cpp",
            "vpatternimage.cpp",
            "vtoolrecord.cpp",
            "vabstractpattern.cpp",
            "vvstconverter.cpp",
            "vvitconverter.cpp",
            "vabstractmconverter.cpp",
            "vlabeltemplateconverter.cpp",
            "vwatermarkconverter.cpp",
        ]
    }

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

    Group {
        name: "Resources"
        files: [
            "schema.qrc",
        ]
    }

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["xml"] }
        Depends { name: "VMiscLib" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
