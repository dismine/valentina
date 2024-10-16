/************************************************************************
 **
 **  @file   vformulaproperty.cpp
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

#include "vformulaproperty.h"
#include "vformulapropertyeditor.h"

#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vformula.h"
#include "../vpropertyexplorer/plugins/vstringproperty.h"
#include "../vpropertyexplorer/vproperty_p.h"
#include "vformulapropertyeditor.h"

//---------------------------------------------------------------------------------------------------------------------
VFormulaProperty::VFormulaProperty(const QString &name)
  : VProperty(name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
              static_cast<QMetaType::Type>(VFormula::FormulaTypeId()))
#else
              static_cast<QVariant::Type>(VFormula::FormulaTypeId()))
#endif
{
    vproperty_d_ptr->type = VPE::Property::Complex;

    auto *tmpFormula = new VPE::VStringProperty(tr("Formula"));
    addChild(tmpFormula);
    tmpFormula->setClearButtonEnable(true);
    tmpFormula->setUpdateBehaviour(true, false);
    tmpFormula->setOsSeparator(VAbstractApplication::VApp()->Settings()->GetOsSeparator());

    // Cannot use virtual function setValue in constructor
    SetFormula(QVariant(0).value<VFormula>());
}

//---------------------------------------------------------------------------------------------------------------------
//! Get the data how it should be displayed
auto VFormulaProperty::data(int column, int role) const -> QVariant
{
    if (column == DPC_Data && (Qt::DisplayRole == role || Qt::EditRole == role))
    {
        return getValue();
    }

    return VProperty::data(column, role);
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormulaProperty::flags(int column) const -> Qt::ItemFlags
{
    if (column == DPC_Name || column == DPC_Data)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return Qt::NoItemFlags;
}

//---------------------------------------------------------------------------------------------------------------------
//! Returns an editor widget, or NULL if it doesn't supply one
auto VFormulaProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                    const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    auto formula = VProperty::vproperty_d_ptr->VariantValue.value<VFormula>();
    auto *tmpEditor = new VFormulaPropertyEditor(parent);
    tmpEditor->setLocale(parent->locale());
    tmpEditor->SetFormula(formula);
    VProperty::vproperty_d_ptr->editor = tmpEditor;
    return VProperty::vproperty_d_ptr->editor;
}

//---------------------------------------------------------------------------------------------------------------------
//! Sets the property's data to the editor (returns false, if the standard delegate should do that)
auto VFormulaProperty::setEditorData(QWidget *editor) -> bool
{
    if (auto *tmpWidget = qobject_cast<VFormulaPropertyEditor *>(editor); tmpWidget != nullptr)
    {
        auto formula = VProperty::vproperty_d_ptr->VariantValue.value<VFormula>();
        tmpWidget->SetFormula(formula);
    }
    else
    {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
//! Gets the data from the widget
auto VFormulaProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    if (const auto *tmpWidget = qobject_cast<const VFormulaPropertyEditor *>(editor); tmpWidget != nullptr)
    {
        QVariant value;
        value.setValue(tmpWidget->GetFormula());
        return value;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormulaProperty::type() const -> QString
{
    return QStringLiteral("formula");
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormulaProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    if (!container)
    {
        container = new VFormulaProperty(getName());

        if (!include_children)
        {
            const QList<VProperty *> tmpChildren = container->getChildren();
            for (auto *tmpChild : tmpChildren)
            {
                container->removeChild(tmpChild);
                delete tmpChild;
            }
        }
    }

    return VProperty::clone(false, container); // Child
}

//---------------------------------------------------------------------------------------------------------------------
void VFormulaProperty::setValue(const QVariant &value)
{
    auto tmpFormula = value.value<VFormula>();
    SetFormula(tmpFormula);
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormulaProperty::getValue() const -> QVariant
{
    VFormula const tmpFormula = GetFormula();
    QVariant value;
    value.setValue(tmpFormula);
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VFormulaProperty::GetFormula() const -> VFormula
{
    return VProperty::vproperty_d_ptr->VariantValue.value<VFormula>();
}

//---------------------------------------------------------------------------------------------------------------------
void VFormulaProperty::SetFormula(const VFormula &formula)
{
    if (vproperty_d_ptr->Children.count() < 1)
    {
        return;
    }

    QVariant value;
    value.setValue(formula);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    value.convert(QMetaType(VFormula::FormulaTypeId()));
#else
    value.convert(VFormula::FormulaTypeId());
#endif
    VProperty::vproperty_d_ptr->VariantValue = value;

    QVariant tmpFormula(formula.GetFormula());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    tmpFormula.convert(QMetaType(QMetaType::QString));
#else
    tmpFormula.convert(QVariant::String);
#endif

    VProperty::vproperty_d_ptr->Children.at(0)->setValue(tmpFormula);

    if (VProperty::vproperty_d_ptr->editor != nullptr)
    {
        setEditorData(VProperty::vproperty_d_ptr->editor); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VFormulaProperty::ValueChildChanged(const QVariant &value, int typeForParent)
{
    Q_UNUSED(typeForParent)
    VFormula newFormula = GetFormula();
    newFormula.SetFormula(value.toString(), FormulaType::FromUser);
    newFormula.Eval();
    SetFormula(newFormula);
}
