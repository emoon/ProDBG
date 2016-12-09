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

    connect(m_ui->selectExe, &QPushButton::released, this, &AmigaUAEConfig::selectExecutable);
    connect(m_ui->selectConfig, &QPushButton::released, this, &AmigaUAEConfig::selectConfigFile);
    connect(m_ui->selectDh0Path, &QPushButton::released, this, &AmigaUAEConfig::selectDh0Path);

    readSettings();
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
    settings.endGroup();
}
