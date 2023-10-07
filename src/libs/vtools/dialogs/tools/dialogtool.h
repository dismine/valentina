/************************************************************************
 **
 **  @file   dialogtool.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef DIALOGTOOL_H
#define DIALOGTOOL_H

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QLocale>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "../dialogtoolbox.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vtools/visualization/visualization.h" // Issue on Windows
#include "../vwidgets/vmaingraphicsscene.h"

template <class T> class QSharedPointer;

Q_DECLARE_LOGGING_CATEGORY(vDialog)

class QDoubleSpinBox;
class QLabel;
class QPlainTextEdit;
class VAbstractTool;

enum class FillComboBox : qint8
{
    Whole,
    NoChildren
};

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

/**
 * @brief The DialogTool class parent for all dialog of tools.
 */
class DialogTool : public QDialog
{
    Q_OBJECT // NOLINT

public:
    DialogTool(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent = nullptr);
    ~DialogTool() override;

    auto GetAssociatedTool() -> VAbstractTool *;
    void SetAssociatedTool(VAbstractTool *tool);

    virtual void ShowDialog(bool click);
    virtual void Build(const Tool &type);
    virtual void SetPiecesList(const QVector<quint32> &list);
    virtual void SetPatternDoc(VAbstractPattern *doc);

    auto GetToolId() const -> quint32;
    void SetToolId(const quint32 &value);

    virtual void SetGroupCategories(const QStringList &categories);

    static void MoveListRowTop(QListWidget *list);
    static void MoveListRowUp(QListWidget *list);
    static void MoveListRowDown(QListWidget *list);
    static void MoveListRowBottom(QListWidget *list);
signals:
    /**
     * @brief DialogClosed signal dialog closed
     * @param result keep result
     */
    void DialogClosed(int result);
    /**
     * @brief DialogApplied emit signal dialog apply changes
     */
    void DialogApplied();
    /**
     * @brief ToolTip emit tooltipe for tool
     * @param toolTip text tooltipe
     */
    void ToolTip(const QString &toolTip);
public slots:
    virtual void ChosenObject(quint32 id, const SceneObject &type);
    virtual void SelectedObject(bool selected, quint32 object, quint32 tool);
    virtual void DialogAccepted();
    /**
     * @brief DialogApply save data and emit signal DialogApplied.
     */
    virtual void DialogApply();
    virtual void DialogRejected();
    virtual void PointNameChanged() {}

protected:
    /** @brief data container with data */
    const VContainer *data;

    VAbstractPattern *m_doc;

    /** @brief isInitialized true if window is initialized */
    bool isInitialized;

    /** @brief bOk button ok */
    QPushButton *bOk;

    /** @brief bApply button apply */
    QPushButton *bApply;

    /**
     * @brief associatedTool vdrawtool associated with opened dialog.
     */
    VAbstractTool *associatedTool;
    quint32 toolId;

    /** @brief prepare show if we prepare. Show dialog after finish working with visual part of tool*/
    bool prepare;

    QPointer<Visualization> vis;

    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    auto eventFilter(QObject *object, QEvent *event) -> bool override;

    virtual auto IsValid() const -> bool = 0;
    virtual void CheckState();

    void FillComboBoxPiecesList(QComboBox *box, const QVector<quint32> &list);
    void FillComboBoxPoints(QComboBox *box, FillComboBox rule = FillComboBox::Whole, quint32 ch1 = NULL_ID,
                            quint32 ch2 = NULL_ID) const;
    void FillComboBoxArcs(QComboBox *box, FillComboBox rule = FillComboBox::Whole, quint32 ch1 = NULL_ID,
                          quint32 ch2 = NULL_ID) const;
    void FillComboBoxSplines(QComboBox *box) const;
    void FillComboBoxSplinesPath(QComboBox *box) const;
    void FillComboBoxCurves(QComboBox *box) const;
    void FillComboBoxTypeLine(QComboBox *box, const QMap<QString, QIcon> &stylesPics,
                              const QString &def = TypeLineLine) const;
    void FillComboBoxLineColors(QComboBox *box) const;
    void FillComboBoxLineColors(QComboBox *box, const QMap<QString, QString> &lineColors) const;
    void FillComboBoxCrossCirclesPoints(QComboBox *box) const;
    void FillComboBoxVCrossCurvesPoint(QComboBox *box) const;
    void FillComboBoxHCrossCurvesPoint(QComboBox *box) const;

    auto GetComboBoxCurrentData(const QComboBox *box, const QString &def) const -> QString;
    void ChangeCurrentData(QComboBox *box, const QVariant &value) const;
    auto Eval(const FormulaData &formulaData, bool &flag) -> qreal;

