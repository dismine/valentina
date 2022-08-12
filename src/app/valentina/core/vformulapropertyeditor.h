/************************************************************************
 **
 **  @file   vformulapropertyeditor.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VFORMULAPROPERTYEDITOR_H
#define VFORMULAPROPERTYEDITOR_H

#include <QWidget>
#include <QToolButton>
#include <QLineEdit>
#include <QLabel>

#include "../vpatterndb/vformula.h"
#include "../vmisc/defglobal.h"

class VFormulaPropertyEditor : public QWidget
{
    Q_OBJECT // NOLINT

public:
    //! Constructor taking a widget as parent
    explicit VFormulaPropertyEditor(QWidget *parent);

    //! Destructor
    ~VFormulaPropertyEditor() override Q_DECL_EQ_DEFAULT;

    //! Returns the formula currently set
    auto GetFormula() const -> VFormula;

    //! Needed for proper event handling
    auto eventFilter(QObject *obj, QEvent *ev) -> bool override;

signals:
    //! This is emitted, when the user changes the color
    void dataChangedByUser(const VFormula &GetFormula, VFormulaPropertyEditor* editor);

    void dataChanged();

public slots:
    //! Sets the color of the widget
    void SetFormula(const VFormula &formula);

private slots:
    void onToolButtonClicked();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VFormulaPropertyEditor) // NOLINT
    VFormula m_formula{};
    QToolButton* m_ToolButton{nullptr};
    QLabel* m_TextLabel{nullptr};
};


#endif // VFORMULAPROPERTYEDITOR_H
