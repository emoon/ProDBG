#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AmigaUAE : public QObject
{
    Q_OBJECT

public:
    AmigaUAE(QObject* parent);
    ~AmigaUAE();

    void runExecutable(const QString& filename);
    bool validateSettings();
    void launchUAE();

    QProcess* m_uaeProcess;

private:
    Q_SLOT void started();
    Q_SLOT void errorOccurred(QProcess::ProcessError error);

    void readSettings();

    bool m_running = false;

    QString m_uaeExe;
    QString m_config;
    QString m_cmdLineArgs;
    QString m_dh0Path;
    QString m_fileToRun;
    bool m_copyFiles;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
