/************************************************************************
 **
 **  @file   vnestingoptionswidget.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 October, 2026
 **
 **  @brief  
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making 
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project 
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
#ifndef VNESTINGOPTIONSWIDGET_H
#define VNESTINGOPTIONSWIDGET_H

#include "../vlayout/vnestingframe.h"

#include <functional>
#include <memory>
#include <QJsonObject>
#include <QLocale>
#include <QWidget>

namespace Ui
{
class VNestingOptionsWidget;
}

class VNestorClient;

/**
 * @brief Speed, stop conditions, token quote and free preview of an online nesting job (spec 12.3).
 * The host dialog owns the rotations and reads Options(); the token budget lives here (spec 20).
 */
class VNestingOptionsWidget : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit VNestingOptionsWidget(QWidget *parent = nullptr);
    ~VNestingOptionsWidget() override;

    /** @brief Enables the preview button; the builder returns the request to send, or an empty object. */
    void SetRequestBuilder(std::function<QJsonObject()> builder);
    auto Options() const -> VNestingOptions; // rotations are left at their default: the host sets them

    auto Speed() const -> NestingSpeed;
    void SetSpeed(NestingSpeed speed);

    /** @brief Roll lengths typed in metres ("50, 50, 32.5" or "50; 50; 32,5"), in mm; unreadable or non-positive
     * values are dropped. Separators: spaces, semicolons, and commas that are not a decimal mark of `locale`. */
    static auto ParseRollLengthsMm(const QString &text, const QLocale &locale) -> QVector<qreal>;

protected:
    void changeEvent(QEvent *event) override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VNestingOptionsWidget) // NOLINT

    std::unique_ptr<Ui::VNestingOptionsWidget> ui;
    VNestorClient *m_client;
    std::function<QJsonObject()> m_builder{};
    int m_previewTimeS{10};
    QList<int> m_workers{2, 4, 6}; // threads per speed, indexed by NestingSpeed, until /health answers
    QList<qreal> m_evalsPerS{1500000, 3000000, 4000000}; // the service's measured rate per speed (spec 20)
    QList<int> m_evalsPerToken{1500000, 1000000, 1000000};

    void FillSpeeds();
    void UpdateQuote();
    void UpdatePreviewButton();
    void ShowRolls();
    void StartPreview();
    void ShowPreview(const QJsonObject &request, const QJsonObject &result);
};

#endif // VNESTINGOPTIONSWIDGET_H
