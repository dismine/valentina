VDynamicLib {
    Depends { name: "Qt"; submodules: ["gui", "widgets"] }
    Depends { name: "VMiscLib" }
    Depends { name: "multibundle"; }

    name: "VPropertyExplorerLib"
    version: "1.0.0"
    files: [
        "vproperty.cpp",
        "vpropertydef.cpp",
        "vpropertydef.h",
        "vpropertydelegate.cpp",
        "vpropertyfactorymanager.cpp",
        "vpropertyformview.cpp",
        "vpropertyformwidget.cpp",
        "vpropertymodel.cpp",
        "vpropertyset.cpp",
        "vpropertytreeview.cpp",
        "vserializedproperty.cpp",
        "vstandardpropertyfactory.cpp",
        "checkablemessagebox.cpp",
        "vpropertyexplorer_global.h",
        "vpropertyfactorymanager_p.h",
        "vpropertytreeview_p.h",
        "vpropertyset_p.h",
        "vabstractpropertyfactory.h",
        "vfileproperty_p.h",
        "vwidgetproperty_p.h",
        "vpropertymodel_p.h",
        "vstandardpropertyfactory.h",
        "vpropertyformview_p.h",
        "vpropertytreeview.h",
        "vpropertyformwidget_p.h",
        "vpropertydelegate.h",
        "vproperty_p.h",
        "vpropertyformwidget.h",
        "vpropertyformview.h",
        "vpropertyset.h",
        "vpropertymodel.h",
        "vproperty.h",
        "vpropertyfactorymanager.h",
        "vserializedproperty.h",
        "vproperties.h",
        "checkablemessagebox.h",
    ]

    Group {
        name: "plugins"
        prefix: "plugins/"
        files: [
            "vtextproperty.cpp",
            "vwidgetproperty.cpp",
            "vemptyproperty.cpp",
            "vboolproperty.cpp",
            "vshortcutproperty.cpp",
            "vcolorproperty.cpp",
            "vshortcutpropertyeditor.cpp",
            "venumproperty.cpp",
            "vfileproperty.cpp",
            "vcolorpropertyeditor.cpp",
            "vfilepropertyeditor.cpp",
            "vnumberproperty.cpp",
            "Vector3d/vvector3dproperty.cpp",
            "vstringproperty.cpp",
            "vpointfproperty.cpp",
            "vobjectproperty.cpp",
            "vlinetypeproperty.cpp",
            "vlinecolorproperty.cpp",
            "vlabelproperty.cpp",
            "vtextproperty.h",
            "vwidgetproperty.h",
            "vcolorproperty.h",
            "vboolproperty.h",
            "vcolorpropertyeditor.h",
            "vshortcutpropertyeditor.h",
            "vemptyproperty.h",
            "vshortcutproperty.h",
            "venumproperty.h",
            "vfilepropertyeditor.h",
            "vfileproperty.h",
            "vnumberproperty.h",
            "Vector3d/vvector3dproperty.h",
            "vstringproperty.h",
            "vpointfproperty.h",
            "vobjectproperty.h",
            "vlinetypeproperty.h",
            "vlinecolorproperty.h",
            "vlabelproperty.h",
        ]
    }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }

    cpp.defines: "VPROPERTYEXPLORER_LIBRARY"
    multibundle.targetApps: ["Valentina", "Tape", "Puzzle"]
}
