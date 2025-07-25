import qbs.Utilities

VLib {
    Depends { name: "Qt"; submodules: ["core", "widgets", "xml"] }
    Depends { name: "VMiscLib" }
    Depends { name: "VPropertyExplorerLib" }
    Depends { name: "VPatternDBLib" }

    Depends {
        name: "Qt.openglwidgets";
        condition: Utilities.versionCompare(Qt.core.version, "6") >= 0
    }

    name: "VWidgetsLib"
    files: [
        "vaspectratiopixmaplabel.cpp",
        "vaspectratiopixmaplabel.h",
        "vcomboboxdelegate.cpp",
        "vdecorationaligningdelegate.cpp",
        "velidedlabel.cpp",
        "vmaingraphicsscene.cpp",
        "vmaingraphicsview.cpp",
        "vmousewheelwidgetadjustmentguard.cpp",
        "vmousewheelwidgetadjustmentguard.h",
        "vtoolbuttonpopup.cpp",
        "vtoolbuttonpopup.h",
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
        "vhighlighter.h",
        "vpiecegrainline.h",
        "vpiecegrainline.cpp",
        "vpiecegrainline_p.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
