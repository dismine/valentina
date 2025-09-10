/************************************************************************
 **
 **  @file   dialogknownmeasurementscsvcolumns.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 11, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef DIALOGKNOWNMEASUREMENTSCSVCOLUMNS_H
#define DIALOGKNOWNMEASUREMENTSCSVCOLUMNS_H

#include <QDialog>
#include <memory>

class QxtCsvModel;
class QComboBox;
class QTextCodec;

enum class KnownMeasurementsColumns : qint8
{
    Name = 0,
    Group = 1,       // optional
    FullName = 2,    // optional
    Formula = 3,     // optional
    Description = 4, // optional
    LAST_DO_NOT_USE = 5
};

namespace Ui
{
class DialogKnownMeasurementsCSVColumns;
}

class DialogKnownMeasurementsCSVColumns : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogKnownMeasurementsCSVColumns(QString filename, QWidget *parent = nullptr);
    ~DialogKnownMeasurementsCSVColumns() override;

    auto ColumnsMap() const -> QVector<int>;

    void SetWithHeader(bool withHeader);
    void SetSeparator(const QChar &separator);
    void SetCodec(QTextCodec *codec);

protected:
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void ColumnChanged();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(DialogKnownMeasurementsCSVColumns) // NOLINT
    std::unique_ptr<Ui::DialogKnownMeasurementsCSVColumns> ui;
    bool m_isInitialized{false};
    QString m_fileName;
    bool m_withHeader{false};
    QChar m_separator{','};
    QTextCodec *m_codec{nullptr};
    QVector<int> m_columnsMap{};

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

    void RetranslateLabels();

    void SetDefaultColumns();

    void CheckStatus();

    template <class T> auto ColumnValid(T column) const -> bool;

    template <class T> void SaveColum(QComboBox *control, T column);
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogKnownMeasurementsCSVColumns::ColumnsMap() const -> QVector<int>
{
    return m_columnsMap;
}

//---------------------------------------------------------------------------------------------------------------------
inline void DialogKnownMeasurementsCSVColumns::SetWithHeader(bool withHeader)
{
    m_withHeader = withHeader;
}

//---------------------------------------------------------------------------------------------------------------------
inline void DialogKnownMeasurementsCSVColumns::SetSeparator(const QChar &separator)
{
    m_separator = separator;
}

#endif // DIALOGKNOWNMEASUREMENTSCSVCOLUMNS_H
