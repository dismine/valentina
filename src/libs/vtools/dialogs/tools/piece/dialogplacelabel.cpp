/************************************************************************
 **
 **  @file   dialogplacelabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 10, 2017
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
#include "dialogplacelabel.h"
#include "ui_dialogplacelabel.h"

#include "../../../visualization/line/vistoolspecialpoint.h"
#include "../../support/dialogeditwrongformula.h"
#include "../vmisc/theme/vtheme.h"
#include "../vpatterndb/vcontainer.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)

#include <QTimer>

//---------------------------------------------------------------------------------------------------------------------
DialogPlaceLabel::DialogPlaceLabel(const VContainer *data, VAbstractPattern *doc, quint32 toolId, QWidget *parent)
  : DialogTool(data, doc, toolId, parent),
    ui(new Ui::DialogPlaceLabel),
    m_showMode(false),
    m_formulaBaseHeightWidth(0),
    m_formulaBaseHeightHeight(0),
    m_formulaBaseHeightAngle(0),
    m_formulaBaseVisible(0),
    timerAngle(new QTimer(this)),
    timerWidth(new QTimer(this)),
    timerHeight(new QTimer(this)),
    m_timerVisible(new QTimer(this)),
    m_flagPoint(false),
    m_flagWidth(false),
    m_flagHeight(false),
    m_flagAngle(false),
    m_flagFormulaVisible(false),
    m_flagError(false)
{
    ui->setupUi(this);
    InitOkCancel(ui);

    InitIcons();

    InitPlaceLabelTab();
    InitControlTab();

    EvalVisible();

    connect(ui->comboBoxPiece, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { CheckPieces(); });

    vis = new VisToolSpecialPoint(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPlaceLabel::~DialogPlaceLabel()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::EnbleShowMode(bool disable)
{
    m_showMode = disable;
    ui->comboBoxPiece->setDisabled(m_showMode);
    ui->comboBoxPoint->setDisabled(m_showMode);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPlaceLabel::GetCenterPoint() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::SetCenterPoint(quint32 id)
{
    setCurrentPointId(ui->comboBoxPoint, id);

    if (not m_showMode)
    {
        VisToolSpecialPoint *point = qobject_cast<VisToolSpecialPoint *>(vis);
        SCASSERT(point != nullptr)
        point->SetPointId(id);
    }

    CheckPoint();
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPlaceLabel::GetLabelType() const -> PlaceLabelType
{
    return static_cast<PlaceLabelType>(ui->comboBoxLabelType->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::SetLabelType(PlaceLabelType type)
{
    const qint32 index = ui->comboBoxLabelType->findData(static_cast<int>(type));
    if (index != -1)
    {
        ui->comboBoxLabelType->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPlaceLabel::GetWidth() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(ui->plainTextEditFormulaWidth->toPlainText(),
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::SetWidth(const QString &value)
{
    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed. TODO : see if I can get the max number of caracters in one line
    // of this PlainTextEdit to change 80 to this value
    if (formula.length() > 80)
    {
        this->DeployFormulaWidthEdit();
    }
    ui->plainTextEditFormulaWidth->setPlainText(formula);

    //    VisToolPlaceLabel *point = qobject_cast<VisToolPlaceLabel *>(vis);
    //    SCASSERT(point != nullptr)
    //    point->SetPointId(id);

    MoveCursorToEnd(ui->plainTextEditFormulaWidth);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPlaceLabel::GetHeight() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(ui->plainTextEditFormulaHeight->toPlainText(),
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::SetHeight(const QString &value)
{
    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed. TODO : see if I can get the max number of caracters in one line
    // of this PlainTextEdit to change 80 to this value
    if (formula.length() > 80)
    {
        this->DeployFormulaHeightEdit();
    }
    ui->plainTextEditFormulaHeight->setPlainText(formula);

    //    VisToolPlaceLabel *point = qobject_cast<VisToolPlaceLabel *>(vis);
    //    SCASSERT(point != nullptr)
    //    point->SetPointId(id);

    MoveCursorToEnd(ui->plainTextEditFormulaHeight);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPlaceLabel::GetAngle() const -> QString
{
    return VTranslateVars::TryFormulaFromUser(ui->plainTextEditFormulaAngle->toPlainText(),
                                              VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::SetAngle(const QString &value)
{
    const QString formula = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        value, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed. TODO : see if I can get the max number of caracters in one line
    // of this PlainTextEdit to change 80 to this value
    if (formula.length() > 80)
    {
        this->DeployFormulaAngleEdit();
    }
    ui->plainTextEditFormulaAngle->setPlainText(formula);

    //    VisToolPlaceLabel *point = qobject_cast<VisToolPlaceLabel *>(vis);
    //    SCASSERT(point != nullptr)
    //    point->SetPointId(id);

    MoveCursorToEnd(ui->plainTextEditFormulaAngle);
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPlaceLabel::GetPieceId() const -> quint32
{
    return getCurrentObjectId(ui->comboBoxPiece);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::SetPieceId(quint32 id)
{
    if (ui->comboBoxPiece->count() <= 0)
    {
        ui->comboBoxPiece->addItem(data->GetPiece(id).GetName(), id);
    }
    else
    {
        const qint32 index = ui->comboBoxPiece->findData(id);
        if (index != -1)
        {
            ui->comboBoxPiece->setCurrentIndex(index);
        }
        else
        {
            ui->comboBoxPiece->setCurrentIndex(0);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::SetPiecesList(const QVector<quint32> &list)
{
    FillComboBoxPiecesList(ui->comboBoxPiece, list);

    if (list.isEmpty())
    {
        qWarning() << tr("The list of pieces is empty. Please, first create at least one piece for current pattern "
                         "piece.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not prepare)
    {
        if (type == SceneObject::Point)
        {
            if (SetObject(id, ui->comboBoxPoint, QString()))
            {
                if (vis != nullptr)
                {
                    vis->VisualMode(id);
                }
                CheckPoint();
                prepare = true;

                timerWidth->setSingleShot(formulaTimerTimeout);
                timerHeight->setSingleShot(formulaTimerTimeout);
                timerAngle->setSingleShot(formulaTimerTimeout);

                this->setModal(true);
                this->show();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::ShowVisualization()
{
    if (not m_showMode)
    {
        AddVisualization<VisToolSpecialPoint>();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormulaWidth->blockSignals(true);
    ui->plainTextEditFormulaHeight->blockSignals(true);
    ui->plainTextEditFormulaAngle->blockSignals(true);
    ui->plainTextEditFormulaVisible->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    if (event->type() == QEvent::PaletteChange)
    {
        InitIcons();
        InitDialogButtonBoxIcons(ui->buttonBox);
    }

    // remember to call base class implementation
    DialogTool::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::DeployFormulaWidthEdit()
{
    DeployFormula(this, ui->plainTextEditFormulaWidth, ui->pushButtonGrowWidth, m_formulaBaseHeightWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::DeployFormulaHeightEdit()
{
    DeployFormula(this, ui->plainTextEditFormulaHeight, ui->pushButtonGrowHeight, m_formulaBaseHeightHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::DeployFormulaAngleEdit()
{
    DeployFormula(this, ui->plainTextEditFormulaAngle, ui->pushButtonGrowAngle, m_formulaBaseHeightAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::DeployVisibleFormulaTextEdit()
{
    DeployFormula(this, ui->plainTextEditFormulaVisible, ui->pushButtonGrowVisible, m_formulaBaseVisible);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::EvalWidth()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormulaWidth->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditFormulaWidth;
    formulaData.labelResult = ui->labelResultCalculationWidth;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    formulaData.checkLessThanZero = true;

    Eval(formulaData, m_flagWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::EvalHeight()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormulaHeight->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditFormulaHeight;
    formulaData.labelResult = ui->labelResultCalculationHeight;
    formulaData.postfix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    formulaData.checkLessThanZero = true;

    Eval(formulaData, m_flagHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::EvalAngle()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormulaAngle->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditFormulaAngle;
    formulaData.labelResult = ui->labelResultCalculationAngle;
    formulaData.postfix = degreeSymbol;
    formulaData.checkZero = false;

    Eval(formulaData, m_flagAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::EvalVisible()
{
    FormulaData formulaData;
    formulaData.formula = ui->plainTextEditFormulaVisible->toPlainText();
    formulaData.variables = data->DataVariables();
    formulaData.labelEditFormula = ui->labelEditVisible;
    formulaData.labelResult = ui->labelResultVisible;
    formulaData.postfix = QString();
    formulaData.checkZero = false;
    formulaData.checkLessThanZero = true;

    Eval(formulaData, m_flagFormulaVisible);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::FXWidth()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit rectangle width"));
    dialog->SetFormula(GetWidth());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetWidth(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::FXHeight()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit rectangle width"));
    dialog->SetFormula(GetHeight());
    dialog->setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetHeight(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::FXAngle()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Edit angle"));
    dialog->SetFormula(GetAngle());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetAngle(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::FXVisible()
{
    QScopedPointer<DialogEditWrongFormula> dialog(new DialogEditWrongFormula(data, toolId, this));
    dialog->setWindowTitle(tr("Control visibility"));
    dialog->SetFormula(GetFormulaVisible());
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaVisible(dialog->GetFormula());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::InitPlaceLabelTab()
{
    FillComboBoxPoints(ui->comboBoxPoint);
    FillPlaceLabelTypes();

    m_formulaBaseHeightWidth = ui->plainTextEditFormulaWidth->height();
    m_formulaBaseHeightHeight = ui->plainTextEditFormulaHeight->height();
    m_formulaBaseHeightAngle = ui->plainTextEditFormulaAngle->height();

    ui->plainTextEditFormulaWidth->installEventFilter(this);
    ui->plainTextEditFormulaHeight->installEventFilter(this);
    ui->plainTextEditFormulaAngle->installEventFilter(this);

    ui->plainTextEditFormulaWidth->setPlainText(
        QString::number(UnitConvertor(1, Unit::Cm, VAbstractValApplication::VApp()->patternUnits())));
    ui->plainTextEditFormulaHeight->setPlainText(
        QString::number(UnitConvertor(1, Unit::Cm, VAbstractValApplication::VApp()->patternUnits())));

    connect(ui->toolButtonExprWidth, &QPushButton::clicked, this, &DialogPlaceLabel::FXWidth);
    connect(ui->toolButtonExprHeight, &QPushButton::clicked, this, &DialogPlaceLabel::FXHeight);
    connect(ui->toolButtonExprAngle, &QPushButton::clicked, this, &DialogPlaceLabel::FXAngle);

    connect(ui->plainTextEditFormulaWidth, &QPlainTextEdit::textChanged, this,
            [this]() { timerWidth->start(formulaTimerTimeout); });

    connect(ui->plainTextEditFormulaHeight, &QPlainTextEdit::textChanged, this,
            [this]() { timerHeight->start(formulaTimerTimeout); });

    connect(ui->plainTextEditFormulaAngle, &QPlainTextEdit::textChanged, this,
            [this]() { timerAngle->start(formulaTimerTimeout); });

    connect(ui->pushButtonGrowWidth, &QPushButton::clicked, this, &DialogPlaceLabel::DeployFormulaWidthEdit);
    connect(ui->pushButtonGrowHeight, &QPushButton::clicked, this, &DialogPlaceLabel::DeployFormulaHeightEdit);
    connect(ui->pushButtonGrowAngle, &QPushButton::clicked, this, &DialogPlaceLabel::DeployFormulaAngleEdit);

    timerWidth->setSingleShot(true);
    timerHeight->setSingleShot(true);
    timerAngle->setSingleShot(true);

    connect(timerWidth, &QTimer::timeout, this, &DialogPlaceLabel::EvalWidth);
    connect(timerHeight, &QTimer::timeout, this, &DialogPlaceLabel::EvalHeight);
    connect(timerAngle, &QTimer::timeout, this, &DialogPlaceLabel::EvalAngle);

    EvalWidth();
    EvalHeight();
    EvalAngle();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::InitControlTab()
{
    this->m_formulaBaseVisible = ui->plainTextEditFormulaVisible->height();

    ui->plainTextEditFormulaVisible->installEventFilter(this);

    m_timerVisible->setSingleShot(true);

    connect(m_timerVisible, &QTimer::timeout, this, &DialogPlaceLabel::EvalVisible);
    connect(ui->toolButtonExprVisible, &QPushButton::clicked, this, &DialogPlaceLabel::FXVisible);
    connect(ui->plainTextEditFormulaVisible, &QPlainTextEdit::textChanged, this,
            [this]() { m_timerVisible->start(formulaTimerTimeout); });
    connect(ui->pushButtonGrowVisible, &QPushButton::clicked, this, &DialogPlaceLabel::DeployVisibleFormulaTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::FillPlaceLabelTypes()
{
    ui->comboBoxLabelType->addItem(tr("Segment"), static_cast<int>(PlaceLabelType::Segment));
    ui->comboBoxLabelType->addItem(tr("Rectangle"), static_cast<int>(PlaceLabelType::Rectangle));
    ui->comboBoxLabelType->addItem(tr("Cross"), static_cast<int>(PlaceLabelType::Cross));
    ui->comboBoxLabelType->addItem(tr("T-shaped"), static_cast<int>(PlaceLabelType::Tshaped));
    ui->comboBoxLabelType->addItem(tr("Doubletree"), static_cast<int>(PlaceLabelType::Doubletree));
    ui->comboBoxLabelType->addItem(tr("Corner"), static_cast<int>(PlaceLabelType::Corner));
    ui->comboBoxLabelType->addItem(tr("Triangle"), static_cast<int>(PlaceLabelType::Triangle));
    ui->comboBoxLabelType->addItem(tr("H-shaped"), static_cast<int>(PlaceLabelType::Hshaped));
    ui->comboBoxLabelType->addItem(tr("Button"), static_cast<int>(PlaceLabelType::Button));
    ui->comboBoxLabelType->addItem(tr("Circle"), static_cast<int>(PlaceLabelType::Circle));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::CheckPieces()
{
    QColor color;
    if (ui->comboBoxPiece->count() <= 0 || ui->comboBoxPiece->currentIndex() == -1)
    {
        m_flagError = false;
        color = errorColor;
    }
    else
    {
        m_flagError = true;
        color = OkColor(this);
    }
    ChangeColor(ui->labelPiece, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::CheckPoint()
{
    QColor color;
    if (ui->comboBoxPoint->currentIndex() != -1)
    {
        m_flagPoint = true;
        color = OkColor(this);
    }
    else
    {
        m_flagPoint = false;
        color = errorColor;
    }
    ChangeColor(ui->labelPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::InitIcons()
{
    const QString resource = QStringLiteral("icon");

    const QString fxIcon = QStringLiteral("24x24/fx.png");
    ui->toolButtonExprWidth->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprHeight->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprAngle->setIcon(VTheme::GetIconResource(resource, fxIcon));
    ui->toolButtonExprVisible->setIcon(VTheme::GetIconResource(resource, fxIcon));

    const QString equalIcon = QStringLiteral("24x24/equal.png");
    ui->label_4->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_5->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_6->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
    ui->label_8->setPixmap(VTheme::GetPixmapResource(resource, equalIcon));
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPlaceLabel::GetFormulaVisible() const -> QString
{
    QString formula = ui->plainTextEditFormulaVisible->toPlainText();
    return VTranslateVars::TryFormulaFromUser(formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPlaceLabel::SetFormulaVisible(const QString &formula)
{
    const QString f = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    // increase height if needed.
    if (f.length() > 80)
    {
        this->DeployVisibleFormulaTextEdit();
    }
    ui->plainTextEditFormulaVisible->setPlainText(f);
    MoveCursorToEnd(ui->plainTextEditFormulaVisible);
}
