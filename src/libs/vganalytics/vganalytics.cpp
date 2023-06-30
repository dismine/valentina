/************************************************************************
 **
 **  @file   vganalytics.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 6, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
 **  <https://gitlab.com/smart-pattern/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/
#include "vganalytics.h"
#include "vganalyticsworker.h"

#include <QDataStream>
#include <QDebug>
#include <QGuiApplication>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQueue>
#include <QScreen>
#include <QSettings>
#include <QStringLiteral>
#include <QTimer>
#include <QUrlQuery>
#include <QUuid>

//---------------------------------------------------------------------------------------------------------------------
VGAnalytics::VGAnalytics(QObject *parent)
  : QObject{parent},
    d(new VGAnalyticsWorker(this))
{
}

//---------------------------------------------------------------------------------------------------------------------
VGAnalytics::~VGAnalytics()
{
    delete d;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::Instance() -> VGAnalytics *
{
    static VGAnalytics *instance = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    if (instance == nullptr)
    {
        instance = new VGAnalytics();
    }

    return instance;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetLogLevel(enum VGAnalytics::LogLevel logLevel)
{
    d->m_logLevel = logLevel;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::LogLevel() const -> enum VGAnalytics::LogLevel { return d->m_logLevel; }

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetRepoRevision(const QString &rev)
{
    d->m_repoRevision = !rev.isEmpty() ? rev : QStringLiteral("Unknown");
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::RepoRevision() const -> QString
{
    return d->m_repoRevision;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetGUILanguage(const QString &language)
{
    d->m_guiLanguage = language.toLower().replace(QChar('_'), QChar('-'));
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::GUILanguage() const -> QString
{
    return d->m_guiLanguage;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetSendInterval(int milliseconds)
{
    d->m_timer.setInterval(milliseconds);
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::SendInterval() const -> int
{
    return (d->m_timer.interval());
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::IsEnabled() -> bool
{
    return d->m_isEnabled;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetMeasurementId(const QString &measurementId)
{
    d->m_measurementId = measurementId;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetApiSecret(const QString &apiSecret)
{
    d->m_apiSecret = apiSecret;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetClientID(const QString &clientID)
{
    d->m_clientID = clientID;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::Enable(bool state)
{
    d->Enable(state);
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SetNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
    if (d->networkManager != networkAccessManager)
    {
        // Delete the old network manager if it was our child
        if (d->networkManager && d->networkManager->parent() == this)
        {
            d->networkManager->deleteLater();
        }

        d->networkManager = networkAccessManager;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::NetworkAccessManager() const -> QNetworkAccessManager *
{
    return d->networkManager;
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendAppFreshInstallEvent(qint64 engagementTimeMsec)
{
    SendEvent(QStringLiteral("vapp_fresh_install"), InitAppStartEventParams(engagementTimeMsec));
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendAppStartEvent(qint64 engagementTimeMsec)
{
    SendEvent(QStringLiteral("vapp_start"), InitAppStartEventParams(engagementTimeMsec));
}

//---------------------------------------------------------------------------------------------------------------------
void VGAnalytics::SendAppCloseEvent(qint64 engagementTimeMsec)
{
    QHash<QString, QJsonValue> params{
        // {QStringLiteral("category"), ""},
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id
        // must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
    };

    SendEvent(QStringLiteral("vapp_close"), params);
    QNetworkReply *reply = d->PostMessage();
    if (reply)
    {
        QTimer timer;
        const int timeoutSeconds = 3; // Wait for 3 seconds
        timer.setSingleShot(true);
        timer.start(timeoutSeconds * 1000);

        QEventLoop loop;
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec(); // wait for finished
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * A query for a POST message will be created to report this event. The
 * created query will be stored in a message queue.
 */
