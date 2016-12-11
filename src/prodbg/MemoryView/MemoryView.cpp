#include "MemoryView.h"
#include "ui_MemoryView.h"
#include "Backend/IBackendRequests.h"

#include <QtCore/QDebug>
#include <QtCore/QMetaEnum>

template <typename EnumType>
static void enumToCombo(QComboBox* combo, int startValue)
{
    QMetaEnum me = QMetaEnum::fromType<EnumType>();

    for (int i = 0; i < me.keyCount(); ++i) {
        combo->addItem(QString::fromUtf8(me.key(i)));
    }

    combo->setCurrentIndex(startValue);
}

prodbg::MemoryView::MemoryView(QWidget* parent)
    : Base(parent)
    , m_Ui(new Ui_MemoryView)
{
    m_Ui->setupUi(this);

    connect(m_Ui->m_Address, &QLineEdit::returnPressed, this, &MemoryView::jumpAddressChanged);

    enumToCombo<MemoryViewWidget::Endianess>(m_Ui->m_Endianess, m_Ui->m_View->endianess());
    enumToCombo<MemoryViewWidget::DataType>(m_Ui->m_Type, m_Ui->m_View->dataType());

    connect(m_Ui->m_Endianess, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MemoryView::endianChanged);
    connect(m_Ui->m_Type, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MemoryView::dataTypeChanged);

    QIntValidator* countValidator = new QIntValidator(1, 64, this);
    m_Ui->m_Count->setValidator(countValidator);

    connect(m_Ui->m_Count, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this,
            &MemoryView::countChanged);
}

prodbg::MemoryView::~MemoryView()
{
    delete m_Ui;
}

void prodbg::MemoryView::endianChanged(int e)
{
    m_Ui->m_View->setEndianess(static_cast<MemoryViewWidget::Endianess>(e));
}

void prodbg::MemoryView::setBackendInterface(IBackendRequests* interface)
{
    m_interface = interface;

    m_Ui->m_View->setBackendInterface(interface);

    if (interface) {
        connect(interface, &IBackendRequests::endResolveAddress, this, &prodbg::MemoryView::endResolveAddress);
    }
}

void prodbg::MemoryView::dataTypeChanged(int t)
{
    m_Ui->m_View->setDataType(static_cast<MemoryViewWidget::DataType>(t));
}

void prodbg::MemoryView::jumpAddressChanged()
{
    jumpToAddressExpression(m_Ui->m_Address->text());
}

void prodbg::MemoryView::jumpToAddressExpression(const QString& str)
{
    if (m_interface) {
        m_interface->beginResolveAddress(str, &m_evalAddress);
    }

    // TODO: Parse, evaluate, etc.
    // qDebug() << "Jump to address:" << str;
}

void prodbg::MemoryView::endResolveAddress(uint64_t* out)
{
    if (!out) {
        m_Ui->m_View->setExpressionStatus(false);
    } else {
        m_Ui->m_View->setExpressionStatus(true);
        m_Ui->m_Address->setText(QStringLiteral("0x") + QString::number(*out, 16));
        m_Ui->m_View->setAddress(*out);
    }

    m_Ui->m_View->update();
}

void prodbg::MemoryView::countChanged(const QString& text)
{
    bool ok = false;
    int count = text.toInt(&ok, /*base:*/ 0);
    if (ok) {
        m_Ui->m_View->setElementsPerLine(count);
    }
}
