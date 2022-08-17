/************************************************************************
 **
 **  @file   dialogpatternproperties.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 2, 2014
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

#include "dialogpatternproperties.h"
#include "ui_dialogpatternproperties.h"
#include <QBuffer>
#include <QPushButton>
#include <QFileDialog>
#include <QMenu>
#include <QDate>
#include <QMessageBox>
#include <QRadioButton>
#include <QCompleter>
#include <QSet>
#include <QImageReader>
#include <QMimeType>
#include <QDesktopServices>
#include <QUrl>

#include "../xml/vpattern.h"
#include "../vpatterndb/vcontainer.h"
#include "../core/vapplication.h"
#include "../vmisc/vvalentinasettings.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../qmuparser/qmudef.h"
#include "../ifc/xml/vpatternimage.h"
#include "../ifc/xml/utils.h"

//---------------------------------------------------------------------------------------------------------------------
DialogPatternProperties::DialogPatternProperties(VPattern *doc,  VContainer *pattern, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::DialogPatternProperties),
      m_doc(doc),
      m_pattern(pattern)
{
    ui->setupUi(this);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    SCASSERT(doc != nullptr)

    VValentinaSettings *settings = VAbstractValApplication::VApp()->ValentinaSettings();
    settings->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    if (VAbstractValApplication::VApp()->GetPatternPath().isEmpty())
    {
        ui->lineEditPathToFile->setText(tr("<Empty>"));
        ui->lineEditPathToFile->setToolTip(tr("File was not saved yet."));
        ui->pushButtonShowInExplorer->setEnabled(false);
    }
    else
    {
        ui->lineEditPathToFile->setText(QDir::toNativeSeparators(VAbstractValApplication::VApp()->GetPatternPath()));
        ui->lineEditPathToFile->setToolTip(QDir::toNativeSeparators(VAbstractValApplication::VApp()->GetPatternPath()));
        ui->pushButtonShowInExplorer->setEnabled(true);
    }
    ui->lineEditPathToFile->setCursorPosition(0);

    connect(ui->pushButtonShowInExplorer, &QPushButton::clicked, this, []()
    {
        ShowInGraphicalShell(VAbstractValApplication::VApp()->GetPatternPath());
    });
#if defined(Q_OS_MAC)
    ui->pushButtonShowInExplorer->setText(tr("Show in Finder"));
#endif //defined(Q_OS_MAC)

    //----------------------- Label language
    for (auto &name : VApplication::LabelLanguages())
    {
        ui->comboBoxLabelLanguage->addItem(QLocale(name).nativeLanguageName(), name);
    }

    int index = ui->comboBoxLabelLanguage->findData(
                VAbstractValApplication::VApp()->ValentinaSettings()->GetLabelLanguage());
    if (index != -1)
    {
        ui->comboBoxLabelLanguage->setCurrentIndex(index);
    }

    connect(ui->comboBoxLabelLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DialogPatternProperties::DescEdited);

    ui->plainTextEditDescription->setPlainText(doc->GetDescription());
    connect(ui->plainTextEditDescription, &QPlainTextEdit::textChanged, this, &DialogPatternProperties::DescEdited);

    ui->plainTextEditTechNotes->setPlainText(doc->GetNotes());
    connect(ui->plainTextEditTechNotes, &QPlainTextEdit::textChanged, this, &DialogPatternProperties::DescEdited);

    InitImage();

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &DialogPatternProperties::Ok);
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this,
            &DialogPatternProperties::Apply);

    QPushButton *bCancel = ui->buttonBox->button(QDialogButtonBox::Cancel);
    SCASSERT(bCancel != nullptr)
    connect(bCancel, &QPushButton::clicked, this, &DialogPatternProperties::close);

    ui->tabWidget->setCurrentIndex(0);

    const bool readOnly = doc->IsReadOnly();
    ui->checkBoxPatternReadOnly->setChecked(readOnly);
    if (not readOnly)
    {
        connect(ui->checkBoxPatternReadOnly, &QRadioButton::toggled, this, [this](){m_securityChanged = true;});
    }
    else
    {
        ui->checkBoxPatternReadOnly->setDisabled(true);
    }

    //----------------------- Passmark length
    m_variables = pattern->DataMeasurements().keys() + pattern->DataIncrements().keys();
    m_completer = new QCompleter(m_variables, this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setModelSorting(QCompleter::UnsortedModel);
    m_completer->setFilterMode(Qt::MatchContains);
    m_completer->setCaseSensitivity(Qt::CaseSensitive);
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated), this, [this]()
    {
        ValidatePassmarkLength();
        DescEdited();
    });

    ui->lineEditPassmarkLength->setCompleter(m_completer);
    connect(ui->lineEditPassmarkLength, &QLineEdit::textEdited, this, [this]()
    {
        ValidatePassmarkLength();
        DescEdited();
    });

    ui->lineEditPassmarkLength->installEventFilter(this);
    m_oldPassmarkLength = doc->GetPassmarkLengthVariable();
    ui->lineEditPassmarkLength->setText(m_oldPassmarkLength);
    ValidatePassmarkLength();

    //Initialization change value. Set to default value after initialization
    m_defaultChanged = false;
    m_securityChanged = false;
}

//---------------------------------------------------------------------------------------------------------------------
DialogPatternProperties::~DialogPatternProperties()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
auto DialogPatternProperties::eventFilter(QObject *object, QEvent *event) -> bool
{
    if (ui->lineEditPassmarkLength == qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
            if ((keyEvent->key() == Qt::Key_Space) && ((keyEvent->modifiers() & Qt::ControlModifier) != 0U))
            {
                m_completer->complete();
                return true;
            }
        }

        return false;// clazy:exclude=base-class-event
    }

    return QDialog::eventFilter(object, event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::Apply()
{
    switch (ui->tabWidget->currentIndex())
    {
        case 0:
            SaveDescription();
            break;
        case 1:
            SaveReadOnlyState();
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::Ok()
{
    SaveDescription();
    SaveReadOnlyState();

    close();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::DescEdited()
{
    m_descriptionChanged = true;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::SaveDescription()
{
    if (m_descriptionChanged)
    {
        m_doc->SetNotes(ui->plainTextEditTechNotes->document()->toPlainText());
        m_doc->SetDescription(ui->plainTextEditDescription->document()->toPlainText());
        m_doc->SetLabelPrefix(qvariant_cast<QString>(ui->comboBoxLabelLanguage->currentData()));
        m_doc->SetPassmarkLengthVariable(ui->lineEditPassmarkLength->text());

        if (m_oldPassmarkLength != ui->lineEditPassmarkLength->text())
        {
            emit UpddatePieces();
            m_oldPassmarkLength = ui->lineEditPassmarkLength->text();
        }

        m_descriptionChanged = false;
        emit m_doc->patternChanged(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::SaveReadOnlyState()
{
    if (m_securityChanged)
    {
        m_doc->SetReadOnly(ui->checkBoxPatternReadOnly->isChecked());
        m_securityChanged = false;
        emit m_doc->patternChanged(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::ValidatePassmarkLength() const
{
    const QString text = ui->lineEditPassmarkLength->text();
    QPalette palette = ui->lineEditPassmarkLength->palette();
    const QPalette::ColorRole foregroundRole = ui->lineEditPassmarkLength->foregroundRole();

    QRegularExpression rx(NameRegExp());
    if (not text.isEmpty())
    {
        palette.setColor(foregroundRole,
                         rx.match(text).hasMatch() && m_variables.contains(text) ? Qt::black : Qt::red);
    }
    else
    {
        palette.setColor(foregroundRole, Qt::black);
    }

    ui->lineEditPassmarkLength->setPalette(palette);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::InitImage()
{
    ui->imageLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->imageLabel->setScaledContents(true);
    connect(ui->imageLabel, &QWidget::customContextMenuRequested, this, [this]()
    {
        QMenu menu(this);
        menu.addAction(m_deleteAction);
        menu.addAction(m_changeImageAction);
        menu.addAction(m_saveImageAction);
        menu.addAction(m_showImageAction);
        menu.exec(QCursor::pos());
        menu.show();
    });

    m_deleteAction      = new QAction(tr("Delete image"), this);
    m_changeImageAction = new QAction(tr("Change image"), this);
    m_saveImageAction   = new QAction(tr("Save image to file"), this);
    m_showImageAction   = new QAction(tr("Show image"), this);

    connect(m_deleteAction, &QAction::triggered, this, [this]()
    {
        m_doc->DeleteImage();
        ui->imageLabel->setText(tr("Change image"));
        m_deleteAction->setEnabled(false);
        m_saveImageAction->setEnabled(false);
        m_showImageAction->setEnabled(false);
    });

    connect(m_changeImageAction, &QAction::triggered, this, &DialogPatternProperties::ChangeImage);
    connect(m_saveImageAction, &QAction::triggered, this, &DialogPatternProperties::SaveImage);
    connect(m_showImageAction, &QAction::triggered, this, &DialogPatternProperties::ShowImage);

    const VPatternImage image = m_doc->GetImage();
    if (image.IsValid())
    {
        ui->imageLabel->setPixmap(image.GetPixmap(ui->imageLabel->width(), ui->imageLabel->height()));
    }
    else
    {
        m_deleteAction->setEnabled(false);
        m_saveImageAction->setEnabled(false);
        m_showImageAction->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::ChangeImage()
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Image for pattern"), QString(),
                                                          PrepareImageFilters(), nullptr,
                                                          VAbstractApplication::VApp()->NativeFileDialog());
    if (not fileName.isEmpty())
    {
        VPatternImage image = VPatternImage::FromFile(fileName);

        if (not image.IsValid())
        {
            qCritical() << tr("Invalid image. Error: %1").arg(image.ErrorString());
            return;
        }

        m_doc->SetImage(image);
        ui->imageLabel->setPixmap(image.GetPixmap(ui->imageLabel->width(), ui->imageLabel->height()));

        m_deleteAction->setEnabled(true);
        m_saveImageAction->setEnabled(true);
        m_showImageAction->setEnabled(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::SaveImage()
{
    const VPatternImage image = m_doc->GetImage();

    if (not image.IsValid())
    {
        qCritical() << tr("Unable to save image. Error: %1").arg(image.ErrorString());
        return;
    }

    QMimeType mime = image.MimeTypeFromData();
    QString path = QDir::homePath() + QDir::separator() + tr("untitled");

    QStringList suffixes = mime.suffixes();
    if (not suffixes.isEmpty())
    {
        path += '.' + suffixes.at(0);
    }

    QString filter = mime.filterString();
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), path, filter, nullptr,
                                                    VAbstractApplication::VApp()->NativeFileDialog());
    if (not filename.isEmpty())
    {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(QByteArray::fromBase64(image.ContentData()));
        }
        else
        {
            qCritical() << tr("Unable to save image. Error: %1").arg(file.errorString());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPatternProperties::ShowImage()
{
    const VPatternImage image = m_doc->GetImage();

    if (not image.IsValid())
    {
        qCritical() << tr("Unable to show image. Error: %1").arg(image.ErrorString());
        return;
    }

    QMimeType mime = image.MimeTypeFromData();
    QString name = QDir::tempPath() + QDir::separator() + QStringLiteral("image.XXXXXX");

    QStringList suffixes = mime.suffixes();
    if (not suffixes.isEmpty())
    {
        name += '.' + suffixes.at(0);
    }

    delete m_tmpImage;
    m_tmpImage = new QTemporaryFile(name, this);
    if (m_tmpImage->open())
    {
        m_tmpImage->write(QByteArray::fromBase64(image.ContentData()));
        m_tmpImage->flush();
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_tmpImage->fileName()));
    }
    else
    {
        qCritical() << tr("Unable to open temp file");
    }
}
