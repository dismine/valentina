/************************************************************************
 **
 **  @file   dialogflippingbyline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
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

#ifndef DIALOGFLIPPINGBYLINE_H
#define DIALOGFLIPPINGBYLINE_H

#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "dialogoperationtool.h"

namespace Ui
{
class DialogFlippingByLine;
}

class DialogFlippingByLine final : public DialogOperationTool
{
    Q_OBJECT // NOLINT

public:
    explicit DialogFlippingByLine(const VContainer *data, VAbstractPattern *doc, quint32 toolId,
                                  QWidget *parent = nullptr);
    ~DialogFlippingByLine() override;

    auto GetFirstLinePointId() const -> quint32;
    void SetFirstLinePointId(quint32 value);

    auto GetSecondLinePointId() const -> quint32;
    void SetSecondLinePointId(quint32 value);

    void ShowDialog(bool click) override;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;

protected:
    void ShowVisualization() override;

    /** @brief SaveData Put dialog data in local variables */
    void SaveData() override;
    auto IsValid() const -> bool override;

    void OnSourceObjectsSet() override;

    auto Widgets() const -> OperationWidgets override;

private slots:
    void PointChanged();

private:
    Q_DISABLE_COPY_MOVE(DialogFlippingByLine) // NOLINT

    Ui::DialogFlippingByLine *ui;

    /** @brief number number of handled objects */
    qint32 number{0};
};

#endif // DIALOGFLIPPINGBYLINE_H
