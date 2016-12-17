#include "AmigaUAEConfig.h"
#include "ui_AmigaUAEConfig.h"
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmigaUAEConfig::AmigaUAEConfig(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::AmigaUAEConfig)
{
    m_ui->setupUi(this);

    m_ui->configMode->addItem(QStringLiteral("Automatic (Fastest startup) - FS-UAE"));
    m_ui->configMode->addItem(QStringLiteral("Automatic (Fastest startup) - WinUAE"));
    //m_ui->configMode->addItem(QStringLiteral("Automatic (A500)"));
    //m_ui->configMode->addItem(QStringLiteral("Automatic (A1200)"));
    m_ui->configMode->addItem(QStringLiteral("Manual"));

    connect(m_ui->selectExe, &QPushButton::clicked, this, &AmigaUAEConfig::selectExecutable);
    connect(m_ui->selectConfig, &QPushButton::clicked, this, &AmigaUAEConfig::selectConfigFile);
    connect(m_ui->selectDh0Path, &QPushButton::clicked, this, &AmigaUAEConfig::selectDh0Path);
    connect(m_ui->selectROM, &QPushButton::clicked, this, &AmigaUAEConfig::selectRomFile);

    connect(m_ui->configMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AmigaUAEConfig::configModeChanged);

    readSettings();

    updateMode();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmigaUAEConfig::~AmigaUAEConfig()
{
    writeSettings();
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void selectFilename(const QString& title, QLineEdit* line)
{
    QString path = QFileDialog::getOpenFileName(nullptr, title, line->text());

    if (!path.isEmpty()) {
        line->setText(path);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAEConfig::selectExecutable()
{
    selectFilename(QStringLiteral("Select UAE executable"), m_ui->uaeExe);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAEConfig::selectRomFile()
{
    selectFilename(QStringLiteral("Select Amiga ROM file"), m_ui->romPath);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAEConfig::selectConfigFile()
{
    selectFilename(QStringLiteral("Select UAE configuration"), m_ui->configPath);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAEConfig::selectDh0Path()
{
    QString path = QFileDialog::getExistingDirectory(this, QStringLiteral("Select dh0 directory"), m_ui->dh0Path->text());

    if (path.isEmpty()) {
        return;
    }

    m_ui->dh0Path->setText(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAEConfig::configModeChanged(int)
{
    updateMode();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAEConfig::updateMode()
{
    const int index = m_ui->configMode->currentIndex();

    if (index == ConfigMode_Manual) {
        m_ui->dh0Path->setEnabled(true);
        m_ui->copyFilesToHdd->setEnabled(true);
        m_ui->configPath->setEnabled(true);
        m_ui->skipUAELaunch->setEnabled(true);
        m_ui->selectConfig->setEnabled(true);
        m_ui->selectDh0Path->setEnabled(true);
        m_ui->romPath->setEnabled(false);
        m_ui->selectROM->setEnabled(false);
    } else {
        m_ui->dh0Path->setEnabled(false);
        m_ui->copyFilesToHdd->setEnabled(false);
        m_ui->configPath->setEnabled(false);
        m_ui->skipUAELaunch->setEnabled(false);
        m_ui->selectConfig->setEnabled(false);
        m_ui->selectDh0Path->setEnabled(false);
        m_ui->romPath->setEnabled(true);
        m_ui->selectROM->setEnabled(true);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAEConfig::writeSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    settings.beginGroup(QStringLiteral("AmigaUAEConfig"));
    settings.setValue(QStringLiteral("executablePath"), m_ui->uaeExe->text());
    settings.setValue(QStringLiteral("configPath"), m_ui->configPath->text());
    settings.setValue(QStringLiteral("cmdlineArgs"), m_ui->cmdlineArgs->text());
    settings.setValue(QStringLiteral("dh0Path"), m_ui->dh0Path->text());
    settings.setValue(QStringLiteral("copyFilesToHDD"), m_ui->copyFilesToHdd->isChecked());
    settings.setValue(QStringLiteral("skipUAELaunch"), m_ui->skipUAELaunch->isChecked());
    settings.setValue(QStringLiteral("configMode"), m_ui->configMode->currentIndex());
    settings.setValue(QStringLiteral("romPath"), m_ui->romPath->text());
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmigaUAEConfig::readSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    settings.beginGroup(QStringLiteral("AmigaUAEConfig"));
    m_ui->uaeExe->setText(settings.value(QStringLiteral("executablePath")).toString());
    m_ui->configPath->setText(settings.value(QStringLiteral("configPath")).toString());
    m_ui->cmdlineArgs->setText(settings.value(QStringLiteral("cmdlineArgs")).toString());
    m_ui->dh0Path->setText(settings.value(QStringLiteral("dh0Path")).toString());
    m_ui->copyFilesToHdd->setChecked(settings.value(QStringLiteral("copyFilesToHDD")).toBool());
    m_ui->skipUAELaunch->setChecked(settings.value(QStringLiteral("skipUAELaunch")).toBool());
    m_ui->configMode->setCurrentIndex(settings.value(QStringLiteral("configMode")).toInt());
    m_ui->romPath->setText(settings.value(QStringLiteral("romPath")).toString());
    settings.endGroup();

}
