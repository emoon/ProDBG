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

    // TODO: Structure this better

    QProcess* m_uaeProcess;
    uint16_t m_setFileId;
    uint16_t m_setHddPathId;
    QString m_uaeExe;
    QString m_config;
    QString m_cmdLineArgs;
    QString m_dh0Path;
    QString m_fileToRun;
    bool m_copyFiles;

private:
    Q_SLOT void started();
    Q_SLOT void errorOccurred(QProcess::ProcessError error);

    void readSettings();

    bool m_running = false;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
