#include "appearance_widget.h"
#include "../config/config.h"
#include "ui_appearance_widget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AppearanceWidget::AppearanceWidget(QWidget* parent)
    : QDialog(parent), m_ui(new Ui_AppearanceWidget) {
    m_ui->setupUi(this);

    auto config = Config::instance();
    auto themes = config->theme_list();

    m_ui->theme_combo->clear();

    for (auto& theme : themes) {
        m_ui->theme_combo->addItem(theme);
    }

    m_ui->theme_combo->setCurrentIndex(Config::instance()->current_theme());
    m_ui->theme_combo->setMaxVisibleItems(10);

    connect(m_ui->theme_combo,
            QOverload<int>::of(&QComboBox::currentIndexChanged), config,
            &Config::load_theme);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AppearanceWidget::~AppearanceWidget() { delete m_ui; }

