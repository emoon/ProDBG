#include "registers_view.h"
<<<<<<< HEAD
#include "ui_RegisterView.h"

void RegistersPlugin::create(QWidget* parent) {
    m_ui = new Ui_RegisterView;

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

    // TODO: is this correct?
    m_ui->m_registers->setParent(parent);

    //printf("RegistersPlugin %d\n", t);
}

void RegistersPlugin::set_backend_interface(prodbg::IBackendRequests* interface) {
    m_backend = interface;
}
=======

void RegistersPlugin::test(int t) {
    printf("RegistersPlugin %d\n", t);
}

>>>>>>> 8b9929dcf62565f0e6b23d15f245e4c7afb83f65


