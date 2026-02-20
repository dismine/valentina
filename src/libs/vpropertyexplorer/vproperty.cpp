/************************************************************************
 **
 **  @file   vproperty.cpp
 **  @author hedgeware <internal(at)hedgeware.net>
 **  @date
 **
 **  @brief
 **  @copyright
 **  All rights reserved. This program and the accompanying materials
 **  are made available under the terms of the GNU Lesser General Public License
 **  (LGPL) version 2.1 which accompanies this distribution, and is available at
 **  http://www.gnu.org/licenses/lgpl-2.1.html
 **
 **  This library is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 **  Lesser General Public License for more details.
 **
 *************************************************************************/

#include "vproperty.h"

#include <QByteArray>
#include <QFlags>
#include <QItemEditorFactory>
#include <QLineEdit>
#include <QList>
#include <QMetaProperty>
#include <QObject>
#include <QStandardItemEditorCreator>
#include <QWidget>

#include "vproperty_p.h"

//! Standard constructor, takes a name and a parent property as argument
VPE::VProperty::VProperty(const QString& name,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                          QMetaType::Type type)
#else
                          QVariant::Type type)
#endif
    : QObject(), vproperty_d_ptr(new VPropertyPrivate(name, type))
{

}

VPE::VProperty::VProperty(VPropertyPrivate *d)
    : vproperty_d_ptr(d)
{
}


VPE::VProperty::~VProperty()
{
    VPE::VProperty::setPropertyParent(nullptr);

    while (!vproperty_d_ptr->Children.isEmpty())
    {
        VProperty* tmpChild = vproperty_d_ptr->Children.takeLast();
        delete tmpChild;
    }

    delete vproperty_d_ptr;
}

auto VPE::VProperty::type() const -> QString
{
    return "string";
}

//! Get the data how it should be displayed
auto VPE::VProperty::data(int column, int role) const -> QVariant
{
    if (column == DPC_Name && Qt::DisplayRole == role)
    {
        return QVariant(vproperty_d_ptr->Name);
    }
    else if (column == DPC_Data && (Qt::DisplayRole == role || Qt::EditRole == role))
    {
        return vproperty_d_ptr->VariantValue;
    }
    else if (Qt::ToolTipRole == role)
    {
        return QVariant(vproperty_d_ptr->Description);
    }
    else
        return QVariant();
}

auto VPE::VProperty::setData(const QVariant &data, int role) -> bool
{
    bool tmpResult = false;
    if (Qt::EditRole == role)
    {
        tmpResult = (vproperty_d_ptr->VariantValue != data);
        setValue(data);
    }

    return tmpResult;
}

auto VPE::VProperty::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index,
                           const QAbstractItemDelegate *delegate) const -> bool
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(index)
    Q_UNUSED(delegate)

    return false;
}

//! Returns an editor widget, or NULL if it doesn't supply one
auto VPE::VProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                  const QAbstractItemDelegate *delegate) -> QWidget *
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    auto *factory = new QItemEditorFactory;
    QItemEditorCreatorBase *lineCreator = new QStandardItemEditorCreator<QLineEdit>();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    factory->registerEditor(QMetaType::QString, lineCreator);
#else
    factory->registerEditor(QVariant::String, lineCreator);
#endif
    QItemEditorFactory::setDefaultFactory(factory);

    vproperty_d_ptr->editor = factory->createEditor(static_cast<int>(vproperty_d_ptr->PropertyVariantType), parent);
    return vproperty_d_ptr->editor;
}

auto VPE::VProperty::setEditorData(QWidget *editor) -> bool
{
    if (!editor)
    {
        return false;
    }

    if (QByteArray const n = editor->metaObject()->userProperty().name(); !n.isEmpty())
    {
        const QSignalBlocker blocker(editor);
        editor->setProperty(n, vproperty_d_ptr->VariantValue);
        return true;
    }

    return false;
}

//! Gets the data from the widget
auto VPE::VProperty::getEditorData(const QWidget *editor) const -> QVariant
{
    if (!editor)
    {
        return {};
    }

    if (QByteArray const n = editor->metaObject()->userProperty().name(); !n.isEmpty())
    {
        return editor->property(n);
    }

    return {};
}

//! Returns item flags
auto VPE::VProperty::flags(int column) const -> Qt::ItemFlags
{
    if (column == DPC_Name)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    if (column == DPC_Data)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    }

    return Qt::NoItemFlags;
}


void VPE::VProperty::setValue(const QVariant &value)
{
    vproperty_d_ptr->VariantValue = value;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    vproperty_d_ptr->VariantValue.convert(QMetaType(vproperty_d_ptr->PropertyVariantType));
#else
    vproperty_d_ptr->VariantValue.convert(static_cast<int>(vproperty_d_ptr->PropertyVariantType));
#endif
    if (vproperty_d_ptr->editor != nullptr)
    {
        setEditorData(vproperty_d_ptr->editor);
    }
}

auto VPE::VProperty::getValue() const -> QVariant
{
    return vproperty_d_ptr->VariantValue;
}

// cppcheck-suppress unusedFunction
auto VPE::VProperty::serialize() const -> QString
{
    return getValue().toString();
}

void VPE::VProperty::deserialize(const QString& value)
{
    setValue(QVariant(value));
}


void VPE::VProperty::setName(const QString& name)
{
    vproperty_d_ptr->Name = name;
}

auto VPE::VProperty::getName() const -> QString
{
    return vproperty_d_ptr->Name;
}


void VPE::VProperty::setDescription(const QString& desc)
{
    vproperty_d_ptr->Description = desc;
}

