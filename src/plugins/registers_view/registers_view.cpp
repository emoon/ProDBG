#include "registers_view.h"
#include "ui_RegisterView.h"
#include "pd_ui_register_plugin.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegistersPlugin::init(QWidget* parent) {
    m_ui = new Ui_RegisterView;
    m_ui->setupUi(parent);

#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    QStringList header;
    header << QStringLiteral("Name") << QStringLiteral("Value");

    m_ui->m_registers->setColumnCount(2);
    m_ui->m_registers->setHorizontalHeaderLabels(header);
    m_ui->m_registers->verticalHeader()->setVisible(false);
    m_ui->m_registers->setShowGrid(false);
    m_ui->m_registers->setFont(font);
    m_ui->m_registers->setStyleSheet(QStringLiteral("QTableWidget::item { padding: 0px }"));
    m_ui->m_registers->verticalHeader()->setDefaultSectionSize(m_ui->m_registers->fontMetrics().height() + 2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDView* RegistersPlugin::create(QWidget* parent) {
    RegistersPlugin* instance = new RegistersPlugin;
    instance->init(parent);
    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RegistersPlugin::~RegistersPlugin() {
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegistersPlugin::set_backend_interface(PDIBackendRequests* interface) {
    m_backend = interface;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void pd_register_view(PDRegisterViewPlugin* reg) {
    reg->register_view(new RegistersPlugin);
}

