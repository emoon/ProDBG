#include "AmigaUAE.h"
#include "Backend/IdService.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QString>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmigaUAE::AmigaUAE(QObject* parent)
    : QObject(parent)
    , m_uaeProcess(nullptr)
    , m_copyFiles(true)
{
    m_setFileId = IdService_register("AmigaUAE_SetFile");
    m_setHddPathId = IdService_register("AmigaUAE_SetHddPath");

    m_uaeProcess = new QProcess();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmigaUAE::~AmigaUAE()
{
    m_uaeProcess->kill();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AmigaUAE::openFile()
{
    QString path = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Select Amiga executable to debug"));

    if (path.isEmpty()) {
        return false;
    }

    m_localExeToRun = path;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAE::runExecutable(const QString& filename)
{
    if (!validateSettings()) {
        return;
    }

    launchUAE();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AmigaUAE::validateSettings()
{
    readSettings();

    if (m_uaeExe.isEmpty()) {
        QMessageBox::critical(nullptr, QStringLiteral("No UAE executable selected"),
                              QStringLiteral("In order to run Amiga executables you need to select the emulator "
                                             "executable in \"Config -> Amiga UAE..\""));
        return false;
    }

    if (m_config.isEmpty()) {
        QMessageBox::critical(nullptr, QStringLiteral("No UAE config selected"),
                              QStringLiteral("In order to run Amiga executables you need to select a configuration "
                                             "file for UAE in \"Config -> Amiga UAE..\""));
        return false;
    }

    if (m_dh0Path.isEmpty()) {
        QMessageBox::critical(nullptr, QStringLiteral("No hard drive path selected"),
                              QStringLiteral("In order to run Amiga executables you need to select the directory use "
                                             "for the hard drive in \"Config -> Amiga UAE..\""));
        return false;
    }

    QFileInfo fileInfo(m_localExeToRun);
    QString filename = fileInfo.fileName();

    // If we aren't going to copy file we have to make sure the path to the file is located in the output
    // hdd directory

    if (!m_copyFiles) {
        if (m_localExeToRun.indexOf(m_dh0Path) != 0) {
            QMessageBox::critical(
                nullptr, QStringLiteral("Executable not peresent in path"),
                QStringLiteral("The executable you are trying to run isn't present in the HDD path."
                               "Either select correct path or the 'Copy files' option in \"Config -> Amiga UAE..\""));
            return false;
        }
    } else {
        QFileInfo fileInfo(m_localExeToRun);
        QString filename = fileInfo.fileName();
        QString fileDest = QDir::cleanPath(m_dh0Path + QDir::separator() + filename);

        if (!QFile::copy(m_localExeToRun, fileDest)) {
            QString error = QStringLiteral("Failed to copy file ");
            error += m_localExeToRun;
            error += QStringLiteral(" to ");
            error += fileDest;
            error += QStringLiteral(" . Make sure file is readable/directory is writeable/etc ");

            QMessageBox::critical(nullptr, QStringLiteral("Failed to copy file"), error);

            return false;
        }
    }

    m_fileToRun = QStringLiteral("dh0:") + filename;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAE::launchUAE()
{
    QStringList args;

    m_running = false;

    connect(m_uaeProcess, &QProcess::started, this, &AmigaUAE::started);
    connect(m_uaeProcess, &QProcess::errorOccurred, this, &AmigaUAE::errorOccurred);

    args << m_config;
    args << QStringLiteral("--remote-debugger=1");
    args << m_cmdLineArgs.split(QLatin1Char(' '));

    m_uaeProcess->start(m_uaeExe, args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAE::started()
{
    m_running = true;
    printf("started uae ok!\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAE::errorOccurred(QProcess::ProcessError error)
{
    (void)error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAE::readSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    settings.beginGroup(QStringLiteral("AmigaUAEConfig"));
    m_uaeExe = settings.value(QStringLiteral("executablePath")).toString();
    m_config = settings.value(QStringLiteral("configPath")).toString();
    m_cmdLineArgs = settings.value(QStringLiteral("cmdlineArgs")).toString();
    m_dh0Path = settings.value(QStringLiteral("dh0Path")).toString();
    m_copyFiles = settings.value(QStringLiteral("copyFilesToHDD")).toBool();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
