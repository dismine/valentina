import qbs.FileInfo

VApp {
    Depends { name: "freedesktop2" }
    Depends { name: "tenv" }

    version: "0.7.52"
    install: true
    installDir: buildconfig.installAppPath
    installDebugInformation: true

    Properties {
        // Breakpoints do not work if debug the app inside of bundle. In debug mode we turn off creating a bundle.
        // Probably it will breake some dependencies. Version for Mac designed to work inside an app bundle.
        condition: qbs.targetOS.contains("macos") && qbs.buildVariant == "debug"
        bundle.isBundle: false
    }

    Properties {
        condition: buildconfig.enableAppImage && qbs.targetOS.contains("unix") && !qbs.targetOS.contains("macos")
        cpp.dynamicLibraries: ["icudata", "icui18n", "icuuc"]
    }

    Group {
        name: "freedesktop"
        prefix: FileInfo.joinPaths(project.sourceDirectory, "dist", FileInfo.pathSeparator())
        files: [
            product.targetName + ".desktop"
        ]
    }

    freedesktop2.desktopKeys: ({
        'Exec': FileInfo.joinPaths(qbs.installPrefix,
                                   product.installDir,
                                   product.targetName) + ' %F',
        'X-Application-Version': product.version,
    })
}
