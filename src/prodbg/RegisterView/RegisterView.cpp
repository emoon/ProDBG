#include "RegisterView.h"
#include "Backend/IBackendRequests.h"
#include "Ui_RegisterView.h"
#include <QDebug>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RegisterView::RegisterView(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui_RegisterView)
{
    m_ui->setupUi(this);
    connect(m_ui->pushButton, &QPushButton::released, this, &RegisterView::getSomeData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RegisterView::~RegisterView()
{
    delete m_ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::dataTypeChanged(int type)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::getSomeData()
{
    if (!m_interface) {
        return;
    }

    qDebug() << "About to request some data";

    // Request 16 bytes

    m_interface->beginReadMemory(0, 16, &m_memRes);
    m_interface->beginReadRegisters(&m_targetRegisters);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::endReadMemory(QVector<uint16_t>* res, uint64_t address, int addressSize)
{
    qDebug() << "RegisterView Got memory " << res << " " << res->size() << " addressSize " << addressSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::endReadRegisters(QVector<IBackendRequests::Register>* registers)
{
    for (auto& reg : *registers) {
        qDebug() << "Got register " << reg.name;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::programCounterChanged(uint64_t pc) {
    (void)pc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RegisterView::setBackendInterface(IBackendRequests* interface)
{
    m_interface = interface;
    connect(m_interface, &IBackendRequests::endReadMemory, this, &RegisterView::endReadMemory);
    connect(m_interface, &IBackendRequests::programCounterChanged, this, &RegisterView::programCounterChanged);
    connect(m_interface, &IBackendRequests::endReadRegisters, this, &RegisterView::endReadRegisters);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
