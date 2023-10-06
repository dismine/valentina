/************************************************************************
 **
 **  @file   tmainwindow.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 7, 2015
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

#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

#include <QTableWidget>

#include "../vformat/vmeasurements.h"
#include "../vmisc/def.h"
#include "../vmisc/vlockguard.h"
#include "../vmisc/vtablesearch.h"
#include "../vwidgets/vabstractmainwindow.h"

namespace Ui
{
class TMainWindow;
} // namespace Ui

class QLabel;
class QxtCsvModel;
class VMeasurement;

class TMainWindow : public VAbstractMainWindow
{
    Q_OBJECT // NOLINT

public:
    explicit TMainWindow(QWidget *parent = nullptr);
    ~TMainWindow() override;

    auto CurrentFile() const -> QString;

    void RetranslateTable();

    void SetDimensionABase(qreal base);
    void SetDimensionBBase(qreal base);
    void SetDimensionCBase(qreal base);
    void SetPUnit(Unit unit);

    auto LoadFile(const QString &path) -> bool;

    void UpdateWindowTitle();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto eventFilter(QObject *object, QEvent *event) -> bool override;
    void ExportToCSVData(const QString &fileName, bool withHeader, int mib, const QChar &separator) final;
    auto RecentFileList() const -> QStringList override;

private slots:
    void FileNew();
    void OpenIndividual();
    void OpenMultisize();
    void OpenTemplate();
    void CreateFromExisting();
    void Preferences();
    void ToolBarStyles();

    bool FileSave();   // NOLINT(modernize-use-trailing-return-type)
    bool FileSaveAs(); // NOLINT(modernize-use-trailing-return-type)
    void AboutToShowWindowMenu();
    void ShowWindow() const;
    void ImportDataFromCSV();

#if defined(Q_OS_MAC)
    void AboutToShowDockMenu();
    void OpenAt(QAction *where);
#endif // defined(Q_OS_MAC)

    void SaveCustomerName();
    void SaveEmail();
    void SaveGender(int index);
    void SaveBirthDate(const QDate &date);
    void SaveNotes();
    void SavePMSystem(int index);

    void Remove();
    void MoveTop();
    void MoveUp();
    void MoveDown();
    void MoveBottom();
    void Fx();

    void AddCustom();
    void AddKnown();
    void AddSeparator();
    void ImportFromPattern();

    void DimensionABaseChanged();
    void DimensionBBaseChanged();
    void DimensionCBaseChanged();

    void GradationChanged();

    void ShowMData();

    void DeployFormula();

    void SaveMName(const QString &text);
    void SaveMValue();
    void SaveMBaseValue(double value);
    void SaveMShiftA(double value);
    void SaveMShiftB(double value);
    void SaveMShiftC(double value);
    void SaveMCorrectionValue(double value);
    void SaveMDescription();
    void SaveMFullName();
    void SaveMUnits();
    void SaveMDimension();

    void FullCircumferenceChanged(bool checked);

    void ExportToIndividual();

    void RestrictFirstDimesion();
    void RestrictSecondDimesion();
    void RestrictThirdDimesion();

    void EditDimensionLabels();
    void DimensionCustomNames();

    void AskDefaultSettings();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(TMainWindow) // NOLINT
    Ui::TMainWindow *ui;
    VMeasurements *m_m{nullptr};
    VContainer *m_data{nullptr};
    Unit m_mUnit{Unit::Cm};
    Unit m_pUnit{Unit::Cm};
    MeasurementsType m_mType{MeasurementsType::Individual};
    qreal m_currentDimensionA{0};
    qreal m_currentDimensionB{0};
    qreal m_currentDimensionC{0};
    QString m_curFile{};
    QComboBox *m_gradationDimensionA{nullptr};
    QComboBox *m_gradationDimensionB{nullptr};
    QComboBox *m_gradationDimensionC{nullptr};
    QComboBox *m_comboBoxUnits{nullptr};
    int m_formulaBaseHeight;
    QSharedPointer<VLockGuard<char>> m_lock{nullptr};
    QSharedPointer<VTableSearch> m_search{};
    QLabel *m_labelGradationDimensionA{nullptr};
    QLabel *m_labelGradationDimensionB{nullptr};
    QLabel *m_labelGradationDimensionC{nullptr};
    QLabel *m_labelPatternUnit{nullptr};
    bool m_isInitialized{false};
    bool m_mIsReadOnly{false};
    QTimer *m_gradation;
    QMenu *m_searchHistory;

    QVector<QObject *> m_hackedWidgets{};

    struct MultisizeMeasurement
    {
        MultisizeMeasurement() = default;

        QString name{};        // NOLINT(misc-non-private-member-variables-in-classes)
        qreal base{0};         // NOLINT(misc-non-private-member-variables-in-classes)
        qreal shiftA{0};       // NOLINT(misc-non-private-member-variables-in-classes)
        qreal shiftB{0};       // NOLINT(misc-non-private-member-variables-in-classes)
        qreal shiftC{0};       // NOLINT(misc-non-private-member-variables-in-classes)
        QString fullName{};    // NOLINT(misc-non-private-member-variables-in-classes)
        QString description{}; // NOLINT(misc-non-private-member-variables-in-classes)
    };

    void SetupMenu();
    void InitWindow();
    void InitMenu();
    void InitDimensionsBaseValue();
    void InitDimensionGradation(int index, const MeasurementDimension_p &dimension, QComboBox *control);
    void InitDimensionControls();
    void InitDimesionShifts();
    void InitTable();
    void SetDecimals();
    void InitPatternUnits();
    void InitComboBoxUnits();
    void InitMeasurementUnits();
    static void InitGender(QComboBox *gender);
    void InitMeasurementDimension();
    void InitSearch();

    void InitSearchHistory();
    void SaveSearchRequest();
    void UpdateSearchControlsTooltips();

    void RetranslateTableHeaders();

    void ShowNewMData(bool fresh);
    void ShowUnits();
    static void ShowHeaderUnits(QTableWidget *table, int column, const QString &unit);

    void MeasurementsWereSaved(bool saved);
    void SetCurrentFile(const QString &fileName);
    auto SaveMeasurements(const QString &fileName, QString &error) -> bool;

    auto MaybeSave() -> bool;

    auto AddCell(const QString &text, int row, int column, int aligment, bool ok = true) -> QTableWidgetItem *;
    auto AddSeparatorCell(const QString &text, int row, int column, int aligment, bool ok = true) -> QTableWidgetItem *;

    void RefreshData(bool freshCall = false);
    void RefreshTable(bool freshCall = false);
    void RefreshMeasurementData(const QSharedPointer<VMeasurement> &meash, qint32 currentRow);

    auto GetCustomName() const -> QString;
    static auto ClearCustomName(const QString &name) -> QString;

    auto EvalFormula(const QString &formula, bool fromUser, VContainer *data, QLabel *label, bool specialUnits) -> bool;
    void ShowMDiagram(const QString &name);

    auto Open(const QString &pathTo, const QString &filter) -> QString;
    void UpdatePadlock(bool ro);
    void MeasurementGUI();
    void Controls();
    void MFields(bool enabled);

    void ReadSettings();
    void WriteSettings();

    static auto FilterMeasurements(const QStringList &mNew, const QStringList &mFilter) -> QStringList;

    void UpdatePatternUnit();

    auto LoadFromExistingFile(const QString &path) -> bool;

    void CreateWindowMenu(QMenu *menu);

    template <class T> void HackWidget(T **widget);
    void HackDimensionBaseValue();
    void HackDimensionShifts();

    auto CheckMName(const QString &name, const QSet<QString> &importedNames) const -> QString;
    void ShowError(const QString &text);
    void RefreshDataAfterImport();

    void ImportIndividualMeasurements(const QxtCsvModel &csv, const QVector<int> &map, bool withHeader);
    void ImportMultisizeMeasurements(const QxtCsvModel &csv, const QVector<int> &map, bool withHeader);
    auto ImportMultisizeMeasurement(const QxtCsvModel &csv, int i, const QVector<int> &map, vsizetype dimensionsCount,
                                    QSet<QString> &importedNames) -> MultisizeMeasurement;

    void SetCurrentPatternUnit();

    void ShowDimensionControls();
    void SetDimensionBases();
    void SetCurrentDimensionValues();

    auto DimensionRestrictedValues(int index, const MeasurementDimension_p &dimension) -> QVector<double>;

    auto OrderedMeasurments() const -> QMap<int, QSharedPointer<VMeasurement>>;

    void InitIcons();
};

#endif // TMAINWINDOW_H
