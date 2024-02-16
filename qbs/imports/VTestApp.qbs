VApp {
    type: base.concat("autotest")
    install: false
    condition: buildconfig.enableUnitTests

    Depends { name: "coverage"; required: false }

    Properties {
        condition: qbs.targetOS.contains("macos")
        bundle.isBundle: false
    }
}
