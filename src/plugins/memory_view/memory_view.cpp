#include "memory_view.h"
#include <QtCore/QDebug>
#include <QtCore/QMetaEnum>
#include <QtCore/QSettings>
#include "backend/backend_requests_interface.h"
#include "ui_memory_view.h"
#include "pd_ui_register_plugin.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename EnumType> static void enum_to_combo(QComboBox* combo, int start_value) {
    QMetaEnum me = QMetaEnum::fromType<EnumType>();

    for (int i = 0, e = me.keyCount(); i < e; ++i) {
        combo->addItem(QString::fromUtf8(me.key(i)));
    }

    combo->setCurrentIndex(start_value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HexView::init(QWidget* parent) {
    m_ui = new Ui_MemoryView;
    m_ui->setupUi(parent);

    connect(m_ui->m_address, &QLineEdit::returnPressed, this, &HexView::jump_address_changed);

    enum_to_combo<MemoryViewWidget::Endianess>(m_ui->m_endianess, m_ui->m_view->endianess());
    enum_to_combo<MemoryViewWidget::DataType>(m_ui->m_type, m_ui->m_view->data_type());

    connect(m_ui->m_endianess, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &HexView::endian_changed);
    connect(m_ui->m_type, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &HexView::data_type_changed);

    QIntValidator* count_validator = new QIntValidator(1, 64, this);
    m_ui->m_count->setValidator(count_validator);

    connect(m_ui->m_count, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &HexView::count_changed);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDMemoryView* HexView::create(QWidget* parent) {
    HexView* instance = new HexView;
    instance->init(parent);
    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HexView::~HexView() {
    //writeSettings();
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HexView::endian_changed(int e) {
    m_ui->m_view->set_endianess(static_cast<MemoryViewWidget::Endianess>(e));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HexView::data_type_changed(int t) {
    m_ui->m_view->set_data_type(static_cast<MemoryViewWidget::DataType>(t));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HexView::jump_address_changed() {
    jump_to_address_expression(m_ui->m_address->text());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HexView::jump_to_address_expression(const QString& str) {
    /*
    if (m_interface) {
        m_interface->beginResolveAddress(str, &m_evalAddress);
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HexView::end_resolve_address(uint64_t* out) {
    if (!out) {
        m_ui->m_view->set_expression_status(false);
    } else {
        m_ui->m_view->set_expression_status(true);
        m_ui->m_address->setText(QStringLiteral("0x") + QString::number(*out, 16));
        m_ui->m_view->set_address(*out);
    }

    m_ui->m_view->update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HexView::set_backend_interface(PDIBackendRequests* interface) {
    m_ui->m_view->set_backend_interface(interface);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HexView::count_changed(int index) {
    bool ok = false;
    int count = m_ui->m_count->itemText(index).toInt(&ok, /*base:*/ 0);
    if (ok) {
        m_ui->m_view->set_elements_per_line(count);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void HexView::read_settings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    // TODO: Support more than one memory view
    settings.beginGroup(QStringLiteral("HexView_0"));
    m_ui->m_endianess->setCurrentIndex(settings.value(QStringLiteral("endian")).toInt());
    m_ui->m_type->setCurrentIndex(settings.value(QStringLiteral("data_type")).toInt());
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HexView::write_settings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    // TODO: Support more than one memory view
    settings.beginGroup(QStringLiteral("HexView_0"));
    settings.setValue(QStringLiteral("endian"), m_ui->m_endianess->currentIndex());
    settings.setValue(QStringLiteral("data_type"), m_ui->m_type->currentIndex());
    settings.setValue(QStringLiteral("count"), m_ui->m_type->currentIndex());
    settings.endGroup();
}
*/


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void pd_register_view(PDRegisterViewPlugin* reg) {
    reg->register_memory_view(new HexView);
}

