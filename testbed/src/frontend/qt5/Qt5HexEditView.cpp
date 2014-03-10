#include "Qt5HexEditView.h"
#include "Qt5MainWindow.h"

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditViewContextMenu::Qt5HexEditViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent)
    : Qt5DynamicViewContextMenu(mainWindow, parent)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditViewContextMenu::~Qt5HexEditViewContextMenu()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditView::Qt5HexEditView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5DynamicView* parent)
    : Qt5BaseView(mainWindow, dock, parent)
{
    m_type = Qt5ViewType_HexEdit;

    focusInEvent(nullptr);

    m_hexEdit = new Qt5HexEditWidget(this);
    m_hexEdit->setFocusProxy(this);

    createFrameEmbedWidget(m_hexEdit, "Memory");

    connect(m_hexEdit, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuProxy(const QPoint &)));

    // \todo: this code is awful

    m_statusBar = new QStatusBar(m_hexEdit);

    // Address Label
    m_addressNameLabel = new QLabel();
    m_addressNameLabel->setText(tr("Address:"));
    m_statusBar->addPermanentWidget(m_addressNameLabel);

    m_addressLabel = new QLabel();
    m_addressLabel->setFrameShape(QFrame::Panel);
    m_addressLabel->setFrameShadow(QFrame::Sunken);
    m_addressLabel->setMinimumWidth(70);
    m_statusBar->addPermanentWidget(m_addressLabel);

    connect(m_hexEdit, SIGNAL(currentAddressChanged(int)), this, SLOT(setAddress(int)));

    // Size Label
    m_sizeNameLabel = new QLabel();
    m_sizeNameLabel->setText(tr("Size:"));
    m_statusBar->addPermanentWidget(m_sizeNameLabel);

    m_sizeLabel = new QLabel();
    m_sizeLabel->setFrameShape(QFrame::Panel);
    m_sizeLabel->setFrameShadow(QFrame::Sunken);
    m_sizeLabel->setMinimumWidth(70);
    m_statusBar->addPermanentWidget(m_sizeLabel);

    connect(m_hexEdit, SIGNAL(currentSizeChanged(int)), this, SLOT(setSize(int)));

    // Overwrite Mode Label
    m_overwriteModeNameLabel = new QLabel();
    m_overwriteModeNameLabel->setText(tr("Mode:"));
    m_statusBar->addPermanentWidget(m_overwriteModeNameLabel);

    m_overwriteModeLabel = new QLabel();
    m_overwriteModeLabel->setFrameShape(QFrame::Panel);
    m_overwriteModeLabel->setFrameShadow(QFrame::Sunken);
    m_overwriteModeLabel->setMinimumWidth(70);
    m_statusBar->addPermanentWidget(m_overwriteModeLabel);

    //setOverwriteMode(m_hexEdit->getOverwriteMode());

    m_statusBar->showMessage(tr("Ready"));

    testEditor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditView::~Qt5HexEditView()
{
    disconnect();

    // Reset Focus Tracking (for safety)
    m_mainWindow->setCurrentWindow(nullptr, Qt5ViewType_Reset);

    centralWidget()->deleteLater();
    emit signalDelayedSetCentralWidget(nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditView::setAddress(int address)
{
    m_addressLabel->setText(QString("%1").arg(address, 1, 16));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditView::setSize(int size)
{
    m_sizeLabel->setText(QString("%1").arg(size));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5HexEditView::testEditor()
{
    QString fileName = QString(tr("examples/SimpleBin/MyTest.bin"));

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, tr("SDI"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    m_hexEdit->setAddressArea(true);
    m_hexEdit->setAsciiArea(true);
    m_hexEdit->setHighlighting(true);
    m_hexEdit->setOverwriteMode(false);
    m_hexEdit->setReadOnly(false);

    m_hexEdit->setHighlightingColor(QColor(0xff, 0xff, 0x99, 0xff));
    m_hexEdit->setAddressAreaColor(QColor(0xd4, 0xd4, 0xd4, 0xff));
    m_hexEdit->setSelectionColor(QColor(0x6d, 0x9e, 0xff, 0xff));
    m_hexEdit->setFont(QFont("Courier", 12));

    m_hexEdit->setAddressWidth(4);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_hexEdit->setData(file.readAll());
    QApplication::restoreOverrideCursor();

    setWindowFilePath(fileName);

    //setCurrentFile(fileName);
    m_statusBar->showMessage(tr("File loaded"), 2000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
