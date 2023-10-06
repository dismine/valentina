/************************************************************************
 **
 **  @file   vformulapropertyeditor.cpp
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

#include "vformulapropertyeditor.h"

#include <QApplication>
#include <QColorDialog>
#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QSpacerItem>

#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vpropertyexplorer/vproperty.h"
#include "../vtools/dialogs/support/dialogeditwrongformula.h"

// VFormulaPropertyEditor
//---------------------------------------------------------------------------------------------------------------------
VFormulaPropertyEditor::VFormulaPropertyEditor(QWidget *parent)
  : QWidget(parent)
{
    setAutoFillBackground(true);

    // Create the tool button
    m_ToolButton = new QToolButton(this);
    m_ToolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_ToolButton->setText(QStringLiteral("..."));
    m_ToolButton->setIcon(VTheme::GetIconResource(QStringLiteral("icon"), QStringLiteral("16x16/fx.png")));
    m_ToolButton->setIconSize(QSize(16, 16));
    m_ToolButton->setFixedSize(24, 24);
    m_ToolButton->installEventFilter(this);
    setFocusProxy(m_ToolButton); // Make the ToolButton the focus proxy
    setFocusPolicy(m_ToolButton->focusPolicy());
    connect(m_ToolButton, &QToolButton::clicked, this, &VFormulaPropertyEditor::onToolButtonClicked);

    // Create the text label
    m_TextLabel = new QLabel(this);
    m_TextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_TextLabel->setText(m_formula.getStringValue());

    // The layout (a horizontal layout)
    auto *layout = new QHBoxLayout(this);
    layout->setSpacing(3);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_TextLabel);
    // Spacer (this is needed for proper display of the label and button)
    layout->addSpacerItem(new QSpacerItem(1000000000, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
    layout->addWidget(m_ToolButton);
}

//---------------------------------------------------------------------------------------------------------------------
void VFormulaPropertyEditor::SetFormula(const VFormula &formula)
{
    if (this->m_formula != formula)
    {
        this->m_formula = formula;
        m_TextLabel->setText(this->m_formula.getStringValue());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VFormulaPropertyEditor::onToolButtonClicked()
{
    QScopedPointer<DialogEditWrongFormula> tmpWidget(new DialogEditWrongFormula(
        m_formula.getData(), m_formula.getToolId(), VAbstractValApplication::VApp()->getMainWindow()));
    tmpWidget->setCheckZero(m_formula.getCheckZero());
    tmpWidget->setPostfix(m_formula.getPostfix());
    tmpWidget->SetFormula(m_formula.GetFormula(FormulaType::FromUser));

    if (tmpWidget->exec() == QDialog::Accepted)
    {
        m_formula.SetFormula(tmpWidget->GetFormula(), FormulaType::ToUser);
        m_formula.Eval();
        m_TextLabel->setText(m_formula.getStringValue());
        emit dataChangedByUser(m_formula, this);
        QCoreApplication::postEvent(this, new VPE::UserChangeEvent());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormulaPropertyEditor::eventFilter(QObject *obj, QEvent *ev) -> bool
{
    if (obj == m_ToolButton && ev->type() == QEvent::KeyPress)
    {
        // Ignore the event, so that eventually the delegate gets the event.
        ev->ignore();
        return true;
    }

    return QWidget::eventFilter(obj, ev);
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormulaPropertyEditor::GetFormula() const -> VFormula
{
    return m_formula;
}
