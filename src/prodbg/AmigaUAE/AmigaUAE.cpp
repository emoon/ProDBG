#include "AmigaUAE.h"
#include <QSettings>
#include <QString>
#include <QMessageBox>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmigaUAE::AmigaUAE()
    : m_copyFiles(true)
    , m_uaeProcess(nullptr)
{
    (void)m_uaeProcess;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmigaUAE::~AmigaUAE()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAE::runExecutable(const QString& filename)
{
    if (!validateSettings()) {
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AmigaUAE::validateSettings()
{
    readSettings();

    if (m_uaeExe.isEmpty()) {
        QMessageBox::critical(nullptr, QStringLiteral("No UAE executable selected"), 
                QStringLiteral("In order to run Amiga executables you need to select the emulator executable in \"Config -> Amiga UAE..\""));
        return false;
    }

    if (m_config.isEmpty()) {
        QMessageBox::critical(nullptr, QStringLiteral("No UAE config selected"), 
                QStringLiteral("In order to run Amiga executables you need to select a configuration file for UAE in \"Config -> Amiga UAE..\""));
        return false;
    }

    if (m_dh0Path.isEmpty()) {
        QMessageBox::critical(nullptr, QStringLiteral("No hard drive path selected"), 
                QStringLiteral("In order to run Amiga executables you need to select the directory use for the hard drive in \"Config -> Amiga UAE..\""));
        return false;
    }

    return false;
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
