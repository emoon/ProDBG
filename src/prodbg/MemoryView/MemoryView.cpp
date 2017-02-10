#include "MemoryView.h"
#include "ui_MemoryView.h"
#include "Backend/IBackendRequests.h"
#include <QtCore/QDebug>
#include <QtCore/QMetaEnum>
#include <QSettings>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename EnumType>
static void enumToCombo(QComboBox* combo, int startValue)
{
    QMetaEnum me = QMetaEnum::fromType<EnumType>();

    for (int i = 0; i < me.keyCount(); ++i) {
        combo->addItem(QString::fromUtf8(me.key(i)));
    }

    combo->setCurrentIndex(startValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryView::MemoryView(QWidget* parent)
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

    readSettings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MemoryView::~MemoryView()
{
    writeSettings();
    printf("destruct MemoryView\n");
    delete m_Ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryView::endianChanged(int e)
{
    m_Ui->m_View->setEndianess(static_cast<MemoryViewWidget::Endianess>(e));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryView::interfaceSet()
{
    m_Ui->m_View->setBackendInterface(m_interface);

    if (m_interface) {
        connect(m_interface, &IBackendRequests::endResolveAddress, this, &MemoryView::endResolveAddress);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryView::dataTypeChanged(int t)
{
    m_Ui->m_View->setDataType(static_cast<MemoryViewWidget::DataType>(t));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryView::jumpAddressChanged()
{
    jumpToAddressExpression(m_Ui->m_Address->text());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryView::jumpToAddressExpression(const QString& str)
{
    if (m_interface) {
        m_interface->beginResolveAddress(str, &m_evalAddress);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryView::endResolveAddress(uint64_t* out)
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryView::countChanged(const QString& text)
{
    bool ok = false;
    int count = text.toInt(&ok, /*base:*/ 0);
    if (ok) {
        m_Ui->m_View->setElementsPerLine(count);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
void MemoryView::readSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    // TODO: Support more than one memory view
    settings.beginGroup(QStringLiteral("MemoryView_0"));
    m_Ui->m_Endianess->setCurrentIndex(settings.value(QStringLiteral("endian")).toInt());
    m_Ui->m_Type->setCurrentIndex(settings.value(QStringLiteral("data_type")).toInt());
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MemoryView::writeSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    // TODO: Support more than one memory view
    settings.beginGroup(QStringLiteral("MemoryView_0"));
    settings.setValue(QStringLiteral("endian"), m_Ui->m_Endianess->currentIndex());
    settings.setValue(QStringLiteral("data_type"), m_Ui->m_Type->currentIndex());
    settings.setValue(QStringLiteral("count"), m_Ui->m_Type->currentIndex());
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
