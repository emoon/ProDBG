#include "CodeViews.h"
#include "CodeView/CodeView.h"
#include "CodeView/DisassemblyView.h"
#include <QDebug>
#include <QFileInfo>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeViews::CodeViews(QWidget* parent)
    : QTabWidget(parent)
{
    setTabsClosable(true);
    m_disassemblyView = new DisassemblyView(nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeViews::~CodeViews()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::toggleBreakpoint()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::openFile(const QString& filename)
{
    QFileInfo info(filename);

    CodeView* codeView = new CodeView;
    codeView->initDefaultSourceFile(filename);
    codeView->setBreakpointModel(m_breakpoints);

    int index = addTab(codeView, info.fileName());

    setCurrentIndex(index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::programCounterChanged(const IBackendRequests::ProgramCounterChange& pc)
{
    (void)pc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::setBackendInterface(IBackendRequests* iface)
{
    m_interface = iface;

    connect(m_interface, &IBackendRequests::programCounterChanged, this, &CodeViews::programCounterChanged);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Toggles to the disassembly view. This view is (right now) always placed to the left most of the tabs

void CodeViews::toggleSourceAsm()
{
    const int tabsCount = count();

    if (m_mode == SourceView) {
        if (tabsCount == 0) {
            addTab(m_disassemblyView, QStringLiteral("Disassembly"));
        } else {
            // Check if the first tab is disassembly, if not insert it at that position
            if (tabText(0).indexOf(QStringLiteral("DisassemblyView")) != 0) {
                insertTab(0, m_disassemblyView, QStringLiteral("Disassembly"));
            }
        }

        m_oldIndex = currentIndex();
        setCurrentIndex(0);

        m_mode = Disassembly;
    } else {
        if (m_oldIndex <= tabsCount) {
            setCurrentIndex(m_oldIndex);
        }

        m_mode = SourceView;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