void VGAnalytics::SendEvent(const QString &eventName, const QHash<QString, QJsonValue> &params)
{
    /*
    // event body example
    {
        "client_id": "XXXXXXXXXX.YYYYYYYYYY",
        "events": [{
            "name": "refund",
            "params": {
                "currency": "USD",
                "value": "9.99",
                "transaction_id": "ABC-123"
            }
        }]
    }
    */

    QJsonObject root;
    root[QStringLiteral("client_id")] = d->m_clientID;

    QJsonObject event;
    event[QStringLiteral("name")] = eventName;

    QJsonObject eventParams;

    auto i = params.constBegin();
    while (i != params.constEnd())
    {
        eventParams[i.key()] = i.value();
        ++i;
    }

    // In order for user activity to display in standard reports like Realtime, engagement_time_msec and session_id must
    // be supplied as part of the params for an event.
    // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    eventParams[QStringLiteral("session_id")] = qApp->sessionId();
    event[QStringLiteral("params")] = eventParams;

    QJsonArray eventArray;
    eventArray.append(event);
    root[QStringLiteral("events")] = eventArray;

    d->EnqueQueryWithCurrentTime(root);
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::InitAppStartEventParams(qint64 engagementTimeMsec) const -> QHash<QString, QJsonValue>
{
    QHash<QString, QJsonValue> params{
        // {QStringLiteral("category"), ""},
        {QStringLiteral("qt_version"), QT_VERSION_STR},
        {QStringLiteral("app_version"), d->m_appVersion},
        {QStringLiteral("word_size"), QStringLiteral("%1 bit").arg(QSysInfo::WordSize)},
        {QStringLiteral("cpu_architecture"), QSysInfo::currentCpuArchitecture()},
        {QStringLiteral("revision"), d->m_repoRevision},
        {QStringLiteral("os_version"), QSysInfo::prettyProductName()},
        {QStringLiteral("screen_size"), d->m_screenResolution},
        {QStringLiteral("screen_scale_factor"), d->m_screenScaleFactor},
        // In order for user activity to display in standard reports like Realtime, engagement_time_msec and
        // session_id must be supplied as part of the params for an event.
        // https://developers.google.com/analytics/devguides/collection/protocol/ga4/sending-events?client_type=gtag#optional_parameters_for_reports
        {QStringLiteral("engagement_time_msec"), engagementTimeMsec},
        {QStringLiteral("gui_language"), d->m_guiLanguage},
        {QStringLiteral("countryId"), TerritoryCode()},
    };
    return params;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGAnalytics::TerritoryCode() -> QString
{
    QLocale loc = QLocale::system();

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    return QLocale::territoryToCode(loc.territory());
#elif QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
    return QLocale::countryToCode(loc.country());
#else
    return GetTerritoryCode(loc.country());
#endif
}

//---------------------------------------------------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK(6, 1, 0)
auto VGAnalytics::GetTerritoryCode(QLocale::Country territory) -> QString
{
    if (territory == QLocale::AnyCountry || territory > QLocale::LastCountry)
    {
        return {};
    }

    static const QHash<QLocale::Country, QString> territoryCodeList = {
        {QLocale::Afghanistan, QLatin1String("AF")},
        {QLocale::AlandIslands, QLatin1String("AX")},
        {QLocale::Albania, QLatin1String("AL")},
        {QLocale::Algeria, QLatin1String("DZ")},
        {QLocale::AmericanSamoa, QLatin1String("AS")},
        {QLocale::Andorra, QLatin1String("AD")},
        {QLocale::Angola, QLatin1String("AO")},
        {QLocale::Anguilla, QLatin1String("AI")},
        {QLocale::Antarctica, QLatin1String("AQ")},
        {QLocale::AntiguaAndBarbuda, QLatin1String("AG")},
        {QLocale::Argentina, QLatin1String("AR")},
        {QLocale::Armenia, QLatin1String("AM")},
        {QLocale::Aruba, QLatin1String("AW")},
        {QLocale::AscensionIsland, QLatin1String("AC")},
        {QLocale::Australia, QLatin1String("AU")},
        {QLocale::Austria, QLatin1String("AT")},
        {QLocale::Azerbaijan, QLatin1String("AZ")},
        {QLocale::Bahamas, QLatin1String("BS")},
        {QLocale::Bahrain, QLatin1String("BH")},
        {QLocale::Bangladesh, QLatin1String("BD")},
        {QLocale::Barbados, QLatin1String("BB")},
        {QLocale::Belarus, QLatin1String("BY")},
        {QLocale::Belgium, QLatin1String("BE")},
        {QLocale::Belize, QLatin1String("BZ")},
        {QLocale::Benin, QLatin1String("BJ")},
        {QLocale::Bermuda, QLatin1String("BM")},
        {QLocale::Bhutan, QLatin1String("BT")},
        {QLocale::Bolivia, QLatin1String("BO")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::BosniaAndHerzegovina, QLatin1String("BA")},
#else
        {QLocale::BosniaAndHerzegowina, QLatin1String("BA")},
#endif
        {QLocale::Botswana, QLatin1String("BW")},
        {QLocale::BouvetIsland, QLatin1String("BV")},
        {QLocale::Brazil, QLatin1String("BR")},
        {QLocale::BritishIndianOceanTerritory, QLatin1String("IO")},
        {QLocale::BritishVirginIslands, QLatin1String("VG")},
        {QLocale::Brunei, QLatin1String("BN")},
        {QLocale::Bulgaria, QLatin1String("BG")},
        {QLocale::BurkinaFaso, QLatin1String("BF")},
        {QLocale::Burundi, QLatin1String("BI")},
        {QLocale::Cambodia, QLatin1String("KH")},
        {QLocale::Cameroon, QLatin1String("CM")},
        {QLocale::Canada, QLatin1String("CA")},
        {QLocale::CanaryIslands, QLatin1String("IC")},
        {QLocale::CapeVerde, QLatin1String("CV")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::CaribbeanNetherlands, QLatin1String("BQ")},
#endif
        {QLocale::CaymanIslands, QLatin1String("KY")},
        {QLocale::CentralAfricanRepublic, QLatin1String("CF")},
        {QLocale::CeutaAndMelilla, QLatin1String("EA")},
        {QLocale::Chad, QLatin1String("TD")},
        {QLocale::Chile, QLatin1String("CL")},
        {QLocale::China, QLatin1String("CN")},
        {QLocale::ChristmasIsland, QLatin1String("CX")},
        {QLocale::ClippertonIsland, QLatin1String("CP")},
        {QLocale::CocosIslands, QLatin1String("CC")},
        {QLocale::Colombia, QLatin1String("CO")},
        {QLocale::Comoros, QLatin1String("KM")},
        {QLocale::CongoBrazzaville, QLatin1String("CG")},
        {QLocale::CongoKinshasa, QLatin1String("CD")},
        {QLocale::CookIslands, QLatin1String("CK")},
        {QLocale::CostaRica, QLatin1String("CR")},
        {QLocale::Croatia, QLatin1String("HR")},
        {QLocale::Cuba, QLatin1String("CU")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::Curacao, QLatin1String("CW")},
#else
        {QLocale::CuraSao, QLatin1String("CW")},
#endif
        {QLocale::Cyprus, QLatin1String("CY")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::Czechia, QLatin1String("CZ")},
#else
        {QLocale::CzechRepublic, QLatin1String("CZ")},
#endif
        {QLocale::Denmark, QLatin1String("DK")},
        {QLocale::DiegoGarcia, QLatin1String("DG")},
        {QLocale::Djibouti, QLatin1String("DJ")},
        {QLocale::Dominica, QLatin1String("DM")},
        {QLocale::DominicanRepublic, QLatin1String("DO")},
        {QLocale::Ecuador, QLatin1String("EC")},
        {QLocale::Egypt, QLatin1String("EG")},
        {QLocale::ElSalvador, QLatin1String("SV")},
        {QLocale::EquatorialGuinea, QLatin1String("GQ")},
        {QLocale::Eritrea, QLatin1String("ER")},
        {QLocale::Estonia, QLatin1String("EE")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::Eswatini, QLatin1String("SZ")},
#else
        {QLocale::Swaziland, QLatin1String("SZ")},
#endif
        {QLocale::Ethiopia, QLatin1String("ET")},
        {QLocale::Europe, QLatin1String("150")},
        {QLocale::EuropeanUnion, QLatin1String("EU")},
        {QLocale::FalklandIslands, QLatin1String("FK")},
        {QLocale::FaroeIslands, QLatin1String("FO")},
        {QLocale::Fiji, QLatin1String("FJ")},
        {QLocale::Finland, QLatin1String("FI")},
        {QLocale::France, QLatin1String("FR")},
        {QLocale::FrenchGuiana, QLatin1String("GF")},
        {QLocale::FrenchPolynesia, QLatin1String("PF")},
        {QLocale::FrenchSouthernTerritories, QLatin1String("TF")},
        {QLocale::Gabon, QLatin1String("GA")},
        {QLocale::Gambia, QLatin1String("GM")},
        {QLocale::Georgia, QLatin1String("GE")},
        {QLocale::Germany, QLatin1String("DE")},
        {QLocale::Ghana, QLatin1String("GH")},
        {QLocale::Gibraltar, QLatin1String("GI")},
        {QLocale::Greece, QLatin1String("GR")},
        {QLocale::Greenland, QLatin1String("GL")},
        {QLocale::Grenada, QLatin1String("GD")},
        {QLocale::Guadeloupe, QLatin1String("GP")},
        {QLocale::Guam, QLatin1String("GU")},
        {QLocale::Guatemala, QLatin1String("GT")},
        {QLocale::Guernsey, QLatin1String("GG")},
        {QLocale::GuineaBissau, QLatin1String("GW")},
        {QLocale::Guinea, QLatin1String("GN")},
        {QLocale::Guyana, QLatin1String("GY")},
        {QLocale::Haiti, QLatin1String("HT")},
        {QLocale::HeardAndMcDonaldIslands, QLatin1String("HM")},
        {QLocale::Honduras, QLatin1String("HN")},
        {QLocale::HongKong, QLatin1String("HK")},
        {QLocale::Hungary, QLatin1String("HU")},
        {QLocale::Iceland, QLatin1String("IS")},
        {QLocale::India, QLatin1String("IN")},
        {QLocale::Indonesia, QLatin1String("ID")},
        {QLocale::Iran, QLatin1String("IR")},
        {QLocale::Iraq, QLatin1String("IQ")},
        {QLocale::Ireland, QLatin1String("IE")},
        {QLocale::IsleOfMan, QLatin1String("IM")},
        {QLocale::Israel, QLatin1String("IL")},
        {QLocale::Italy, QLatin1String("IT")},
        {QLocale::IvoryCoast, QLatin1String("CI")},
        {QLocale::Jamaica, QLatin1String("JM")},
        {QLocale::Japan, QLatin1String("JP")},
        {QLocale::Jersey, QLatin1String("JE")},
        {QLocale::Jordan, QLatin1String("JO")},
        {QLocale::Kazakhstan, QLatin1String("KZ")},
        {QLocale::Kenya, QLatin1String("KE")},
        {QLocale::Kiribati, QLatin1String("KI")},
        {QLocale::Kosovo, QLatin1String("XK")},
        {QLocale::Kuwait, QLatin1String("KW")},
        {QLocale::Kyrgyzstan, QLatin1String("KG")},
        {QLocale::Laos, QLatin1String("LA")},
        {QLocale::LatinAmerica, QLatin1String("419")},
        {QLocale::Latvia, QLatin1String("LV")},
        {QLocale::Lebanon, QLatin1String("LB")},
        {QLocale::Lesotho, QLatin1String("LS")},
        {QLocale::Liberia, QLatin1String("LR")},
        {QLocale::Libya, QLatin1String("LY")},
        {QLocale::Liechtenstein, QLatin1String("LI")},
        {QLocale::Lithuania, QLatin1String("LT")},
        {QLocale::Luxembourg, QLatin1String("LU")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::Macao, QLatin1String("MO")},
#else
        {QLocale::Macau, QLatin1String("MO")},
#endif
        {QLocale::Macedonia, QLatin1String("MK")},
        {QLocale::Madagascar, QLatin1String("MG")},
        {QLocale::Malawi, QLatin1String("MW")},
        {QLocale::Malaysia, QLatin1String("MY")},
        {QLocale::Maldives, QLatin1String("MV")},
        {QLocale::Mali, QLatin1String("ML")},
        {QLocale::Malta, QLatin1String("MT")},
        {QLocale::MarshallIslands, QLatin1String("MH")},
        {QLocale::Martinique, QLatin1String("MQ")},
        {QLocale::Mauritania, QLatin1String("MR")},
        {QLocale::Mauritius, QLatin1String("MU")},
        {QLocale::Mayotte, QLatin1String("YT")},
        {QLocale::Mexico, QLatin1String("MX")},
        {QLocale::Micronesia, QLatin1String("FM")},
        {QLocale::Moldova, QLatin1String("MD")},
        {QLocale::Monaco, QLatin1String("MC")},
        {QLocale::Mongolia, QLatin1String("MN")},
        {QLocale::Montenegro, QLatin1String("ME")},
        {QLocale::Montserrat, QLatin1String("MS")},
        {QLocale::Morocco, QLatin1String("MA")},
        {QLocale::Mozambique, QLatin1String("MZ")},
        {QLocale::Myanmar, QLatin1String("MM")},
        {QLocale::Namibia, QLatin1String("NA")},
        {QLocale::NauruCountry, QLatin1String("NR")},
        {QLocale::Nepal, QLatin1String("NP")},
        {QLocale::Netherlands, QLatin1String("NL")},
        {QLocale::NewCaledonia, QLatin1String("NC")},
        {QLocale::NewZealand, QLatin1String("NZ")},
        {QLocale::Nicaragua, QLatin1String("NI")},
        {QLocale::Nigeria, QLatin1String("NG")},
        {QLocale::Niger, QLatin1String("NE")},
        {QLocale::Niue, QLatin1String("NU")},
        {QLocale::NorfolkIsland, QLatin1String("NF")},
        {QLocale::NorthernMarianaIslands, QLatin1String("MP")},
        {QLocale::NorthKorea, QLatin1String("KP")},
        {QLocale::Norway, QLatin1String("NO")},
        {QLocale::Oman, QLatin1String("OM")},
        {QLocale::OutlyingOceania, QLatin1String("QO")},
        {QLocale::Pakistan, QLatin1String("PK")},
        {QLocale::Palau, QLatin1String("PW")},
        {QLocale::PalestinianTerritories, QLatin1String("PS")},
        {QLocale::Panama, QLatin1String("PA")},
        {QLocale::PapuaNewGuinea, QLatin1String("PG")},
        {QLocale::Paraguay, QLatin1String("PY")},
        {QLocale::Peru, QLatin1String("PE")},
        {QLocale::Philippines, QLatin1String("PH")},
        {QLocale::Pitcairn, QLatin1String("PN")},
        {QLocale::Poland, QLatin1String("PL")},
        {QLocale::Portugal, QLatin1String("PT")},
        {QLocale::PuertoRico, QLatin1String("PR")},
        {QLocale::Qatar, QLatin1String("QA")},
        {QLocale::Reunion, QLatin1String("RE")},
        {QLocale::Romania, QLatin1String("RO")},
        {QLocale::Russia, QLatin1String("RU")},
        {QLocale::Rwanda, QLatin1String("RW")},
        {QLocale::SaintBarthelemy, QLatin1String("BL")},
        {QLocale::SaintHelena, QLatin1String("SH")},
        {QLocale::SaintKittsAndNevis, QLatin1String("KN")},
        {QLocale::SaintLucia, QLatin1String("LC")},
        {QLocale::SaintMartin, QLatin1String("MF")},
        {QLocale::SaintPierreAndMiquelon, QLatin1String("PM")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::SaintVincentAndGrenadines, QLatin1String("VC")},
#else
        {QLocale::SaintVincentAndTheGrenadines, QLatin1String("VC")},
#endif
        {QLocale::Samoa, QLatin1String("WS")},
        {QLocale::SanMarino, QLatin1String("SM")},
        {QLocale::SaoTomeAndPrincipe, QLatin1String("ST")},
        {QLocale::SaudiArabia, QLatin1String("SA")},
        {QLocale::Senegal, QLatin1String("SN")},
        {QLocale::Serbia, QLatin1String("RS")},
        {QLocale::Seychelles, QLatin1String("SC")},
        {QLocale::SierraLeone, QLatin1String("SL")},
        {QLocale::Singapore, QLatin1String("SG")},
        {QLocale::SintMaarten, QLatin1String("SX")},
        {QLocale::Slovakia, QLatin1String("SK")},
        {QLocale::Slovenia, QLatin1String("SI")},
        {QLocale::SolomonIslands, QLatin1String("SB")},
        {QLocale::Somalia, QLatin1String("SO")},
        {QLocale::SouthAfrica, QLatin1String("ZA")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::SouthGeorgiaAndSouthSandwichIslands, QLatin1String("GS")},
#else
        {QLocale::SouthGeorgiaAndTheSouthSandwichIslands, QLatin1String("GS")},
#endif
        {QLocale::SouthKorea, QLatin1String("KR")},
        {QLocale::SouthSudan, QLatin1String("SS")},
        {QLocale::Spain, QLatin1String("ES")},
        {QLocale::SriLanka, QLatin1String("LK")},
        {QLocale::Sudan, QLatin1String("SD")},
        {QLocale::Suriname, QLatin1String("SR")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::SvalbardAndJanMayen, QLatin1String("SJ")},
#else
        {QLocale::SvalbardAndJanMayenIslands, QLatin1String("SJ")},
#endif

        {QLocale::Sweden, QLatin1String("SE")},
        {QLocale::Switzerland, QLatin1String("CH")},
        {QLocale::Syria, QLatin1String("SY")},
        {QLocale::Taiwan, QLatin1String("TW")},
        {QLocale::Tajikistan, QLatin1String("TJ")},
        {QLocale::Tanzania, QLatin1String("TZ")},
        {QLocale::Thailand, QLatin1String("TH")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::TimorLeste, QLatin1String("TL")},
#else
        {QLocale::EastTimor, QLatin1String("TL")},
#endif
        {QLocale::Togo, QLatin1String("TG")},
        {QLocale::TokelauCountry, QLatin1String("TK")},
        {QLocale::Tonga, QLatin1String("TO")},
        {QLocale::TrinidadAndTobago, QLatin1String("TT")},
        {QLocale::TristanDaCunha, QLatin1String("TA")},
        {QLocale::Tunisia, QLatin1String("TN")},
        {QLocale::Turkey, QLatin1String("TR")},
        {QLocale::Turkmenistan, QLatin1String("TM")},
        {QLocale::TurksAndCaicosIslands, QLatin1String("TC")},
        {QLocale::TuvaluCountry, QLatin1String("TV")},
        {QLocale::Uganda, QLatin1String("UG")},
        {QLocale::Ukraine, QLatin1String("UA")},
        {QLocale::UnitedArabEmirates, QLatin1String("AE")},
        {QLocale::UnitedKingdom, QLatin1String("GB")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::UnitedStatesOutlyingIslands, QLatin1String("UM")},
#else
        {QLocale::UnitedStatesMinorOutlyingIslands, QLatin1String("UM")},
#endif
        {QLocale::UnitedStates, QLatin1String("US")},
        {QLocale::UnitedStatesVirginIslands, QLatin1String("VI")},
        {QLocale::Uruguay, QLatin1String("UY")},
        {QLocale::Uzbekistan, QLatin1String("UZ")},
        {QLocale::Vanuatu, QLatin1String("VU")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::VaticanCity, QLatin1String("VA")},
#else
        {QLocale::VaticanCityState, QLatin1String("VA")},
#endif
        {QLocale::Venezuela, QLatin1String("VE")},
        {QLocale::Vietnam, QLatin1String("VN")},
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        {QLocale::WallisAndFutuna, QLatin1String("WF")},
#else
        {QLocale::WallisAndFutunaIslands, QLatin1String("WF")},
#endif
        {QLocale::WesternSahara, QLatin1String("EH")},
        {QLocale::World, QLatin1String("001")},
        {QLocale::Yemen, QLatin1String("YE")},
        {QLocale::Zambia, QLatin1String("ZM")},
        {QLocale::Zimbabwe, QLatin1String("ZW")},
    };

    if (territoryCodeList.contains(territory))
    {
        return territoryCodeList.value(territory);
    }

    return {};
}
#endif

//---------------------------------------------------------------------------------------------------------------------
/**
 * Qut stream to persist class GAnalytics.
 */
auto operator<<(QDataStream &outStream, const VGAnalytics &analytics) -> QDataStream &
{
    outStream << analytics.d->PersistMessageQueue();

    return outStream;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * In stream to read GAnalytics from file.
 */
auto operator>>(QDataStream &inStream, VGAnalytics &analytics) -> QDataStream &
{
    QList<QString> dataList;
    inStream >> dataList;
    analytics.d->ReadMessagesFromFile(dataList);

    return inStream;
}
