/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/

/*!
\class QxtCsvModel
\brief The QxtCsvModel class provides a QAbstractTableModel for CSV Files
 */

#include "qxtcsvmodel.h"

#include <QFile>
#include <QIODevice>
#include <QList>
#include <QTextStream>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "../vmisc/vtextcodec.h"
#else
#include <QTextCodec>
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

class QxtCsvModelPrivate : public QxtPrivate<QxtCsvModel>
{
public:
    QxtCsvModelPrivate()
      : csvData(),
        header(),
        maxColumn(0),
        quoteMode(QxtCsvModel::DefaultQuoteMode)
    {
    }
    QXT_DECLARE_PUBLIC(QxtCsvModel)
    virtual ~QxtCsvModelPrivate() = default;

    QList<QStringList> csvData;
    QStringList header;
    vsizetype maxColumn;
    QxtCsvModel::QuoteMode quoteMode;

private:
    Q_DISABLE_COPY_MOVE(QxtCsvModelPrivate) // NOLINT
};

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

/*!
  Creates an empty QxtCsvModel with parent \a parent.
  */
QxtCsvModel::QxtCsvModel(QObject *parent)
  : QAbstractTableModel(parent){QXT_INIT_PRIVATE(QxtCsvModel)}

    /*!
      Creates a QxtCsvModel with the parent \a parent and content loaded from \a file.

      See \a setSource for information on the \a withHeader and \a separator properties, or
      if you need control over the quoting method or codec used to parse the file.

      \sa setSource
      */
    QxtCsvModel::QxtCsvModel(QIODevice * file, QObject * parent, bool withHeader, QChar separator, VTextCodec *codec)
  : QAbstractTableModel(parent)
{
    QXT_INIT_PRIVATE(QxtCsvModel)
    setSource(file, withHeader, separator, codec);
}

/*!
  \overload

  Creates a QxtCsvModel with the parent \a parent and content loaded from \a file.

  See \a setSource for information on the \a withHeader and \a separator properties, or
  if you need control over the quoting method or codec used to parse the file.

  \sa setSource
  */
QxtCsvModel::QxtCsvModel(const QString &filename, QObject *parent, bool withHeader, QChar separator, VTextCodec *codec)
  : QAbstractTableModel(parent)
{
    QXT_INIT_PRIVATE(QxtCsvModel)
    QFile src(filename);
    setSource(&src, withHeader, separator, codec);
}

QT_WARNING_POP

/*!
    \reimp
 */
auto QxtCsvModel::rowCount(const QModelIndex &parent) const -> int
{
    if (parent.row() != -1 && parent.column() != -1)
    {
        return 0;
    }
    return static_cast<int>(qxt_d().csvData.count());
}

/*!
    \reimp
 */
auto QxtCsvModel::columnCount(const QModelIndex &parent) const -> int
{
    if (parent.row() != -1 && parent.column() != -1)
    {
        return 0;
    }
    return static_cast<int>(qxt_d().maxColumn);
}

/*!
    \reimp
 */
auto QxtCsvModel::data(const QModelIndex &index, int role) const -> QVariant
{
    if (index.parent() != QModelIndex())
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole)
    {
        if (index.row() < 0 || index.column() < 0 || index.row() >= rowCount())
        {
            return QVariant();
        }
        const QStringList &row = qxt_d().csvData[index.row()];
        if (index.column() >= row.length())
        {
            return QVariant();
        }
        return row[index.column()];
    }
    return QVariant();
}

/*!
    \reimp
 */
