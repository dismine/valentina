import qbs.Utilities

VLib {
    Depends { name: "Qt"; submodules: ["core", "gui", "xml", "svg", "concurrent"] }
    Depends { name: "VMiscLib" }
    Depends { name: "GraafLib" }

    Depends {
        name: "Qt.xmlpatterns"
        condition: Utilities.versionCompare(Qt.core.version, "6") < 0
    }

    Depends {
        name: "xerces-c"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                   (!buildconfig.useConanPackages || (buildconfig.useConanPackages && !buildconfig.conanXercesEnabled))
    }

    Depends {
        name: "XercesC"
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 && buildconfig.useConanPackages &&
                   buildconfig.conanXercesEnabled
    }

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
            "vexceptionterminatedposition.h",
            "vexceptionwrongid.h",
            "vexceptionundo.h",
            "vexceptioninvalidnotch.h",
            "vexceptioninvalidhistory.h",
            "vexceptionobjecterror.cpp",
            "vexceptionemptyparameter.cpp",
            "vexceptionconversionerror.cpp",
            "vexceptionbadid.cpp",
            "vexceptionterminatedposition.cpp",
            "vexceptionwrongid.cpp",
            "vexceptionundo.cpp",
            "vexceptioninvalidnotch.cpp",
            "vexceptioninvalidhistory.cpp"
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
            "vparsererrorhandler.cpp",
            "vparsererrorhandler.h",
            "vpatternblockmapper.cpp",
            "vpatternblockmapper.h",
            "vpatternconverter.h",
            "vpatterngraph.cpp",
            "vpatterngraph.h",
            "vpatterngraphnode.cpp",
            "vpatterngraphnode.h",
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
            "vknownmeasurementsconverter.cpp",
            "vknownmeasurementsconverter.h",
        ]
    }

    Group {
        name: "Resources"
        files: [
            "schema.qrc",
        ]
    }

    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["core", "xml"] }
        Depends { name: "VMiscLib" }
        Depends { name: "xerces-c"; condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                                               (!buildconfig.useConanPackages ||
                                                (buildconfig.useConanPackages && !buildconfig.conanXercesEnabled)) }
        Depends { name: "XercesC"; condition: Utilities.versionCompare(Qt.core.version, "6") >= 0 &&
                                              buildconfig.useConanPackages && buildconfig.conanXercesEnabled }
        Depends { name: "GraafLib" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
