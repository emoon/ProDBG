#include "prefs_dialog.h"
#include "appearance_widget.h"
#include "ui_prefs_dialog.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Config {
    QString name;
    QWidget* widget;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PrefsDialog::PrefsDialog(QWidget* parent)
    : QDialog(parent), m_ui(new Ui_PrefsDialog) {
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));

    Config configs[] = {
        {QStringLiteral("Apperance"), new AppearanceWidget(this)},
    };

    QListWidgetItem* first_item = nullptr;

    for (unsigned i = 0; i < sizeof_array(configs); ++i) {
        QListWidgetItem* item = new QListWidgetItem(configs[i].name);

        if (i == 0) {
            first_item = item;
        }

        QWidget* widget = configs[i].widget;

        item->setData(Qt::UserRole, i);

        m_ui->panel->addWidget(widget);
        m_ui->config_categories->addItem(item);
    }

    m_ui->config_categories->setCurrentItem(first_item);

    connect(m_ui->save_buttons, &QDialogButtonBox::accepted, this,
            &QDialog::close);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrefsDialog::change_layout(QListWidgetItem* curr, QListWidgetItem* prev) {
    if (!curr) curr = prev;

    int index = curr->data(Qt::UserRole).toInt();

    if (index) {
        m_ui->panel->setCurrentIndex(index - 1);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PrefsDialog::~PrefsDialog() { delete m_ui; }

}  // namespace prodbg
