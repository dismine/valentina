VLib {
    Depends { name: "windeployqt"; }
    Depends { name: "i18nconfig"; }

    buildconfig.staticBuild: false

    windeployqt.noVirtualkeyboard: true

    Properties {
        condition: i18nconfig.limitDeploymentOfQtTranslations
        windeployqt.languages: i18nconfig.qtTranslationLocales.join(',')
    }
}
