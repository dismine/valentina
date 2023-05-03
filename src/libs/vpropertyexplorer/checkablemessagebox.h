/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** All rights reserved. This program and the accompanying materials are made
** available under the terms of the GNU Lesser General Public License (LGPL)
** version 2.1 which accompanies this distribution, and is available at
** http://www.gnu.org/licenses/lgpl-2.1.html
**
** This library is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
** for more details.
**
****************************************************************************/

#ifndef CHECKABLEMESSAGEBOX_H
#define CHECKABLEMESSAGEBOX_H

#include <qcompilerdetection.h>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFlags>
#include <QMessageBox>
#include <QMetaObject>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QtGlobal>

#include "vpropertyexplorer_global.h"

class QSettings;

namespace Utils
{

class CheckableMessageBoxPrivate;

class VPROPERTYEXPLORERSHARED_EXPORT CheckableMessageBox : public QDialog
{
    Q_OBJECT // NOLINT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QPixmap iconPixmap READ iconPixmap WRITE setIconPixmap)
    Q_PROPERTY(bool isChecked READ isChecked WRITE setChecked)
    Q_PROPERTY(QString checkBoxText READ checkBoxText WRITE setCheckBoxText)
    Q_PROPERTY(QDialogButtonBox::StandardButtons buttons READ standardButtons WRITE setStandardButtons)
    Q_PROPERTY(QDialogButtonBox::StandardButton defaultButton READ defaultButton WRITE setDefaultButton)

public:
    explicit CheckableMessageBox(QWidget *parent);
    virtual ~CheckableMessageBox() override;

    static auto question(QWidget *parent, const QString &title, const QString &question, const QString &checkBoxText,
                         bool *checkBoxSetting,
                         QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Yes | QDialogButtonBox::No,
                         QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::No)
        -> QDialogButtonBox::StandardButton;

    static auto information(QWidget *parent, const QString &title, const QString &text, const QString &checkBoxText,
                            bool *checkBoxSetting, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok,
                            QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton)
        -> QDialogButtonBox::StandardButton;

    static auto doNotAskAgainQuestion(QWidget *parent, const QString &title, const QString &text, QSettings *settings,
                                      const QString &settingsSubKey,
                                      QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Yes |
                                                                                  QDialogButtonBox::No,
                                      QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::No,
                                      QDialogButtonBox::StandardButton acceptButton = QDialogButtonBox::Yes)
        -> QDialogButtonBox::StandardButton;

    static auto doNotShowAgainInformation(QWidget *parent, const QString &title, const QString &text,
                                          QSettings *settings, const QString &settingsSubKey,
                                          QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok,
                                          QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton)
        -> QDialogButtonBox::StandardButton;

    auto text() const -> QString;
    void setText(const QString &);

    auto isChecked() const -> bool;
    void setChecked(bool s);

    auto checkBoxText() const -> QString;
    void setCheckBoxText(const QString &);

    auto isCheckBoxVisible() const -> bool;
    void setCheckBoxVisible(bool);

    auto standardButtons() const -> QDialogButtonBox::StandardButtons;
    void setStandardButtons(QDialogButtonBox::StandardButtons s);
    auto button(QDialogButtonBox::StandardButton b) const -> QPushButton *;
    auto addButton(const QString &text, QDialogButtonBox::ButtonRole role) -> QPushButton *;

    auto defaultButton() const -> QDialogButtonBox::StandardButton;
    void setDefaultButton(QDialogButtonBox::StandardButton s);

    // See static QMessageBox::standardPixmap()
    auto iconPixmap() const -> QPixmap;
    void setIconPixmap (const QPixmap &p);

    // Query the result
    auto clickedButton() const -> QAbstractButton *;
    auto clickedStandardButton() const -> QDialogButtonBox::StandardButton;

    // Conversion convenience
    static auto dialogButtonBoxToMessageBoxButton(QDialogButtonBox::StandardButton) -> QMessageBox::StandardButton;
    static void resetAllDoNotAskAgainQuestions(QSettings *settings);
    static auto hasSuppressedQuestions(QSettings *settings) -> bool;
    static auto msgDoNotAskAgain() -> QString;
    static auto msgDoNotShowAgain() -> QString;

private slots:
    void slotClicked(QAbstractButton *b);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(CheckableMessageBox) // NOLINT
    CheckableMessageBoxPrivate *d;
    enum DoNotAskAgainType{Question, Information};

    static auto askAgain(QSettings *settings, const QString &settingsSubKey) -> bool;
    static void initDoNotAskAgainMessageBox(CheckableMessageBox &messageBox, const QString &title,
                                            const QString &text, QDialogButtonBox::StandardButtons buttons,
                                            QDialogButtonBox::StandardButton defaultButton,
                                            DoNotAskAgainType type);
    static void doNotAskAgain(QSettings *settings, const QString &settingsSubKey);
};

} // namespace Utils

#endif // CHECKABLEMESSAGEBOX_H
