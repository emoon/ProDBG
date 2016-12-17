#pragma once

#include <QDialog>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Ui {
    class AmigaUAEConfig;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AmigaUAEConfig : public QDialog
{
    Q_OBJECT

public:
    explicit AmigaUAEConfig(QWidget* parent = 0);
    ~AmigaUAEConfig();

    enum ConfigMode
    {
        ConfigMode_Auto_Fastest_FsUAE,
        ConfigMode_Auto_Fastest_WinUAE,
        //ConfigMode_Auto_A500,
        //ConfigMode_Auto_A1200,
        ConfigMode_Manual,
    };

private:
    
    Q_SLOT void configModeChanged(int index);
    Q_SLOT void selectExecutable();
    Q_SLOT void selectConfigFile();
    Q_SLOT void selectRomFile();
    Q_SLOT void selectDh0Path();

    void writeSettings();
    void readSettings();
    void updateMode();

    Ui::AmigaUAEConfig* m_ui;
};
