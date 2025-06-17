/************************************************************************
 **
 **  @file   vnumberproperty.h
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

#ifndef VNUMBERPROPERTY_H
#define VNUMBERPROPERTY_H

#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtGlobal>

#include "../vproperty.h"
#include "../vpropertyexplorer_global.h"

namespace VPE
{

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")

//! Class for holding an integer property
class VPROPERTYEXPLORERSHARED_EXPORT VIntegerProperty : public VProperty
{
    Q_OBJECT // NOLINT
public:
    VIntegerProperty(const QString& name, const QMap<QString, QVariant>& settings);

    explicit VIntegerProperty(const QString& name);

    //! Returns an editor widget, or NULL if it doesn't supply one
    //! \param parent The widget to which the editor will be added as a child
    //! \options Render options
    //! \delegate A pointer to the QAbstractItemDelegate requesting the editor. This can be used to connect signals and
    //! slots.
    auto createEditor(QWidget* parent, const QStyleOptionViewItem& options,
                      const QAbstractItemDelegate* delegate) -> QWidget* override;

    //! Gets the data from the widget
    auto getEditorData(const QWidget* editor) const -> QVariant override;

    //! Sets the settings. Available settings:
    //!
    //! key: "Min" - value: Minimum number as integer
    //! key: "Max" - value: Maximum number as integer
    //! key: "Step" - value: Increment step
    //! key: "Suffix" - value: Editor's suffix
    void setSetting(const QString& key, const QVariant& value) override;

    //! Get the settings. This function has to be implemented in a subclass in order to have an effect
    auto getSetting(const QString& key) const -> QVariant override;

    //! Returns the list of keys of the property's settings
    auto getSettingKeys() const -> QStringList override;

    //! Returns a string containing the type of the property
    auto type() const -> QString override;

    //! Clones this property
    //! \param include_children Indicates whether to also clone the children
    //! \param container If a property is being passed here, no new VProperty is being created but instead it is tried
    //! to fill all the data into container. This can also be used when subclassing this function.
    //! \return Returns the newly created property (or container, if it was not NULL)
    Q_REQUIRED_RESULT auto clone(bool include_children = true,
                                 VProperty* container = nullptr) const -> VProperty* override;
public slots:
    void valueChanged(int i);
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VIntegerProperty) // NOLINT

    double m_minValue;
    double m_maxValue;
    double m_singleStep;
    QString m_suffix{};

    static const int StandardMin;// = -1000000;
    static const int StandardMax;// = 1000000;
};


//! Class for holding a double property
class VPROPERTYEXPLORERSHARED_EXPORT VDoubleProperty : public VProperty
{
    Q_OBJECT // NOLINT
public:
    VDoubleProperty(const QString& name, const QMap<QString, QVariant>& settings);

    explicit VDoubleProperty(const QString& name);

    //! Returns an editor widget, or NULL if it doesn't supply one
    //! \param parent The widget to which the editor will be added as a child
    //! \options Render options
    //! \delegate A pointer to the QAbstractItemDelegate requesting the editor. This can be used to connect signals and
    //! slots.
    auto createEditor(QWidget* parent, const QStyleOptionViewItem& options,
                      const QAbstractItemDelegate* delegate) -> QWidget* override;

    //! Gets the data from the widget
    auto getEditorData(const QWidget* editor) const -> QVariant override;

    //! Sets the settings. Available settings:
    //!
    //! key: "Min" - value: Minimum number as integer
    //! key: "Max" - value: Maximum number as integer
    //! key: "Step" - value: Increment step
    //! key: "Suffix" - value: Editor's suffix
    //! key: "Precision" - value: Number of decimals after the decimal point
    void setSetting(const QString& key, const QVariant& value) override;

    //! Get the settings. This function has to be implemented in a subclass in order to have an effect
    auto getSetting(const QString& key) const -> QVariant override;

    //! Returns the list of keys of the property's settings
    auto getSettingKeys() const -> QStringList override;

    //! Returns a string containing the type of the property
    auto type() const -> QString override;

    //! Clones this property
    //! \param include_children Indicates whether to also clone the children
    //! \param container If a property is being passed here, no new VProperty is being created but instead it is tried
    //! to fill all the data into container. This can also be used when subclassing this function.
    //! \return Returns the newly created property (or container, if it was not NULL)
    auto clone(bool include_children = true, VProperty* container = nullptr) const -> VProperty* override;

public slots:
    void valueChanged(int i);

private:
    Q_DISABLE_COPY_MOVE(VDoubleProperty) // NOLINT

    double m_minValue;
    double m_maxValue;
    double m_singleStep;
    QString m_suffix{};

    //! Number of decimals after the decimal point
    int m_precision;

    static const int StandardMin;// = -1000000;
    static const int StandardMax;// = 1000000;
    const static double StandardPrecision;// = 5;
};

QT_WARNING_POP

}  // namespace VPE

#endif // VNUMBERPROPERTY_H