auto VPE::VProperty::getDescription() const -> QString
{
    return vproperty_d_ptr->Description;
}

//! Returns a reference to the list of children
auto VPE::VProperty::getChildren() -> QList<VPE::VProperty *> &
{
    return vproperty_d_ptr->Children;
}

//! Returns a reference to the list of children
auto VPE::VProperty::getChildren() const -> const QList<VPE::VProperty *> &
{
    return vproperty_d_ptr->Children;
}

//! Returns the child at a certain row
auto VPE::VProperty::getChild(int row) const -> VPE::VProperty *
{
    if (row >= 0 && row < getRowCount())
    {
        return vproperty_d_ptr->Children.at(row);
    }
    else
        return nullptr;
}

//! Gets the number of children
auto VPE::VProperty::getRowCount() const -> vpesizetype
{
   return vproperty_d_ptr->Children.count();
}

//! Gets the parent of this property
auto VPE::VProperty::getParent() const -> VPE::VProperty *
{
    return vproperty_d_ptr->Parent;
}

//! Sets the parent of this property
void VPE::VProperty::setPropertyParent(VProperty *parent)
{
    if (vproperty_d_ptr->Parent == parent)
    {
        return;
    }

    VProperty* oldParent = vproperty_d_ptr->Parent;
    vproperty_d_ptr->Parent = parent;

    if (oldParent)
    {
        oldParent->removeChild(this);
    }

    if (vproperty_d_ptr->Parent && vproperty_d_ptr->Parent->getChildRow(this) == -1)
    {
        vproperty_d_ptr->Parent->addChild(this);
    }
}

auto VPE::VProperty::addChild(VProperty *child) -> vpesizetype
{
    if (child && child->getParent() != this)
    {
        child->setPropertyParent(this);
    }

    if (!vproperty_d_ptr->Children.contains(child) && child != nullptr)
    {
        vproperty_d_ptr->Children.push_back(child);
        return vproperty_d_ptr->Children.count()-1;
    }
    else
    {
        return vproperty_d_ptr->Children.indexOf(child);
    }
}

//! Removes a child from the children list
void VPE::VProperty::removeChild(VProperty* child)
{
    vproperty_d_ptr->Children.removeAll(child);

    if (child && child->getParent() == this)
    {
        child->setPropertyParent(nullptr);
    }
}

//! Returns the row the child has
auto VPE::VProperty::getChildRow(VProperty *child) const -> vpesizetype
{
    return vproperty_d_ptr->Children.indexOf(child);
}

//! Returns whether the views have to update the parent of this property if it changes
auto VPE::VProperty::getUpdateParent() const -> bool
{
    return vproperty_d_ptr->UpdateParent;
}

//! Returns whether the views have to update the children of this property if it changes
auto VPE::VProperty::getUpdateChildren() const -> bool
{
    return vproperty_d_ptr->UpdateChildren;
}

//! Sets whether the views should update Parents or children after this property changes
void VPE::VProperty::setUpdateBehaviour(bool update_parent, bool update_children)
{
    vproperty_d_ptr->UpdateParent = update_parent;
    vproperty_d_ptr->UpdateChildren = update_children;
}


void VPE::VProperty::setSettings(const QMap<QString, QVariant>& settings)
{
    QMap<QString, QVariant>::const_iterator tmpIterator = settings.constBegin();
    for (; tmpIterator != settings.constEnd(); ++tmpIterator)
    {
        setSetting(tmpIterator.key(), tmpIterator.value());
    }
}

auto VPE::VProperty::getSettings() const -> QMap<QString, QVariant>
{
    QMap<QString, QVariant> tmpResult;

    const QStringList tmpKeyList = getSettingKeys();
    for (const auto &tmpKey : tmpKeyList)
    {
        tmpResult.insert(tmpKey, getSetting(tmpKey));
    }

    return tmpResult;
}

void VPE::VProperty::setSetting(const QString& key, const QVariant& value)
{
    Q_UNUSED(key)
    Q_UNUSED(value)
    // Not needed in the Standard property
}

auto VPE::VProperty::getSetting(const QString &key) const -> QVariant
{
    // Not needed in the Standard property
    Q_UNUSED(key)
    return {};
}

auto VPE::VProperty::getSettingKeys() const -> QStringList
{
    return {};
}

auto VPE::VProperty::clone(bool include_children, VProperty *container) const -> VPE::VProperty *
{
    if (!container)
    {
        container = new VProperty(getName(), vproperty_d_ptr->PropertyVariantType);
    }

    container->setName(getName());
    container->setDescription(getDescription());
    container->setValue(getValue());
    container->setSettings(getSettings());
    container->setUpdateBehaviour(getUpdateParent(), getUpdateChildren());
    container->setPropertyType(propertyType());

    if (include_children)
    {
        const QList<VProperty*> children = vproperty_d_ptr->Children;
        for (auto *tmpChild : children)
        {
            container->addChild(tmpChild->clone(true));
        }
    }

    return container;
}

auto VPE::VProperty::propertyType() const -> VPE::Property
{
    return vproperty_d_ptr->type;
}

void VPE::VProperty::setPropertyType(const Property &type)
{
    vproperty_d_ptr->type = type;
}

void VPE::VProperty::UpdateParent(const QVariant &value)
{
    Q_UNUSED(value)
}

void VPE::VProperty::ValueChildChanged(const QVariant &value, int typeForParent)
{
    Q_UNUSED(value)
    Q_UNUSED(typeForParent)
}

VPE::UserChangeEvent::~UserChangeEvent()
{}

VPE::VPropertyPrivate::~VPropertyPrivate()
{}
