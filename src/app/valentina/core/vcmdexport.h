/************************************************************************
 **
 **  @file   vcmdexport.h
 **  @author Alex Zaharov <alexzkhr@gmail.com>
 **  @date   25 8, 2015
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

#ifndef VCMDEXPORT_H
#define VCMDEXPORT_H

#include <memory>
#include <vector>
#include <QTextStream>
#include <QCoreApplication>
#include <QCommandLineParser>

#include "../dialogs/dialoglayoutsettings.h"

class VCommandLine;
using VCommandLinePtr = std::shared_ptr<VCommandLine>;
using VLayoutGeneratorPtr = std::shared_ptr<VLayoutGenerator>;
enum class PageOrientation : bool;

//@brief: class used to install export command line options and parse their values
//QCommandLineParser* object must exists until this object alive
class VCommandLine
{
public:
    virtual ~VCommandLine() = default;

    //@brief creates object and applies export related options to parser

    //@brief tests if user enabled test mode from cmd, throws exception if not exactly 1 input VAL file supplied in
    //case test mode enabled
    auto IsTestModeEnabled() const -> bool;

    //@brief Make all parsing warnings into errors. Have effect only in console mode. Use to force Valentina to
    //immediately terminate if a pattern contains a parsing warning.
    auto IsPedantic() const -> bool;

    auto IsNoScalingEnabled() const -> bool;

    //@brief tests if user enabled export from cmd, throws exception if not exactly 1 input VAL file supplied in case
    //export enabled
    auto IsExportEnabled() const -> bool;

    //@brief tests if user enabled export final measurements from cmd, throws exception if not exactly 1 input VAL
    //file supplied in case export enabled
    auto IsExportFMEnabled() const -> bool;

    //@brief returns path to custom measure file or empty string
    auto OptMeasurePath() const -> QString;

    //@brief returns the base name of layout files or empty string if not set
    auto OptBaseName() const -> QString;

    //@brief returns the absolute path to output destination directory or path to application's current directory if
    //not set
    auto OptDestinationPath() const -> QString;

    //@brief returns export type set, defaults 0 - svg
    auto OptExportType() const -> int;

    auto IsBinaryDXF() const -> bool;
    auto IsTextAsPaths() const -> bool;
    auto IsExportOnlyDetails() const -> bool;
    auto IsCSVWithHeader() const -> bool;

    auto ExportXScale() const -> qreal;
    auto ExportYScale() const -> qreal;

    //@brief returns the piece name regex or empty string if not set
    auto OptExportSuchDetails() const -> QString;

    //@brief returns user selected csv codec or empty string if not set
    auto OptCSVCodecName() const -> QString;

    //@brief returns user selected csv separator or empty string if not set
    auto OptCSVSeparator() const -> QChar;

    //@brief returns the destination path for export final measurements or empty string if not set
    auto OptExportFMTo() const -> QString;

    //@brief returns list of user defined materials
    auto OptUserMaterials() const -> QMap<int, QString>;

    //generator creation is moved here ... because most options are for it only, so no need to create extra getters...
    //@brief creates VLayoutGenerator
    auto DefaultGenerator() const -> VLayoutGeneratorPtr;

    //@brief gets filenames which should be loaded
    auto OptInputFileNames() const -> QStringList;

    auto IsGuiEnabled()const -> bool;

    auto IsSetDimensionA() const -> bool;
    auto IsSetDimensionB() const -> bool;
    auto IsSetDimensionC() const -> bool;

    auto OptDimensionA() const -> int;
    auto OptDimensionB() const -> int;
    auto OptDimensionC() const -> int;
    
    auto TiledPageMargins() const -> QMarginsF;
    auto OptTiledPaperSize() const -> VAbstractLayoutDialog::PaperSizeTemplate;
    auto OptTiledPageOrientation() const -> PageOrientation;

protected:

    VCommandLine();

    //@brief returns VAbstractLayoutDialog::PaperSizeTemplate
    auto OptPaperSize() const -> VAbstractLayoutDialog::PaperSizeTemplate;

    auto OptGroup() const -> Cases;

    //@brief: called in destructor of application, so instance destroyed and new maybe created (never happen scenario though)
    static void Reset();

    //@brief called to create single object, by VApplication only
    static auto Get(const QCoreApplication& app) -> VCommandLinePtr;

private:
    Q_DISABLE_COPY_MOVE(VCommandLine) // NOLINT
    static VCommandLinePtr instance; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    QCommandLineParser parser{};
    bool isGuiEnabled{false};
    friend class VApplication;
    
    auto FormatSize(const QString &key) const -> VAbstractLayoutDialog::PaperSizeTemplate;

    void InitCommandLineOptions();
    auto IsOptionSet(const QString &option) const -> bool;
    auto OptionValue(const QString &option) const -> QString;
    auto OptionValues(const QString &option) const -> QStringList;

    auto OptNestingTime() const -> int;
    auto OptEfficiencyCoefficient() const -> qreal;

    void TestPageformat() const;
    void TestGapWidth() const;
    void TestMargins() const;

    auto ParseMargins(const DialogLayoutSettings &diag) const -> QMarginsF;
};

#endif // VCMDEXPORT_H
