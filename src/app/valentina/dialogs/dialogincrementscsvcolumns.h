/************************************************************************
 **
 **  @file   dialogincrementscsvcolumns.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 10, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef DIALOGINCREMENTSCSVCOLUMNS_H
#define DIALOGINCREMENTSCSVCOLUMNS_H

#include <QDialog>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#ifdef WITH_TEXTCODEC
class QTextCodec;
#else
class VTextCodec;
using QTextCodec = VTextCodec;
#endif // WITH_TEXTCODEC
#else
class QTextCodec;
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

enum class IncrementsColumns : qint8
{
    Name = 0,
    Value = 1,
    Description = 2, // optional
    LAST_DO_NOT_USE = 3
};

class QComboBox;
class QxtCsvModel;

namespace Ui
{
class DialogIncrementsCSVColumns;
}

class DialogIncrementsCSVColumns : public QDialog
{
    Q_OBJECT // NOLINT

public:
    explicit DialogIncrementsCSVColumns(const QString &filename, QWidget *parent = nullptr);
    ~DialogIncrementsCSVColumns() override;

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
    Q_DISABLE_COPY_MOVE(DialogIncrementsCSVColumns)
    Ui::DialogIncrementsCSVColumns *ui;
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

    template<class T>
    auto ColumnValid(T column) const -> bool;

    template<class T>
    void SaveColum(QComboBox *control, T column);
};

//---------------------------------------------------------------------------------------------------------------------
inline auto DialogIncrementsCSVColumns::ColumnsMap() const -> QVector<int>
{
    return m_columnsMap;
}

//---------------------------------------------------------------------------------------------------------------------
inline void DialogIncrementsCSVColumns::SetWithHeader(bool withHeader)
{
    m_withHeader = withHeader;
}

//---------------------------------------------------------------------------------------------------------------------
inline void DialogIncrementsCSVColumns::SetSeparator(const QChar &separator)
{
    m_separator = separator;
}

#endif // DIALOGINCREMENTSCSVCOLUMNS_H
