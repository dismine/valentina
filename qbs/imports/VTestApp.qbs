VApp {
    type: base.concat("autotest")
    install: false
    condition: buildconfig.enableUnitTests

    Depends { name: "coverage" }

    Properties {
        condition: qbs.targetOS.contains("macos")
        bundle.isBundle: false
    }
}
