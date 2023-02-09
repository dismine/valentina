/************************************************************************
 **
 **  @file   dialogmeasurementscsvcolumns.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 10, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#ifndef DIALOGMEASUREMENTSCSVCOLUMNS_H
#define DIALOGMEASUREMENTSCSVCOLUMNS_H

#include <QDialog>

#include "../vformat/vdimensions.h"

class QxtCsvModel;
class QTextCodec;

enum class IndividualMeasurementsColumns: qint8
{
    Name = 0,
    Value = 1,
    FullName = 2,   // optional
    Description = 3, // optional
    LAST_DO_NOT_USE = 4
};

enum class MultisizeMeasurementsColumns: qint8
{
    Name = 0,
    BaseValue = 1,
    ShiftA = 2,
    ShiftB = 3,      // optional if not required
    ShiftC = 4,      // optional if not required
    FullName = 5,    // optional
    Description = 6, // optional
    LAST_DO_NOT_USE = 7
};


namespace Ui {
class DialogMeasurementsCSVColumns;
}

class DialogMeasurementsCSVColumns : public QDialog
{
    Q_OBJECT // NOLINT

public:
    DialogMeasurementsCSVColumns(const QString &filename, MeasurementsType type, QWidget *parent = nullptr);
    DialogMeasurementsCSVColumns(const QString &filename, MeasurementsType type,
                                 const QList<MeasurementDimension_p> &dimensions, QWidget *parent = nullptr);
    ~DialogMeasurementsCSVColumns() override;

    auto ColumnsMap() const -> QVector<int>;

    void SetWithHeader(bool withHeader);
    void SetSeparator(const QChar &separator);
    void SetCodec(QTextCodec *codec);

protected:
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void ColumnChanged();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogMeasurementsCSVColumns) // NOLINT
    Ui::DialogMeasurementsCSVColumns *ui;
    bool m_isInitialized{false};
    QString m_fileName;
    bool m_withHeader{false};
    QChar m_separator{','};
    QTextCodec *m_codec{nullptr};
    QVector<int> m_columnsMap{};
    MeasurementsType m_type;
    QList<MeasurementDimension_p> m_dimensions{};
    QVector<QObject *> m_hackedWidgets{};

    auto ColumnMandatory(int column) const -> bool;
    auto ColumnHeader(int column) const -> QString;
    auto ImportColumnCount() const -> int;
    auto MinimumColumns() const -> int;
    auto ColumnsValid() -> bool;
    void ClearColumnCollor();

    void InitColumnsMap();
    void InitColumnsControls();
    void InitImportHeaders();

    auto CSVModel() const -> QSharedPointer<QxtCsvModel>;

    void ShowInputPreview();
    void ShowImportPreview();

    template <class T>
    void HackWidget(T **widget);
    void HackColumnControls();

    void RetranslateLabels();

    void SetDefaultColumns();

    void CheckStatus();

    template <class T>
    auto ColumnValid(T column) const -> bool;

    template <class T>
    void SaveColum(QComboBox *control, T column);
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogMeasurementsCSVColumns::ColumnsMap() const -> QVector<int>
{
    return m_columnsMap;
}

//---------------------------------------------------------------------------------------------------------------------
inline void DialogMeasurementsCSVColumns::SetWithHeader(bool withHeader)
{
    m_withHeader = withHeader;
}

//---------------------------------------------------------------------------------------------------------------------
inline void DialogMeasurementsCSVColumns::SetSeparator(const QChar &separator)
{
    m_separator = separator;
}

//---------------------------------------------------------------------------------------------------------------------
inline void DialogMeasurementsCSVColumns::SetCodec(QTextCodec *codec)
{
    m_codec = codec;
}

#endif // DIALOGMEASUREMENTSCSVCOLUMNS_H
