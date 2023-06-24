import qbs.Utilities

VLib {
    Depends { name: "windeployqt"; }
    Depends { name: "i18nconfig"; }

    buildconfig.staticBuild: false

    Properties {
        condition: qbs.targetOS.contains("windows") && Utilities.versionCompare(Qt.core.version, "6.5") < 0
        windeployqt.noVirtualkeyboard: true
    }

    Properties {
        condition: qbs.targetOS.contains("macos")
        bundle.identifierPrefix: 'ua.com.smart-pattern'
    }

    Properties {
        condition: i18nconfig.limitDeploymentOfQtTranslations
        windeployqt.languages: i18nconfig.qtTranslationLocales.join(',')
    }

    installDebugInformation: qbs.buildVariant !== "release"

    Properties {
        condition: !qbs.targetOS.contains("macos") || (qbs.targetOS.contains("macos") && !buildconfig.enableMultiBundle)
        install: true
        installDir: buildconfig.installLibraryPath
    }
}
