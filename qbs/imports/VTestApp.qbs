VApp {
    type: base.concat("autotest")
    bundle.isBundle: false
    install: false
    condition: buildconfig.enableUnitTests
}
