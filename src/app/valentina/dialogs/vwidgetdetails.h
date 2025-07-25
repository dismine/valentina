/************************************************************************
 **
 **  @file   vwidgetdetails.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 6, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VWIDGETDETAILS_H
#define VWIDGETDETAILS_H

#include <QWidget>

class VAbstractPattern;
class VContainer;
class VPiece;
class QTableWidgetItem;

namespace Ui
{
class VWidgetDetails;
}

class VWidgetDetails : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit VWidgetDetails(VContainer *data, VAbstractPattern *doc, QWidget *parent = nullptr);
    ~VWidgetDetails() override;

signals:
    void Highlight(quint32 id);

public slots:
    void UpdateList();
    void SelectDetail(quint32 id);
    void ToggledPiece(quint32 id);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void InLayoutStateChanged(int row, int column);
    void RenameDetail(int row, int column);
    void ShowContextMenu(const QPoint &pos);

    void on_checkBoxHideNotInLayout_stateChanged();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VWidgetDetails) // NOLINT
    Ui::VWidgetDetails *ui;
    VAbstractPattern *m_doc;
    VContainer *m_data;
    QTimer *m_updateListTimer;

    void FillTable(const QHash<quint32, VPiece> *details);
    void ToggleSectionDetails(bool select);
    void ToggledPieceItem(QTableWidgetItem *item);

    Q_REQUIRED_RESULT static auto PrepareInLayoutColumnCell(const VPiece &det, quint32 id) -> QTableWidgetItem *;
    Q_REQUIRED_RESULT static auto PreparePieceNameColumnCell(const VPiece &det) -> QTableWidgetItem *;
};

#endif // VWIDGETDETAILS_H
