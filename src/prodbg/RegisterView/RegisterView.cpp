#include "RegisterView.h"
#include "Backend/IBackendRequests.h"
#include "ui_RegisterView.h"
#include <QDebug>
#include <stdint.h>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RegisterView::RegisterView(QWidget* parent)
    : View(parent)
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
    m_ui->m_registers->verticalHeader()->setDefaultSectionSize(m_ui->m_registers->fontMetrics().height() + 2);
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

static QString getRegisterValue(IBackendRequests::Register* reg)
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

    return regText;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QTableWidgetItem* buildRegisterValue(IBackendRequests::Register* reg)
{
    QString regText = getRegisterValue(reg);

    QTableWidgetItem* item = new QTableWidgetItem(regText);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    return item;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::endReadRegisters(QVector<IBackendRequests::Register>* registers)
{
    int i = 0;

    // TODO: Rewrite using QTableView/QAbstractItemModel

    int tableCount = m_ui->m_registers->rowCount();
    int registerCount = registers->count();

    const QBrush& defaultTextColor = QGuiApplication::palette().windowText();

    // If not the same amount of registers we just clear and instert them all

    if (tableCount != registerCount) {
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
    } else {
        // Otherwise we update here

        for (auto& reg : *registers) {
            QTableWidgetItem* value = m_ui->m_registers->item(i, 1);
            QString newValue = getRegisterValue(&reg);

            if (value->text() != newValue) {
                value->setText(newValue);
                value->setForeground(Qt::red);
            } else {
                value->setForeground(defaultTextColor);
            }

            ++i;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::programCounterChanged(const IBackendRequests::ProgramCounterChange& pc)
{
    m_interface->beginReadRegisters(&m_targetRegisters);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::interfaceSet()
{
    if (m_interface) {
        connect(m_interface, &IBackendRequests::programCounterChanged, this, &RegisterView::programCounterChanged);
        connect(m_interface, &IBackendRequests::endReadRegisters, this, &RegisterView::endReadRegisters);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
void RegisterView::readSettings()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::writeSettings()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