    void setCurrentPointId(QComboBox *box, const quint32 &value, FillComboBox rule = FillComboBox::NoChildren,
                           const quint32 &ch1 = NULL_ID, const quint32 &ch2 = NULL_ID) const;
    void setCurrentSplineId(QComboBox *box, const quint32 &value) const;
    void setCurrentArcId(QComboBox *box, const quint32 &value, FillComboBox rule = FillComboBox::NoChildren,
                         const quint32 &ch1 = NULL_ID, const quint32 &ch2 = NULL_ID) const;
    void setCurrentSplinePathId(QComboBox *box, const quint32 &value) const;
    void setCurrentCurveId(QComboBox *box, const quint32 &value) const;

    auto getCurrentObjectId(QComboBox *box) const -> quint32;

    template <typename T> auto getCurrentCrossPoint(QComboBox *box) const -> T;

    auto SetObject(const quint32 &id, QComboBox *box, const QString &toolTip) -> bool;

    template <typename T> void InitOkCancelApply(T *ui);

    template <typename T> void InitOkCancel(T *ui);

    template <typename T> void AddVisualization();

    template <typename T> auto GetListInternals(const QListWidget *list) const -> QVector<T>;

    virtual void ShowVisualization() {}
    /**
     * @brief SaveData Put dialog data in local variables
     */
    virtual void SaveData() {}
    auto DNumber(const QString &baseName) const -> quint32;
    void NewNodeItem(QListWidget *listWidget, const VPieceNode &node, bool showPassmark = true,
                     bool showExclusion = true);

    void InitNodeAngles(QComboBox *box);

private:
    Q_DISABLE_COPY_MOVE(DialogTool) // NOLINT
    void FillList(QComboBox *box, const QMap<QString, quint32> &list) const;

    template <typename T> void PrepareList(QMap<QString, quint32> &list, quint32 id) const;

    auto IsSpline(const QSharedPointer<VGObject> &obj) const -> bool;
    auto IsSplinePath(const QSharedPointer<VGObject> &obj) const -> bool;

    template <typename GObject>
    void FillCombo(QComboBox *box, GOType gType, FillComboBox rule = FillComboBox::Whole, const quint32 &ch1 = NULL_ID,
                   const quint32 &ch2 = NULL_ID) const;
};

// cppcheck-suppress unknownMacro
QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto DialogTool::GetListInternals(const QListWidget *list) const -> QVector<T>
{
    SCASSERT(list != nullptr)
    QVector<T> internals;
    for (qint32 i = 0; i < list->count(); ++i)
    {
        QListWidgetItem *item = list->item(i);
        internals.append(qvariant_cast<T>(item->data(Qt::UserRole)));
    }
    return internals;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogTool::GetAssociatedTool() -> VAbstractTool *
{
    return this->associatedTool;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
/**
 * @brief InitOkCancelApply initialise OK / Cancel and Apply buttons
 * @param ui Dialog container
 */
inline void DialogTool::InitOkCancelApply(T *ui)
{
    InitOkCancel(ui);
    bApply = ui->buttonBox->button(QDialogButtonBox::Apply);
    SCASSERT(bApply != nullptr)
    connect(bApply, &QPushButton::clicked, this, &DialogTool::DialogApply);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
/**
 * @brief InitOkCancel initialise OK and Cancel buttons
 * @param ui Dialog container
 */
inline void DialogTool::InitOkCancel(T *ui)
{
    bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)
    connect(bOk, &QPushButton::clicked, this, &DialogTool::DialogAccepted);

    QPushButton *bCancel = ui->buttonBox->button(QDialogButtonBox::Cancel);
    SCASSERT(bCancel != nullptr)
    connect(bCancel, &QPushButton::clicked, this, &DialogTool::DialogRejected);

    VAbstractApplication::VApp()->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void DialogTool::AddVisualization()
{
    if (prepare == false)
    {
        VMainGraphicsScene *scene =
            qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
        SCASSERT(scene != nullptr)

        T *toolVis = qobject_cast<T *>(vis);
        SCASSERT(toolVis != nullptr)

        if (not scene->items().contains(toolVis))
        {
            scene->addItem(toolVis);
        }

        toolVis->SetMode(Mode::Show);
        toolVis->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto DialogTool::getCurrentCrossPoint(QComboBox *box) const -> T
{
    int value;
    bool ok = false;
    value = box->currentData().toInt(&ok);

    if (not ok)
    {
        return static_cast<T>(1);
    }

    switch (value)
    {
        case 1:
        case 2:
            return static_cast<T>(value);
        default:
            return static_cast<T>(1);
    }
}

#endif // DIALOGTOOL_H
