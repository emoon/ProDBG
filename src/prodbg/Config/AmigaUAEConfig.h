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

private:
    Q_SLOT void selectExecutable();
    Q_SLOT void selectConfigFile();
    Q_SLOT void selectDh0Path();

    void writeSettings();
    void readSettings();

    Ui::AmigaUAEConfig* m_ui;
};
