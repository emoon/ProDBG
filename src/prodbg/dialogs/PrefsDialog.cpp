#include "PrefsDialog.h"
#include "ui_PrefsDialog.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Config {
    QString name;
    QWidget* widget;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PrefsDialog::PrefsDialog(QWidget* parent) :
    QDialog(parent),
    m_ui(new Ui_PrefsDialog)
{
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));

    Config configs[] = {
        { QStringLiteral("Apperance"), nullptr },
    };

    for (unsigned i = 0; i < sizeof_array(configs); ++i) {
        QListWidgetItem* item = new QListWidgetItem(configs[i].name);
        QWidget* widget = configs[i].widget;

        //widget->setUserData(0, Qt::UserRole, i);

        m_ui->panel->addWidget(widget);
        m_ui->config_categories->addItem(item);
    }

    connect(m_ui->save_buttons, &QDialogButtonBox::accepted, this, &QDialog::close);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrefsDialog::change_layout(QListWidgetItem* curr, QListWidgetItem* prev) {
    (void)curr;
    (void)prev;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PrefsDialog::~PrefsDialog() {
    delete m_ui;
}

}

