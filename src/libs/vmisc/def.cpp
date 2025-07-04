/************************************************************************
 **
 **  @file   def.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "def.h"

#include <QApplication>
#include <QChar>
#include <QColor>
#include <QComboBox>
#include <QCursor>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QGlobalStatic>
#include <QGraphicsItem>
#include <QGuiApplication>
#include <QImage>
#include <QLatin1Char>
#include <QList>
#include <QMap>
#include <QMessageLogger>
#include <QObject>
#include <QPixmap>
#include <QPixmapCache>
#include <QPrinterInfo>
#include <QProcess>
#include <QRgb>
#include <QUrl>
#include <QtDebug>

#include "../ifc/exception/vexception.h"
#include "compatibility.h"
#include "literals.h"
#include "vabstractapplication.h"

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto LocalList() -> QStringList
{
    static QStringList fileNames;

    // Check if file names have already been cached
    if (!fileNames.isEmpty())
    {
        return fileNames;
    }

    QDirIterator it(VAbstractApplication::translationsPath(), QStringList("valentina_*.qm"), QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        fileNames.append(it.fileName());
    }
    return fileNames;
}

//---------------------------------------------------------------------------------------------------------------------
void InitLanguageList(QComboBox *combobox)
{
    SCASSERT(combobox != nullptr)
    combobox->clear();

    const QStringList fileNames = LocalList();
    const QStringList supportedLocales = SupportedLocales();

    for (auto locale : fileNames)
    {
        // get locale extracted by filename           "valentina_de_De.qm"
        locale.truncate(locale.lastIndexOf('.'_L1));  // "valentina_de_De"
        locale.remove(0, locale.indexOf('_'_L1) + 1); // "de_De"

        if (locale.startsWith("ru"_L1) || locale.startsWith("ir"_L1) || !supportedLocales.contains(locale))
        {
            continue;
        }

        auto const loc = QLocale(locale);
        QString const lang = loc.nativeLanguageName();
        QString country = TerritoryToString(loc);
        if (country == "Czechia"_L1)
        {
            country = "CzechRepublic"_L1;
        }
        QIcon const ico(u"://flags/%1.png"_s.arg(country));

        combobox->addItem(ico, lang, locale);
    }

    // English language is internal and doesn't have own *.qm file.
    // Since Qt 5.12 country names have spaces
    QIcon const ico(u"://flags/%1.png"_s.arg(TerritoryToString(QLocale::UnitedStates)));
    const auto en_US = QStringLiteral("en_US");
    QString const lang = QLocale(en_US).nativeLanguageName();
    combobox->addItem(ico, lang, en_US);
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto QPixmapFromCache(const QString &pixmapPath) -> QPixmap
{
    QPixmap pixmap;

    if (not QPixmapCache::find(pixmapPath, &pixmap))
    {
        pixmap = QPixmap(pixmapPath);
        QPixmapCache::insert(pixmapPath, pixmap);
    }
    return pixmap;
}

//---------------------------------------------------------------------------------------------------------------------
void SetItemOverrideCursor(QGraphicsItem *item, const QString &pixmapPath, int hotX, int hotY)
{
#ifndef QT_NO_CURSOR
    SCASSERT(item != nullptr)
    item->setCursor(QCursor(QPixmapFromCache(pixmapPath), hotX, hotY));
#else
    Q_UNUSED(item)
    Q_UNUSED(pixmapPath)
    Q_UNUSED(hotX)
    Q_UNUSED(hotY)
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto SupportedLocales() -> QStringList
{
    return QStringList{"uk_UA",
                       "de_DE",
                       "cs_CZ",
                       "he_IL",
                       "fr_FR",
                       "it_IT",
                       "nl_NL",
                       "id_ID",
                       "es_ES",
                       "fi_FI",
                       "ro_RO",
                       "zh_CN",
                       "pt_BR",
                       "el_GR",
                       "pl_PL",
                       "hr_HR",
                       "hu_HU",
                       "pt_PT",
                       "ja_JP"};
}

//---------------------------------------------------------------------------------------------------------------------
auto RelativeMPath(const QString &patternPath, const QString &absoluteMPath) -> QString
{
    if (patternPath.isEmpty() || absoluteMPath.isEmpty())
    {
        return absoluteMPath;
    }

    if (QFileInfo(absoluteMPath).isRelative())
    {
        return absoluteMPath;
    }

    return QFileInfo(patternPath).absoluteDir().relativeFilePath(absoluteMPath);
}

//---------------------------------------------------------------------------------------------------------------------
auto AbsoluteMPath(const QString &patternPath, const QString &relativeMPath) -> QString
{
    if (patternPath.isEmpty() || relativeMPath.isEmpty())
    {
        return relativeMPath;
    }

    if (QFileInfo(relativeMPath).isAbsolute())
    {
        return relativeMPath;
    }

    return QFileInfo(QFileInfo(patternPath).absoluteDir(), relativeMPath).absoluteFilePath();
}

//---------------------------------------------------------------------------------------------------------------------
auto GetMinPrinterFields(const QSharedPointer<QPrinter> &printer) -> QMarginsF
{
    QPageLayout layout = printer->pageLayout();
    layout.setUnits(QPageLayout::Millimeter);
    const QMarginsF minMargins = layout.minimumMargins();

    QMarginsF min;
    min.setLeft(UnitConvertor(minMargins.left(), Unit::Mm, Unit::Px));
    min.setRight(UnitConvertor(minMargins.right(), Unit::Mm, Unit::Px));
    min.setTop(UnitConvertor(minMargins.top(), Unit::Mm, Unit::Px));
    min.setBottom(UnitConvertor(minMargins.bottom(), Unit::Mm, Unit::Px));
    return min;
}

//---------------------------------------------------------------------------------------------------------------------
auto GetPrinterFields(const QSharedPointer<QPrinter> &printer) -> QMarginsF
{
    if (printer.isNull())
    {
        return {};
    }

    // We can't use Unit::Px because our dpi in most cases is different
    const QMarginsF m = printer->pageLayout().margins(QPageLayout::Millimeter);

    QMarginsF def;
    def.setLeft(UnitConvertor(m.left(), Unit::Mm, Unit::Px));
    def.setRight(UnitConvertor(m.right(), Unit::Mm, Unit::Px));
    def.setTop(UnitConvertor(m.top(), Unit::Mm, Unit::Px));
    def.setBottom(UnitConvertor(m.bottom(), Unit::Mm, Unit::Px));
    return def;
}

//---------------------------------------------------------------------------------------------------------------------
auto darkenPixmap(const QPixmap &pixmap) -> QPixmap
{
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    const int imgh = img.height();
    const int imgw = img.width();
    for (int y = 0; y < imgh; ++y)
    {
        for (int x = 0; x < imgw; ++x)
        {
            int h;
            int s;
            int v;
            QRgb pixel = img.pixel(x, y);
            const int a = qAlpha(pixel);
            QColor hsvColor(pixel);
            hsvColor.getHsv(&h, &s, &v);
            s = qMin(100, s * 2);
            v = v / 2;
            hsvColor.setHsv(h, s, v);
            pixel = hsvColor.rgb();
            img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), a));
        }
    }
    return QPixmap::fromImage(img);
}

//---------------------------------------------------------------------------------------------------------------------
void ShowInGraphicalShell(const QString &filePath)
{
#ifdef Q_OS_MAC
    QStringList args{"-e", "tell application \"Finder\"",
                     "-e", "activate",
                     "-e", "select POSIX file \"" + filePath + "\"",
                     "-e", "end tell"};
    QProcess::startDetached(QStringLiteral("osascript"), args);
#elif defined(Q_OS_WIN)
    QProcess::startDetached(QStringLiteral("explorer"), QStringList{"/select", QDir::toNativeSeparators(filePath)});
#else
    const int timeout = 1000; // ms
    auto const command = QStringLiteral("dbus-send --reply-timeout=%1 --session --dest=org.freedesktop.FileManager1 "
                                        "--type=method_call /org/freedesktop/FileManager1 "
                                        "org.freedesktop.FileManager1.ShowItems array:string:\"%2\" string:\"\"")
                             .arg(timeout)
                             .arg(QUrl::fromLocalFile(filePath).toString());

    // Sending message through dbus will highlighting file
    QProcess dbus;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // This approach works too.
    dbus.start(command, QStringList()); // clazy:exclude=qt6-deprecated-api-fixes
#else
    dbus.startCommand(command);
#endif
    if (not dbus.waitForStarted(timeout))
    {
        // This way will open only window without highlighting file
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).path()));
        return;
    }

    if (not dbus.waitForFinished(timeout))
    {
        // This way will open only window without highlighting file
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).path()));
        return;
    }
#endif
}

//---------------------------------------------------------------------------------------------------------------------
#ifdef Q_OS_MAC
#if MACOS_LAYER_BACKING_AFFECTED
void MacosEnableLayerBacking()
{
    QOperatingSystemVersion osVer = QOperatingSystemVersion::current();
    const int majorVer = osVer.majorVersion();
    const int minorVer = osVer.minorVersion();
    if (((majorVer == 10 && minorVer >= 16) || majorVer >= 11) && qEnvironmentVariableIsEmpty("QT_MAC_WANTS_LAYER"))
    {
        qputenv("QT_MAC_WANTS_LAYER", "1");
    }
}
#endif // MACOS_LAYER_BACKING_AFFECTED
#endif // Q_OS_MAC

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, strTMark, ("tMark"_L1))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strVMark, ("vMark"_L1))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strVMark2, ("vMark2"_L1))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strUMark, ("uMark"_L1))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strBoxMark, ("boxMark"_L1))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCheckMark, ("checkMark"_L1)) // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
auto PassmarkLineTypeToString(PassmarkLineType type) -> QString
{
    switch (type)
    {
        case PassmarkLineType::OneLine:
            return strOne;
        case PassmarkLineType::TwoLines:
            return strTwo;
        case PassmarkLineType::ThreeLines:
            return strThree;
        case PassmarkLineType::TMark:
            return *strTMark;
        case PassmarkLineType::ExternalVMark:
            return *strVMark;
        case PassmarkLineType::InternalVMark:
            return *strVMark2;
        case PassmarkLineType::UMark:
            return *strUMark;
        case PassmarkLineType::BoxMark:
            return *strBoxMark;
        case PassmarkLineType::CheckMark:
            return *strCheckMark;
        default:
            break;
    }

    return strOne;
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToPassmarkLineType(const QString &value) -> PassmarkLineType
{
    const QStringList values{strOne,     strTwo,    strThree,    *strTMark,    *strVMark,
                             *strVMark2, *strUMark, *strBoxMark, *strCheckMark};

    switch (values.indexOf(value))
    {
        case 0: // strOne
            return PassmarkLineType::OneLine;
        case 1: // strTwo
            return PassmarkLineType::TwoLines;
        case 2: // strThree
            return PassmarkLineType::ThreeLines;
        case 3: // strTMark
            return PassmarkLineType::TMark;
        case 4: // strVMark
            return PassmarkLineType::ExternalVMark;
        case 5: // strVMark2
            return PassmarkLineType::InternalVMark;
        case 6: // strUMark
            return PassmarkLineType::UMark;
        case 7: // strBoxMark
            return PassmarkLineType::BoxMark;
        case 8: // strCheckMark
            return PassmarkLineType::CheckMark;
        default:
            break;
    }
    return PassmarkLineType::OneLine;
}

//---------------------------------------------------------------------------------------------------------------------
auto PassmarkAngleTypeToString(PassmarkAngleType type) -> QString
{
    switch (type)
    {
        case PassmarkAngleType::Straightforward:
            return strStraightforward;
        case PassmarkAngleType::Bisector:
            return strBisector;
        case PassmarkAngleType::Intersection:
            return strIntersection;
        case PassmarkAngleType::IntersectionOnlyLeft:
            return strIntersectionOnlyLeft;
        case PassmarkAngleType::IntersectionOnlyRight:
            return strIntersectionOnlyRight;
        case PassmarkAngleType::Intersection2:
            return strIntersection2;
        case PassmarkAngleType::Intersection2OnlyLeft:
            return strIntersection2OnlyLeft;
        case PassmarkAngleType::Intersection2OnlyRight:
            return strIntersection2OnlyRight;
        default:
            return strStraightforward;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToPassmarkAngleType(const QString &value) -> PassmarkAngleType
{
    const QStringList values{
        strStraightforward,       strBisector,      strIntersection,          strIntersectionOnlyLeft,
        strIntersectionOnlyRight, strIntersection2, strIntersection2OnlyLeft, strIntersection2OnlyRight};

    switch (values.indexOf(value))
    {
        case 0:
            return PassmarkAngleType::Straightforward;
        case 1:
            return PassmarkAngleType::Bisector;
        case 2:
            return PassmarkAngleType::Intersection;
        case 3:
            return PassmarkAngleType::IntersectionOnlyLeft;
        case 4:
            return PassmarkAngleType::IntersectionOnlyRight;
        case 5:
            return PassmarkAngleType::Intersection2;
        case 6:
            return PassmarkAngleType::Intersection2OnlyLeft;
        case 7:
            return PassmarkAngleType::Intersection2OnlyRight;
        default:
            return PassmarkAngleType::Straightforward;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto FoldLineTypeToString(FoldLineType type) -> QString
{
    switch (type)
    {
        case FoldLineType::TwoArrowsTextUnder:
            return strFoldTypeTwoArrowsTextUnder;
        case FoldLineType::TwoArrows:
            return strFoldTypeTwoArrows;
        case FoldLineType::Text:
            return strFoldTypeText;
        case FoldLineType::ThreeDots:
            return strFoldTypeThreeDots;
        case FoldLineType::ThreeX:
            return strFoldTypeThreeX;
        case FoldLineType::None:
            return strFoldTypeNone;
        case FoldLineType::TwoArrowsTextAbove:
        default:
            return strFoldTypeTwoArrowsTextAbove;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToFoldLineType(const QString &value) -> FoldLineType
{
    const QStringList values{strFoldTypeTwoArrowsTextAbove, // 0
                             strFoldTypeTwoArrowsTextUnder, // 1
                             strFoldTypeTwoArrows,          // 2
                             strFoldTypeText,               // 3
                             strFoldTypeThreeDots,          // 4
                             strFoldTypeThreeX,             // 5
                             strFoldTypeNone};              // 6

    switch (values.indexOf(value))
    {
        case 1:
            return FoldLineType::TwoArrowsTextUnder;
        case 2:
            return FoldLineType::TwoArrows;
        case 3:
            return FoldLineType::Text;
        case 4:
            return FoldLineType::ThreeDots;
        case 5:
            return FoldLineType::ThreeX;
        case 6:
            return FoldLineType::None;
        case 0:
        default:
            return FoldLineType::TwoArrowsTextAbove;
    };
}

//---------------------------------------------------------------------------------------------------------------------
auto StrToUnits(const QString &unit) -> Unit
{
    const QStringList units{unitMM, unitCM, unitINCH, unitPX};
    switch (units.indexOf(unit))
    {
        case 0: // mm
            return Unit::Mm;
        case 2: // inch
            return Unit::Inch;
        case 3: // px
            return Unit::Px;
        case 1: // cm
        default:
            return Unit::Cm;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UnitsToStr translate unit to string.
 *
 * This method used when need write unit in xml file and for showing unit in dialogs.
 * @param unit curent unit
 * @param translate true if need show translated name. Default value false.
 * @return string reprezantation for unit.
 */
