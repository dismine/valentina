VApp {
    type: base.concat("autotest")
    install: false
    condition: buildconfig.enableUnitTests

    Properties {
        condition: qbs.targetOS.contains("macos")
        bundle.isBundle: false
    }
}
