VLib {
    Depends { name: "Qt"; submodules: ["widgets"] }
    Depends { name: "VMiscLib" }
    Depends { name: "VPropertyExplorerLib" }

    name: "VWidgetsLib"
    files: [
        "qtcolorpicker.cpp",
        "vcomboboxdelegate.cpp",
        "vdecorationaligningdelegate.cpp",
        "velidedlabel.cpp",
        "vmaingraphicsscene.cpp",
        "vmaingraphicsview.cpp",
        "vwidgetpopup.cpp",
        "vcontrolpointspline.cpp",
        "vgraphicssimpletextitem.cpp",
        "vsimplepoint.cpp",
        "vabstractsimple.cpp",
        "vnobrushscalepathitem.cpp",
        "vsimplecurve.cpp",
        "vabstractmainwindow.cpp",
        "vtextgraphicsitem.cpp",
        "vgrainlineitem.cpp",
        "vpieceitem.cpp",
        "fancytabbar/fancytab.cpp",
        "fancytabbar/fancytabbar.cpp",
        "fancytabbar/stylehelper.cpp",
        "vcurvepathitem.cpp",
        "global.cpp",
        "vscenepoint.cpp",
        "scalesceneitems.cpp",
        "vlineedit.cpp",
        "vplaintextedit.cpp",
        "vhighlighter.cpp",
        "qtcolorpicker.h",
        "vcomboboxdelegate.h",
        "vdecorationaligningdelegate.h",
        "velidedlabel.h",
        "vmaingraphicsscene.h",
        "vmaingraphicsview.h",
        "vwidgetpopup.h",
        "vcontrolpointspline.h",
        "vgraphicssimpletextitem.h",
        "vsimplepoint.h",
        "vabstractsimple.h",
        "vnobrushscalepathitem.h",
        "vsimplecurve.h",
        "vabstractmainwindow.h",
        "vtextgraphicsitem.h",
        "vgrainlineitem.h",
        "vpieceitem.h",
        "fancytabbar/fancytab.h",
        "fancytabbar/fancytabbar.h",
        "fancytabbar/stylehelper.h",
        "vcurvepathitem.h",
        "global.h",
        "vscenepoint.h",
        "scalesceneitems.h",
        "vlineedit.h",
        "vplaintextedit.h",
        "vhighlighter.h"
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
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
