Project {
    name: "Valentina"
    minimumQbsVersion: "1.16"
    references: [
        "src/src.qbs",
        "share/translations/translations.qbs",
        "share/translations/measurements.qbs",
    ]
    qbsSearchPaths: "qbs"

    AutotestRunner {}
}
