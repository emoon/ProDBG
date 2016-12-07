#include "RegisterView.h"
#include "Backend/IBackendRequests.h"
#include "Ui_RegisterView.h"
#include <QDebug>
#include <stdint.h>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RegisterView::RegisterView(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui_RegisterView)
{
    m_ui->setupUi(this);

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
    m_ui->m_registers->verticalHeader()->setDefaultSectionSize( 15 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RegisterView::~RegisterView()
{
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint16_t getU16(uint8_t* ptr)
{
    uint16_t v = (ptr[0] << 8) | ptr[1];
    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t getU32(uint8_t* ptr)
{
    uint32_t v = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint64_t getU64(uint8_t* ptr)
{
    uint64_t v = ((uint64_t)ptr[0] << 56) | ((uint64_t)ptr[1] << 48) | ((uint64_t)ptr[2] << 40) | 
                 ((uint64_t)ptr[3] << 32) | (ptr[4] << 24) | (ptr[5] << 16) | (ptr[6] << 8) | ptr[7];
    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QTableWidgetItem* buildRegisterValue(IBackendRequests::Register* reg) 
{
    QString regText;

    // This code is somewhat temporary but should be good initially

    switch (reg->data.count()) {
        case 1: {
            regText.sprintf("%04x", reg->data.at(0));
            break;
        }

        case 2: {
            regText.sprintf("%04x", getU16((uint8_t*)reg->data.data()));
            break;
        }

        case 4: {
            regText.sprintf("%08x", getU32((uint8_t*)reg->data.data()));
            break;
        }

        case 8: {
            regText.sprintf("%016llx", getU64((uint8_t*)reg->data.data()));
            break;
        }

        default: {
            regText = QStringLiteral("Unsupported register size");
            break;
        }
    }

    QTableWidgetItem* item = new QTableWidgetItem(regText);
    item->setFlags(Qt::ItemIsEnabled);

    return item;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::endReadRegisters(QVector<IBackendRequests::Register>* registers)
{
    int i = 0;

    m_ui->m_registers->clearContents();
    m_ui->m_registers->setRowCount(registers->count());

    for (auto& reg : *registers) {
        QTableWidgetItem* item = new QTableWidgetItem(reg.name);
        QTableWidgetItem* value = buildRegisterValue(&reg);

        item->setFlags(Qt::ItemIsEnabled);

        m_ui->m_registers->setItem(i + 0, 0, item);
        m_ui->m_registers->setItem(i + 0, 1, value);

        ++i;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::programCounterChanged(uint64_t) {
    m_interface->beginReadRegisters(&m_targetRegisters);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::setBackendInterface(IBackendRequests* interface)
{
    m_interface = interface;
    connect(m_interface, &IBackendRequests::programCounterChanged, this, &RegisterView::programCounterChanged);
    connect(m_interface, &IBackendRequests::endReadRegisters, this, &RegisterView::endReadRegisters);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
