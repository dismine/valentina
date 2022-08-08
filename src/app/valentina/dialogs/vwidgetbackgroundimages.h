/************************************************************************
 **
 **  @file   vwidgetbackgroundimages.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef VWIDGETBACKGROUNDIMAGES_H
#define VWIDGETBACKGROUNDIMAGES_H

#include <QWidget>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)

#include "../vmisc/def.h"

class VAbstractPattern;
class VBackgroundPatternImage;

namespace Ui
{
    class VWidgetBackgroundImages;
}

enum class ScaleUnit {Percent, Mm, Cm, Inch, Px};

class VWidgetBackgroundImages : public QWidget
{
    Q_OBJECT

public:
    explicit VWidgetBackgroundImages(VAbstractPattern *doc, QWidget *parent = nullptr);
    ~VWidgetBackgroundImages() override;

signals:
    void DeleteImage(const QUuid &id);
    void SelectImage(const QUuid &id);

public slots:
    void UpdateImages();
    void UpdateImage(const QUuid &id);
    void ImageSelected(const QUuid &id);

private slots:
    void ImageHoldChanged(int row, int column);
    void ImageVisibilityChanged(int row, int column);
    void ImageNameChanged(int row, int column);
    void ContextMenu(const QPoint &pos);
    void CurrentImageChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void MoveImageOnTop();
    void MoveImageUp();
    void MoveImageDown();
    void MoveImageBottom();
    void ApplyImageTransformation();
    void ResetImageTransformationSettings();
    void RelativeTranslationChanged(bool checked);
    void ScaleProportionallyChanged(bool checked);
    void ScaleWidthChanged(double value);
    void ScaleHeightChanged(double value);
    void ImagePositionChanged(const QUuid &id);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VWidgetBackgroundImages)
    Ui::VWidgetBackgroundImages *ui;
    VAbstractPattern *m_doc;

    Unit m_oldImageTranslationUnit{Unit::Mm};
    ScaleUnit m_oldImageScaleUnit{ScaleUnit::Percent};

    void FillTable(const QVector<VBackgroundPatternImage> &images);

    void ToggleImageHold(const QUuid &id) const;
    void ToggleImageVisibility(const QUuid &id) const;

    Q_REQUIRED_RESULT auto ImageRow(const QUuid &id) const -> int;

    Q_REQUIRED_RESULT auto CurrentTranslateUnit() const -> Unit;
    Q_REQUIRED_RESULT auto CurrentScaleUnit() const -> ScaleUnit;
    void InitImageTranslation();

    Q_REQUIRED_RESULT auto ImageWidth() const -> qreal;
    Q_REQUIRED_RESULT auto ImageHeight() const -> qreal;

    Q_REQUIRED_RESULT auto WidthScaleUnitConvertor(qreal value, ScaleUnit from, ScaleUnit to) const -> qreal;
    Q_REQUIRED_RESULT auto HeightScaleUnitConvertor(qreal value, ScaleUnit from, ScaleUnit to) const -> qreal;

    void SetAbsolutePisition(const QUuid &id);
};

#endif // VWIDGETBACKGROUNDIMAGES_H
