#ifndef VPUZZLECOMMANDLINE_H
#define VPUZZLECOMMANDLINE_H

#include <memory>
#include <QCoreApplication>
#include <QCommandLineParser>

class VPuzzleCommandLine;
using VPuzzleCommandLinePtr = std::shared_ptr<VPuzzleCommandLine>;

class VPuzzleCommandLine: public QObject
{
    Q_OBJECT
public:   
    virtual ~VPuzzleCommandLine() = default;

    /** @brief if user enabled export from cmd */
    bool IsExportEnabled() const;

    /** @brief path to export file or empty string if not */
    QString OptionExportFile() const;

    /** @brief list with paths to the raw layout data files */
    QStringList OptionRawLayouts() const;

    /** @brief if user enabled test mode from cmd */
    bool IsTestModeEnabled() const;

    /** @brief if gui enabled or not */
    bool IsGuiEnabled() const;

    /** @brief the file name which should be loaded */
    QStringList OptionFileNames() const;

    /** @brief if high dpi scaling is enabled */
    bool IsNoScalingEnabled() const;
protected:
    VPuzzleCommandLine();

    /** @brief create the single instance of the class inside puzzleapplication */
    static VPuzzleCommandLinePtr Instance(const QCoreApplication &app); 
    static void ProcessInstance(VPuzzleCommandLinePtr &instance, const QStringList &arguments);
private:
    Q_DISABLE_COPY(VPuzzleCommandLine)
    static VPuzzleCommandLinePtr instance;
    QCommandLineParser parser;
    bool isGuiEnabled;
    friend class PuzzleApplication;

    /** @brief add options to the QCommandLineParser that there are in the cmd can be */
    void InitCommandLineOptions();

    bool IsOptionSet(const QString &option)const;
    QString OptionValue(const QString &option) const;
    QStringList OptionValues(const QString &option) const;
};

#endif // VPUZZLECOMMANDLINE_H
