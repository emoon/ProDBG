#pragma once

#include <QDialog>

namespace Ui {
class AmigaUAEConfig;
}

class AmigaUAEConfig : public QDialog
{
    Q_OBJECT

public:
    explicit AmigaUAEConfig(QWidget* parent = 0);
    ~AmigaUAEConfig();

private:
    Ui::AmigaUAEConfig* m_ui;
};