auto QxtCsvModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if (section < qxt_d().header.count() && orientation == Qt::Horizontal &&
        (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole))
    {
        return qxt_d().header[section];
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

/*!
  \overload

  Reads in a CSV file from the provided \a file using \a codec.
  */
void QxtCsvModel::setSource(const QString &filename, bool withHeader, QChar separator, VTextCodec *codec)
{
    QFile src(filename);
    setSource(&src, withHeader, separator, codec);
}

/*!
  Reads in a CSV file from the provided \a file using \a codec.

  The value of \a separator will be used to delimit fields, subject to the specified \a quoteMode.
  If \a withHeader is set to true, the first line of the file will be used to populate the model's
  horizontal header.

  \sa quoteMode
  */
void QxtCsvModel::setSource(QIODevice *file, bool withHeader, QChar separator, VTextCodec *codec)
{
    QxtCsvModelPrivate *d_ptr = &qxt_d();
    bool headerSet = !withHeader;
    if (not file->isOpen())
    {
        file->open(QIODevice::ReadOnly);
    }
    if (withHeader)
    {
        d_ptr->maxColumn = 0;
    }
    else
    {
        d_ptr->maxColumn = d_ptr->header.size();
    }
    d_ptr->csvData.clear();
    QStringList row;
    QString field;
    QChar quote;
    QChar ch, buffer(0);
    bool readCR = false;
    QTextStream stream(file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    codec ? stream.setEncoding(codec->Encoding()) : stream.setAutoDetectUnicode(true);
#else
    codec ? stream.setCodec(codec) : stream.setAutoDetectUnicode(true);
#endif
    while (not stream.atEnd())
    {
        if (buffer != QChar(0))
        {
            ch = buffer;
            buffer = QChar(0);
        }
        else
        {
            stream >> ch;
        }
        if (ch == '\n' && readCR)
        {
            continue;
        }
        if (ch == '\r')
        {
            readCR = true;
        }
        else
        {
            readCR = false;
        }
        if (ch != separator && (ch.category() == QChar::Separator_Line || ch.category() == QChar::Separator_Paragraph ||
                                ch.category() == QChar::Other_Control))
        {
            row << field;
            field.clear();
            if (not row.isEmpty())
            {
                if (not headerSet)
                {
                    d_ptr->header = row;
                    headerSet = true;
                }
                else
                {
                    d_ptr->csvData.append(row);
                }
                if (row.length() > d_ptr->maxColumn)
                {
                    d_ptr->maxColumn = row.length();
                }
            }
            row.clear();
        }
        else if ((d_ptr->quoteMode & DoubleQuote && ch == '"') || (d_ptr->quoteMode & SingleQuote && ch == '\''))
        {
            quote = ch;
            do
            {
                stream >> ch;
                if (ch == '\\' && d_ptr->quoteMode & BackslashEscape)
                {
                    stream >> ch;
                }
                else if (ch == quote)
                {
                    if (d_ptr->quoteMode & TwoQuoteEscape)
                    {
                        stream >> buffer;
                        if (buffer == quote)
                        {
                            buffer = QChar(0);
                            field.append(ch);
                            continue;
                        }
                    }
                    break;
                }
                field.append(ch);
            } while (!stream.atEnd());
        }
        else if (ch == separator)
        {
            row << field;
            field.clear();
        }
        else
        {
            field.append(ch);
        }
    }
    if (not field.isEmpty())
    {
        row << field;
    }
    if (not row.isEmpty())
    {
        if (not headerSet)
        {
            d_ptr->header = row;
        }
        else
        {
            d_ptr->csvData.append(row);
        }
    }
    file->close();
}

/*!
  Sets the horizontal headers of the model to the values provided in \a data.
 */
void QxtCsvModel::setHeaderData(const QStringList &data)
{
    qxt_d().header = data;
    emit headerDataChanged(Qt::Horizontal, 0, static_cast<int>(data.count()));
}

/*!
  \reimp
  */
auto QxtCsvModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) -> bool
{
    if (orientation != Qt::Horizontal)
    {
        return false; // We don't support the vertical header
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        return false; // We don't support any other roles
    }

    if (section < 0)
    {
        return false; // Bogus input
    }

    while (section > qxt_d().header.size())
    {
        qxt_d().header << QString();
    }
    qxt_d().header[section] = value.toString();
    emit headerDataChanged(Qt::Horizontal, section, section);
    return true;
}

/*!
    \reimp
 */
auto QxtCsvModel::setData(const QModelIndex &index, const QVariant &data, int role) -> bool
{
    if (index.parent() != QModelIndex())
    {
        return false;
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole)
    {
        if (index.row() >= rowCount() || index.column() >= columnCount() || index.row() < 0 || index.column() < 0)
        {
            return false;
        }
        QStringList &row = qxt_d().csvData[index.row()];
        while (row.length() <= index.column())
        {
            row << QString();
        }
        row[index.column()] = data.toString();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

/*!
    \reimp
 */
auto QxtCsvModel::insertSingleRow(int row, const QModelIndex &parent) -> bool
{
    return insertRows(row, 1, parent);
}

/*!
    \reimp
 */
auto QxtCsvModel::insertRows(int row, int count, const QModelIndex &parent) -> bool
{
    if (parent != QModelIndex() || row < 0)
    {
        return false;
    }
    beginInsertRows(parent, row, row + count);
    QxtCsvModelPrivate &d_ptr = qxt_d();
    if (row >= rowCount())
    {
        for (int i = 0; i < count; i++)
        {
            d_ptr.csvData << QStringList();
        }
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            d_ptr.csvData.insert(row, QStringList());
        }
    }
    endInsertRows();
    return true;
}

/*!
    \reimp
 */
auto QxtCsvModel::removeSingleRow(int row, const QModelIndex &parent) -> bool
{
    return removeRows(row, 1, parent);
}

/*!
    \reimp
 */
auto QxtCsvModel::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    if (parent != QModelIndex() || row < 0)
    {
        return false;
    }
    if (row >= rowCount())
    {
        return false;
    }
    if (row + count >= rowCount())
    {
        count = rowCount() - row;
    }
    beginRemoveRows(parent, row, row + count);
    QxtCsvModelPrivate &d_ptr = qxt_d();
    for (int i = 0; i < count; i++)
    {
        d_ptr.csvData.removeAt(row);
    }
    endRemoveRows();
    return true;
}

/*!
    \reimp
 */
auto QxtCsvModel::insertSingleColumn(int col, const QModelIndex &parent) -> bool
{
    return insertColumns(col, 1, parent);
}

/*!
    \reimp
 */
auto QxtCsvModel::insertColumns(int col, int count, const QModelIndex &parent) -> bool
{
    if (parent != QModelIndex() || col < 0)
    {
        return false;
    }
    beginInsertColumns(parent, col, col + count - 1);
    QxtCsvModelPrivate &d_ptr = qxt_d();
    for (int i = 0; i < rowCount(); i++)
    {
        QStringList &row = d_ptr.csvData[i];
        while (col >= row.length())
        {
            row.append(QString());
        }
        for (int j = 0; j < count; j++)
        {
            row.insert(col, QString());
        }
    }
    for (int i = 0; i < count; i++)
    {
        d_ptr.header.insert(col, QString());
    }
    d_ptr.maxColumn += count;
    endInsertColumns();
    return true;
}

/*!
    \reimp
 */
auto QxtCsvModel::removeSingleColumn(int col, const QModelIndex &parent) -> bool
{
    return removeColumns(col, 1, parent);
}

/*!
    \reimp
 */
auto QxtCsvModel::removeColumns(int col, int count, const QModelIndex &parent) -> bool
{
    if (parent != QModelIndex() || col < 0)
    {
        return false;
    }
    if (col >= columnCount())
    {
        return false;
    }
    if (col + count >= columnCount())
    {
        count = columnCount() - col;
    }
    beginRemoveColumns(parent, col, col + count);
    QxtCsvModelPrivate &d_ptr = qxt_d();
    for (int i = 0; i < rowCount(); i++)
    {
        for (int j = 0; j < count; j++)
        {
            d_ptr.csvData[i].removeAt(col);
        }
    }
    for (int i = 0; i < count; i++)
    {
        d_ptr.header.removeAt(col);
    }
    endRemoveColumns();
    return true;
}

static auto qxt_addCsvQuotes(QxtCsvModel::QuoteMode mode, QString field) -> QString
{
    bool addDoubleQuotes = ((mode & QxtCsvModel::DoubleQuote) && field.contains('"'));
    bool addSingleQuotes = ((mode & QxtCsvModel::SingleQuote) && field.contains('\''));
    bool const quoteField = (mode & QxtCsvModel::AlwaysQuoteOutput) || addDoubleQuotes || addSingleQuotes;
    if (quoteField && !addDoubleQuotes && !addSingleQuotes)
    {
        if (mode & QxtCsvModel::DoubleQuote)
        {
            addDoubleQuotes = true;
        }
        else if (mode & QxtCsvModel::SingleQuote)
        {
            addSingleQuotes = true;
        }
    }
    if (mode & QxtCsvModel::BackslashEscape)
    {
        if (addDoubleQuotes)
        {
            return '"' + field.replace("\\", "\\\\").replace("\"", "\\\"") + '"';
        }
        if (addSingleQuotes)
        {
            return '\'' + field.replace("\\", "\\\\").replace("'", "\\'") + '\'';
        }
    }
    else
    {
        if (addDoubleQuotes)
        {
            return '"' + field.replace("\"", "\"\"") + '"';
        }
        if (addSingleQuotes)
        {
            return '\'' + field.replace("'", "''") + '\'';
        }
    }
    return field;
}

/*!
  Outputs the content of the model as a CSV file to the device \a dest using \a codec.

  Fields in the output file will be separated by \a separator. Set \a withHeader to true
  to output a row of headers at the top of the file.
 */
auto QxtCsvModel::toCSV(QIODevice *dest, QString &error, bool withHeader, QChar separator, VTextCodec *codec) const
    -> bool
{
    const QxtCsvModelPrivate &d_ptr = qxt_d();
    int rows = rowCount();
    int cols = columnCount();
    QString data;
    if (not dest->isOpen() && not dest->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        error = dest->errorString();
        return false;
    }
    QTextStream stream(dest);

    if (codec)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        stream.setEncoding(codec->Encoding());
#else
        stream.setCodec(codec);
#endif
    }

    if (withHeader)
    {
        data = QString();
        for (int col = 0; col < cols; ++col)
        {
            if (col > 0)
            {
                data += separator;
            }
            data += qxt_addCsvQuotes(d_ptr.quoteMode, d_ptr.header.at(col));
        }
        stream << data << Qt::endl;
    }
    for (int row = 0; row < rows; ++row)
    {
        const QStringList &rowData = d_ptr.csvData[row];
        data = QString();
        for (int col = 0; col < cols; ++col)
        {
            if (col > 0)
            {
                data += separator;
            }
            if (col < rowData.length())
            {
                data += qxt_addCsvQuotes(d_ptr.quoteMode, rowData.at(col));
            }
            else
            {
                data += qxt_addCsvQuotes(d_ptr.quoteMode, QString());
            }
        }
        stream << data << Qt::endl;
    }
    stream << Qt::flush;
    dest->close();
    return true;
}

