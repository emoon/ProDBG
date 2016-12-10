#include "CodeViews.h"
#include "CodeView/CodeView.h"
#include "CodeView/DisassemblyView.h"
#include "BreakpointModel.h"
#include <QDebug>
#include <QFileInfo>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeViews::CodeViews(BreakpointModel* breakpoints, QWidget* parent)
    : QTabWidget(parent)
    , m_breakpoints(breakpoints)
{
    setTabsClosable(true);
    m_disassemblyView = new DisassemblyView(nullptr);
    m_disassemblyView->setBreakpointModel(breakpoints);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeViews::~CodeViews()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::toggleBreakpoint()
{
    const int index = currentIndex();

    if (index == 0 && tabText(0).indexOf(QStringLiteral("Disassembly")) == 0) {
        m_disassemblyView->toggleBreakpoint();
    } else {
        CodeView* view = dynamic_cast<CodeView*>(widget(index));
        Q_ASSERT(view);

        const QString& filename = tabToolTip(index);

        int line = view->getCurrentLine();
        bool added = m_breakpoints->toggleFileLineBreakpoint(filename, line + 1);

        if (added) {
            m_interface->beginAddFileLineBreakpoint(filename, line);
        } else {
            m_interface->beginRemoveFileLineBreakpoint(filename, line);
        }

        view->repaint();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::openFile(const QString& filename, bool setActive)
{
    QFileInfo info(filename);

    CodeView* codeView = new CodeView;
    codeView->initDefaultSourceFile(filename);
    codeView->setBreakpointModel(m_breakpoints);

    int index = addTab(codeView, info.fileName());

    setTabToolTip(index, filename);

    if (setActive) {
        setCurrentIndex(index);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::programCounterChanged(const IBackendRequests::ProgramCounterChange& pc)
{
    m_disassemblyView->updatePc(pc.programCounter);

    // No source/line for the PC so toggle to disassmbly mode if we already aren't there

    if (pc.line == -1) {
        if (m_mode == SourceView) {
            m_wasInSourceView  = true;
        }

        setDisassemblyMode();
    } else {
        if (m_mode == Disassembly && m_wasInSourceView) {
            toggleSourceAsm();
            m_wasInSourceView = false;
        }

        int tabsCount = count();

        // Search for the source file if we already have it open activate it
        for (int i = 0; i < tabsCount; ++i) {
            const QString& filename = tabToolTip(i);

            if (filename != pc.filename) {
                continue;
            }

            CodeView* view = dynamic_cast<CodeView*>(widget(i));
            Q_ASSERT(view);
            view->setLine(pc.line);

            if (m_mode == SourceView) {
                setCurrentIndex(i);
            }

            return;
        }

        openFile(pc.filename, m_mode == SourceView);

        CodeView* view = dynamic_cast<CodeView*>(widget(tabsCount));
        view->setLine(pc.line);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::setBackendInterface(IBackendRequests* iface)
{
    m_interface = iface;

    m_disassemblyView->setBackendInterface(iface);

    connect(m_interface, &IBackendRequests::programCounterChanged, this, &CodeViews::programCounterChanged);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Toggles to the disassembly view. This view is (right now) always placed to the left most of the tabs

void CodeViews::toggleSourceAsm()
{
    if (m_mode == SourceView) {
        setDisassemblyMode();
    } else {
        setSourceMode();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::setDisassemblyMode()
{
    const int tabsCount = count();

    if (tabsCount == 0) {
        addTab(m_disassemblyView, QStringLiteral("Disassembly"));
    } else {
        // Check if the first tab is disassembly, if not insert it at that position
        if (tabText(0).indexOf(QStringLiteral("Disassembly")) != 0) {
            insertTab(0, m_disassemblyView, QStringLiteral("Disassembly"));
        }
    }

    m_oldIndex = currentIndex();

    setCurrentIndex(0);

    m_mode = Disassembly;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::setSourceMode()
{
    const int tabsCount = count();

    if (m_oldIndex <= tabsCount) {
        setCurrentIndex(m_oldIndex);
    }

    m_mode = SourceView;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
