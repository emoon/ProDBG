#include "CodeViews.h"
#include "BreakpointModel.h"
#include "CodeView/CodeView.h"
#include "CodeView/DisassemblyView.h"
#include <QDebug>
#include <QFileInfo>
#include <QSettings>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeViews::CodeViews(BreakpointModel* breakpoints, QWidget* parent)
    : QTabWidget(parent)
    , m_breakpoints(breakpoints)
{
    setTabsClosable(true);
    m_disassemblyView = new DisassemblyView(nullptr);
    m_disassemblyView->setBreakpointModel(breakpoints);
    readSettings();

    connect(this, &QTabWidget::tabCloseRequested, this, &CodeViews::closeTab);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CodeViews::~CodeViews()
{
    writeSettings();
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

        if (m_interface) {
            if (added) {
                m_interface->beginAddFileLineBreakpoint(filename, line);
            } else {
                m_interface->beginRemoveFileLineBreakpoint(filename, line);
            }
        }

        view->repaint();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::sessionEnded()
{
    for (int i = 0, c = count(); i < c; ++i) {
        if (tabText(i).indexOf(QStringLiteral("Disassembly")) == 0) {
            continue;
        }

        CodeView* view = dynamic_cast<CodeView*>(widget(i));
        Q_ASSERT(view);

        view->sessionEnded();
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

void CodeViews::reloadCurrentFile()
{
    const int index = currentIndex();

    if (tabText(index).indexOf(QStringLiteral("Disassembly")) == 0) {
        return;
    }

    CodeView* view = dynamic_cast<CodeView*>(widget(index));
    Q_ASSERT(view);

    view->reload();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::programCounterChanged(const IBackendRequests::ProgramCounterChange& pc)
{
    m_disassemblyView->updatePc(pc.programCounter);


    // No source/line for the PC so toggle to disassmbly mode if we already aren't there

    if (pc.line == -1) {
        if (m_mode == SourceView) {
            m_wasInSourceView = true;
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
            view->setExceptionLine(pc.line);

            if (m_mode == SourceView) {
                setCurrentIndex(i);
            }

            return;
        }

        openFile(pc.filename, m_mode == SourceView);

        CodeView* view = dynamic_cast<CodeView*>(widget(tabsCount));
        view->setExceptionLine(pc.line);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::setBackendInterface(IBackendRequests* iface)
{
    m_interface = iface;

    m_disassemblyView->setBackendInterface(iface);

    if (iface) {
        connect(m_interface, &IBackendRequests::programCounterChanged, this, &CodeViews::programCounterChanged);
        connect(m_interface, &IBackendRequests::sessionEnded, this, &CodeViews::sessionEnded);
    }
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

void CodeViews::closeTab(int index)
{
    removeTab(index);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::readSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeViews"));

    int size = settings.beginReadArray(QStringLiteral("sourceFiles"));

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString filename = settings.value(QStringLiteral("sourceFile")).toString();
        openFile(filename, false);
    }

    settings.endArray();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CodeViews::writeSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("CodeViews"));
    settings.beginWriteArray(QStringLiteral("sourceFiles"));

    int index = 0;

    for (int i = 0, c = count(); i < c; ++i) {
        if (tabText(i).indexOf(QStringLiteral("Disassembly")) == 0) {
            continue;
        }

        QString filename = tabToolTip(i);

        settings.setArrayIndex(index);
        settings.setValue(QStringLiteral("sourceFile"), filename);

        index += 1;
    }

    settings.endArray();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