/*!
  \overload

  Outputs the content of the model as a CSV file to the file specified by \a filename using \a codec.

  Fields in the output file will be separated by \a separator. Set \a withHeader to true
  to output a row of headers at the top of the file.
 */
auto QxtCsvModel::toCSV(const QString &filename, QString &error, bool withHeader, QChar separator,
                        VTextCodec *codec) const -> bool
{
    QFile dest(filename);
    return toCSV(&dest, error, withHeader, separator, codec);
}

/*!
    \reimp
 */
auto QxtCsvModel::flags(const QModelIndex &index) const -> Qt::ItemFlags
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

/*!
 * Returns the current quoting mode.
 * \sa setQuoteMode
 */
auto QxtCsvModel::quoteMode() const -> QxtCsvModel::QuoteMode
{
    return qxt_d().quoteMode;
}

/*!
 * Sets the current quoting mode. The default quoting mode is BothQuotes | BackslashEscape.
 *
 * The quoting mode determines what kinds of quoting is used for reading and writing CSV files.
 * \sa quoteMode
 * \sa QuoteOption
 */
void QxtCsvModel::setQuoteMode(QuoteMode mode)
{
    qxt_d().quoteMode = mode;
}

/*!
  Sets the content of the cell at row \a row and column \a column to \a value.

  \sa text
  */
void QxtCsvModel::setText(int row, int column, const QString &value)
{
    setData(index(row, column), value);
}

/*!
  Fetches the content of the cell at row \a row and column \a column.

  \sa setText
  */
auto QxtCsvModel::text(int row, int column) const -> QString
{
    return data(index(row, column)).toString();
}

/*!
  Sets the content of the header for column \a column to \a value.

  \sa headerText
  */
void QxtCsvModel::setHeaderText(int column, const QString &value)
{
    setHeaderData(column, Qt::Horizontal, value);
}

/*!
  Fetches the content of the cell at row \a row and column \a column.

  \sa setText
  */
auto QxtCsvModel::headerText(int column) const -> QString
{
    return headerData(column, Qt::Horizontal).toString();
}
