/************************************************************************
 **
 **  @file   dialogfinalmeasurements.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 9, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef DIALOGFINALMEASUREMENTS_H
#define DIALOGFINALMEASUREMENTS_H

#include <QDialog>

#include "../vmisc/vtablesearch.h"
#include "../vpatterndb/vcontainer.h"
#include "../xml/vpattern.h"

namespace Ui
{
class DialogFinalMeasurements;
}

class DialogFinalMeasurements : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogFinalMeasurements(VPattern *doc, QWidget *parent = nullptr);
    ~DialogFinalMeasurements() override;

    auto FinalMeasurements() const -> QVector<VFinalMeasurement>;

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    auto eventFilter(QObject *object, QEvent *event) -> bool override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void ShowFinalMeasurementDetails();
    void Add();
    void Remove();
    void MoveUp();
    void MoveDown();
    void SaveName(const QString &text);
    void SaveDescription();
    void SaveFormula();
    void DeployFormula();
    void Fx();
    void FullUpdateFromFile();

private:
    Q_DISABLE_COPY_MOVE(DialogFinalMeasurements) // NOLINT
    Ui::DialogFinalMeasurements *ui;
    /** @brief doc dom document container */
    VPattern *m_doc;
    VContainer m_data;
    QVector<VFinalMeasurement> m_measurements;
    QSharedPointer<VTableSearch> m_search{};
    int formulaBaseHeight{0};
    bool m_isInitialized{false};

    QMenu *m_searchHistory;

    void FillFinalMeasurements(bool freshCall = false);

    void ShowUnits();

    void AddCell(const QString &text, int row, int column, int aligment, bool ok = true);
    auto EvalUserFormula(const QString &formula, bool fromUser) -> bool;
    void Controls();
    void EnableDetails(bool enabled);

    void UpdateTree();

    auto EvalFormula(const QString &formula, bool &ok) -> qreal;

    void InitSearch();
    void InitSearchHistory();
    void SaveSearchRequest();
    void UpdateSearchControlsTooltips();

    void InitIcons();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogFinalMeasurements::FinalMeasurements() const -> QVector<VFinalMeasurement>
{
    return m_measurements;
}

#endif // DIALOGFINALMEASUREMENTS_H
