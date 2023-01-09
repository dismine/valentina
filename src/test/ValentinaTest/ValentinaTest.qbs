VApp {
    Depends { name: "buildconfig" }
    Depends { name: "VTestLib" }
    Depends { name: "Qt"; submodules: ["testlib"] }
    Depends { name: "VToolsLib" }
    Depends { name: "VGeometryLib" }
    Depends { name: "IFCLib" }
    Depends { name: "VDXFLib" }
    Depends { name: "VFormatLib" }

    name: "ValentinaTest"
    buildconfig.appTarget: qbs.targetOS.contains("macos") ? "ValentinaTest" : "valentinaTest"
    targetName: buildconfig.appTarget
    type: base.concat("autotest")
    bundle.isBundle: false
    install: false
    condition: buildconfig.enableUnitTests

    files: [
      "qttestmainlambda.cpp",
      "tst_dxf.cpp",
      "tst_vdomdocument.cpp",
      "tst_vposter.cpp",
      "tst_vspline.cpp",
      "tst_nameregexp.cpp",
      "tst_vlayoutdetail.cpp",
      "tst_varc.cpp",
      "tst_qmutokenparser.cpp",
      "tst_vmeasurements.cpp",
      "tst_vlockguard.cpp",
      "tst_misc.cpp",
      "tst_vcommandline.cpp",
      "tst_vpiece.cpp",
      "tst_vabstractcurve.cpp",
      "tst_findpoint.cpp",
      "tst_vellipticalarc.cpp",
      "tst_vcubicbezierpath.cpp",
      "tst_vgobject.cpp",
      "tst_vsplinepath.cpp",
      "tst_vpointf.cpp",
      "tst_readval.cpp",
      "tst_vtranslatevars.cpp",
      "tst_vabstractpiece.cpp",
      "tst_vtooluniondetails.cpp",
      "tst_dxf.h",
      "tst_vdomdocument.h",
      "tst_vposter.h",
      "tst_vspline.h",
      "tst_nameregexp.h",
      "tst_vlayoutdetail.h",
      "tst_varc.h",
      "tst_qmutokenparser.h",
      "tst_vmeasurements.h",
      "tst_vlockguard.h",
      "tst_misc.h",
      "tst_vcommandline.h",
      "tst_vpiece.h",
      "tst_vabstractcurve.h",
      "tst_findpoint.h",
      "tst_vellipticalarc.h",
      "tst_vcubicbezierpath.h",
      "tst_vgobject.h",
      "tst_vsplinepath.h",
      "tst_vpointf.h",
      "tst_readval.h",
      "tst_vtranslatevars.h",
      "tst_vabstractpiece.h",
      "tst_vtooluniondetails.h", 
    ]

    Group {
        name: "Precompiled headers"
        files: {
            var files = ["stable.h"];
            if (qbs.toolchain.contains("msvc"))
                files.push("stable.cpp")
            return files;
        }
        fileTags: ["cpp_pch_src"]
    }
}
