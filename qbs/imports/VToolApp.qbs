import qbs.FileInfo

VApp {
    Depends { name: "freedesktop2" }

    version: "0.7.52"
    type: base.concat("testSuit")
    install: true
    installDir: buildconfig.installAppPath
    installDebugInformation: true

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