auto UnitsToStr(const Unit &unit, const bool translate) -> QString
{
    switch (unit)
    {
        case Unit::Mm:
            return translate ? QObject::tr("mm") : unitMM;
        case Unit::Inch:
            return translate ? QObject::tr("inch") : unitINCH;
        case Unit::Px:
            return translate ? QObject::tr("px") : unitPX;
        case Unit::LAST_UNIT_DO_NOT_USE:
            return {};
        case Unit::Cm:
        default:
            return translate ? QObject::tr("cm") : unitCM;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InitLanguages(QComboBox *combobox)
{
    InitLanguageList(combobox);

    // set default translators and language checked
    qint32 const index = combobox->findData(VAbstractApplication::VApp()->Settings()->GetLocale());
    if (index != -1)
    {
        combobox->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InitPieceLabelLanguages(QComboBox *combobox)
{
    InitLanguageList(combobox);

    combobox->addItem(QApplication::translate("InitPieceLabelLanguages", "Default"),
                      VCommonSettings::defaultPieceLabelLocale);

    qint32 const index = combobox->findData(VAbstractApplication::VApp()->Settings()->GetPieceLabelLocale());
    if (index != -1)
    {
        combobox->setCurrentIndex(index);
    }
}

const quint32 CustomSARecord::streamHeader = 0xEBFF7586; // CRC-32Q string "CustomSARecord"
const quint16 CustomSARecord::classVersion = 1;

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &out, const CustomSARecord &record) -> QDataStream &
{
    out << CustomSARecord::streamHeader << CustomSARecord::classVersion;

    // Added in classVersion = 1
    out << record.startPoint;
    out << record.path;
    out << record.endPoint;
    out << record.reverse;
    out << record.includeType;

    // Added in classVersion = 2

    return out;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &in, CustomSARecord &record) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    in >> actualStreamHeader;

    if (actualStreamHeader != CustomSARecord::streamHeader)
    {
        QString const message = QCoreApplication::tr("CustomSARecord prefix mismatch error: actualStreamHeader = 0x%1 "
                                                     "and streamHeader = 0x%2")
                                    .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                                    .arg(CustomSARecord::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    in >> actualClassVersion;

    if (actualClassVersion > CustomSARecord::classVersion)
    {
        QString const message = QCoreApplication::tr("CustomSARecord compatibility error: actualClassVersion = %1 and "
                                                     "classVersion = %2")
                                    .arg(actualClassVersion)
                                    .arg(CustomSARecord::classVersion);
        throw VException(message);
    }

    in >> record.startPoint;
    in >> record.path;
    in >> record.endPoint;
    in >> record.reverse;
    in >> record.includeType;

    //    if (actualClassVersion >= 2)
    //    {

    //    }

    return in;
}

//---------------------------------------------------------------------------------------------------------------------
auto IncrementTypeToString(IncrementType type) -> QString
{
    switch (type)
    {
        case IncrementType::Increment:
            return strTypeIncrement;
        case IncrementType::Separator:
            return strTypeSeparator;
        default:
            break;
    }

    return strTypeIncrement;
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToIncrementType(const QString &value) -> IncrementType
{
    const QStringList values{strTypeIncrement, strTypeSeparator};

    switch (values.indexOf(value))
    {
        case 0:
            return IncrementType::Increment;
        case 1:
            return IncrementType::Separator;
        default:
            break;
    }
    return IncrementType::Increment;
}

//---------------------------------------------------------------------------------------------------------------------
auto MeasurementTypeToString(MeasurementType type) -> QString
{
    switch (type)
    {
        case MeasurementType::Measurement:
            return strTypeMeasurement;
        case MeasurementType::Separator:
            return strTypeSeparator;
        default:
            break;
    }

    return strTypeIncrement;
}

//---------------------------------------------------------------------------------------------------------------------
auto StringToMeasurementType(const QString &value) -> MeasurementType
{
    const QStringList values{strTypeMeasurement, strTypeSeparator};

    switch (values.indexOf(value))
    {
        case 0:
            return MeasurementType::Measurement;
        case 1:
            return MeasurementType::Separator;
        default:
            break;
    }
    return MeasurementType::Measurement;
}

//---------------------------------------------------------------------------------------------------------------------
auto SplitFilePaths(const QString &path) -> QStringList
{
    QStringList result;
    QString subPath = QDir::cleanPath(path);
    QString lastFileName;

    do
    {
        QFileInfo const fileInfo(subPath);
        lastFileName = fileInfo.fileName();
        if (not lastFileName.isEmpty())
        {
            result.prepend(lastFileName);
            subPath = fileInfo.path();
        }
    } while (not lastFileName.isEmpty());

    return result;
}
