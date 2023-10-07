/************************************************************************
 **
 **  @file   dialogtruedarts.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 6, 2015
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

#ifndef DIALOGTRUEDARTS_H
#define DIALOGTRUEDARTS_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
class DialogTrueDarts;
}

class DialogTrueDarts : public DialogTool
{
    Q_OBJECT // NOLINT

public:
    DialogTrueDarts(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogTrueDarts() override;

    auto GetFirstNewDartPointName() -> QString;
    auto GetSecondNewDartPointName() -> QString;
    void SetNewDartPointNames(const QString &firstPoint, const QString &secondPoint);

    auto GetFirstBasePointId() const -> quint32;
    void SetFirstBasePointId(const quint32 &value);

    auto GetSecondBasePointId() const -> quint32;
    void SetSecondBasePointId(const quint32 &value);

    auto GetFirstDartPointId() const -> quint32;
    void SetFirstDartPointId(const quint32 &value);

    auto GetSecondDartPointId() const -> quint32;
    void SetSecondDartPointId(const quint32 &value);

    auto GetThirdDartPointId() const -> quint32;
    void SetThirdDartPointId(const quint32 &value);

    void SetChildrenId(const quint32 &ch1, const quint32 &ch2);

    void SetNotes(const QString &notes);
    auto GetNotes() const -> QString;

public slots:
    void ChosenObject(quint32 id, const SceneObject &type) override;
    void PointNameChanged() override;
    void NameDartPoint1Changed();
    void NameDartPoint2Changed();

protected:
    void ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    void SaveData() override;
    auto IsValid() const -> bool final;

private:
    Q_DISABLE_COPY_MOVE(DialogTrueDarts) // NOLINT
    Ui::DialogTrueDarts *ui;

    QString d1PointName;
    QString d2PointName;

    quint32 ch1;
    quint32 ch2;

    bool flagName1;
    bool flagName2;
    bool flagError;

    /** @brief number number of handled objects */
    qint32 number{0};

    void NameChanged(QLabel *labelEditNamePoint, const QString &pointD1Name, const QString &pointD2Name,
                     QLineEdit *secondPointName, bool &flagName);

    void FillComboBoxs(const quint32 &ch1, const quint32 &ch2);

    void CheckName(QLineEdit *edit, QLabel *labelEditNamePoint, const QString &pointD1Name, const QString &pointD2Name,
                   QLineEdit *secondPointName, bool &flagName);
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogTrueDarts::IsValid() const -> bool
{
    return flagName1 && flagName2 && flagError;
}

#endif // DIALOGTRUEDARTS_H
