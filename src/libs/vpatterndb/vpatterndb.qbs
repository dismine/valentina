VLib {
    Depends { name: "Qt"; submodules: ["concurrent"] }
    Depends { name: "VMiscLib" }
    Depends { name: "IFCLib" }
    Depends { name: "VGeometryLib" }
    Depends { name: "QMUParserLib" }
    Depends { name: "VFormatLib" }

    name: "VPatternDBLib"
    files: [
        "testpassmark.cpp",
        "vcontainer.cpp",
        "calculator.cpp",
        "vnodedetail.cpp",
        "vtranslatevars.cpp",
        "vformula.cpp",
        "vpiece.cpp",
        "vpiecenode.cpp",
        "vpiecepath.cpp",
        "vpassmark.cpp",
        "testpassmark.h",
        "vcontainer.h",
        "calculator.h",
        "vnodedetail.h",
        "vnodedetail_p.h",
        "vtranslatevars.h",
        "vformula.h",
        "vpiece.h",
        "vpiece_p.h",
        "vpiecenode.h",
        "vpiecenode_p.h",
        "vpiecepath.h",
        "vpiecepath_p.h",
        "vformula_p.h",
        "vpassmark.h",
    ]

    Group {
        name: "variables"
        prefix: "variables/"
        files: [
            "vpiecearea.cpp",
            "varcradius.cpp",
            "vcurveangle.cpp",
            "vcurvelength.cpp",
            "vcurvevariable.cpp",
            "vincrement.cpp",
            "vinternalvariable.cpp",
            "vlineangle.cpp",
            "vlinelength.cpp",
            "vmeasurement.cpp",
            "vvariable.cpp",
            "vcurveclength.cpp",
            "vpiecearea.h",
            "vpiecearea_p.h",
            "varcradius.h",
            "varcradius_p.h",
            "vcurveangle.h",
            "vcurvelength.h",
            "vcurvevariable.h",
            "vcurvevariable_p.h",
            "vincrement.h",
            "vincrement_p.h",
            "vinternalvariable.h",
            "vinternalvariable_p.h",
            "vlineangle.h",
            "vlineangle_p.h",
            "vlinelength.h",
            "vlinelength_p.h",
            "vmeasurement.h",
            "vmeasurement_p.h",
            "vvariable.h",
            "vvariable_p.h",
            "vcurveclength.h",
        ]
    }

    Group {
        name: "floatItemData"
        prefix: "floatItemData/"
        files: [
            "vpiecelabeldata.cpp",
            "vpatternlabeldata.cpp",
            "vgrainlinedata.cpp",
            "vabstractfloatitemdata.cpp",
            "vpiecelabeldata.h",
            "vpatternlabeldata.h",
            "vgrainlinedata.h",
            "vabstractfloatitemdata.h",
            "vabstractfloatitemdata_p.h",
            "vgrainlinedata_p.h",
            "floatitemdef.h",
            "vpatternlabeldata_p.h",
            "vpiecelabeldata_p.h",
        ]
    }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
