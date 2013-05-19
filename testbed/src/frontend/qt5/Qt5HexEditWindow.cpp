#include "Qt5HexEditWindow.h"
#include <QAction>
#include <QLabel>
#include <QMenuBar>
#include <QHBoxLayout>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5HexEditWindow::Qt5HexEditWindow()//QWindow* parent)
//: QWindow(parent)
{
	initialize();
}

Qt5HexEditWindow::~Qt5HexEditWindow()
{

}

void Qt5HexEditWindow::setAddress(int address)
{
    m_addressLabel->setText(QString("%1").arg(address, 1, 16));
}

void Qt5HexEditWindow::setSize(int size)
{
    m_sizeLabel->setText(QString("%1").arg(size));
}

void Qt5HexEditWindow::initialize()
{
	setAttribute(Qt::WA_DeleteOnClose);

	m_hexEdit = new Qt5HexEditWidget;
	setCentralWidget(m_hexEdit);

	initializeStatusBar();

	setUnifiedTitleAndToolBarOnMac(true);

	testEditor();
}

void Qt5HexEditWindow::initializeStatusBar()
{
	// Address Label
    m_addressNameLabel = new QLabel();
    m_addressNameLabel->setText(tr("Address:"));
    statusBar()->addPermanentWidget(m_addressNameLabel);

    m_addressLabel = new QLabel();
    m_addressLabel->setFrameShape(QFrame::Panel);
    m_addressLabel->setFrameShadow(QFrame::Sunken);
    m_addressLabel->setMinimumWidth(70);
    statusBar()->addPermanentWidget(m_addressLabel);

    connect(m_hexEdit, SIGNAL(currentAddressChanged(int)), this, SLOT(setAddress(int)));

    // Size Label
    m_sizeNameLabel = new QLabel();
    m_sizeNameLabel->setText(tr("Size:"));
    statusBar()->addPermanentWidget(m_sizeNameLabel);

    m_sizeLabel = new QLabel();
    m_sizeLabel->setFrameShape(QFrame::Panel);
    m_sizeLabel->setFrameShadow(QFrame::Sunken);
    m_sizeLabel->setMinimumWidth(70);
    statusBar()->addPermanentWidget(m_sizeLabel);

    connect(m_hexEdit, SIGNAL(currentSizeChanged(int)), this, SLOT(setSize(int)));

    // Overwrite Mode Label
    m_overwriteModeNameLabel = new QLabel();
    m_overwriteModeNameLabel->setText(tr("Mode:"));
    statusBar()->addPermanentWidget(m_overwriteModeNameLabel);

    m_overwriteModeLabel = new QLabel();
    m_overwriteModeLabel->setFrameShape(QFrame::Panel);
    m_overwriteModeLabel->setFrameShadow(QFrame::Sunken);
    m_overwriteModeLabel->setMinimumWidth(70);
    statusBar()->addPermanentWidget(m_overwriteModeLabel);

    //setOverwriteMode(m_hexEdit->getOverwriteMode());

    statusBar()->showMessage(tr("Ready"));
}

void Qt5HexEditWindow::testEditor()
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
    //QColor lineColor = QColor(Qt::darkGray).lighter(50);
    m_hexEdit->setAddressAreaColor(Qt::black);//QColor(0xd4, 0xd4, 0xd4, 0xff));
    m_hexEdit->setSelectionColor(QColor(0x6d, 0x9e, 0xff, 0xff));
    m_hexEdit->setFont(QFont("Courier", 12));

    m_hexEdit->setAddressWidth(4);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_hexEdit->setData(file.readAll());
    QApplication::restoreOverrideCursor();

    setWindowFilePath(fileName);

    //setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

}

