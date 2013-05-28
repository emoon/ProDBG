#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Qt5CodeEditor.h"
#include "Qt5Layout.h"
#include <QMainWindow>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

class Qt5DockWidget;
class Qt5MainWindow;
class Qt5ContextMenu;
class Qt5BaseView;

class Qt5BaseView : public QMainWindow
{
	Q_OBJECT

    friend class Qt5DynamicView;
    friend class Qt5MainWindow;
    friend class Qt5DockWidget;

public:
	Qt5BaseView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5BaseView* parent = nullptr);
    virtual ~Qt5BaseView();

    virtual Qt5ViewType getViewType() const = 0;

    int m_id;
    int m_entry;

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void closeEvent(QCloseEvent* event);

    virtual Qt5ContextMenu* createContextMenu()
    {
        return nullptr;
    }

    Qt5MainWindow* m_mainWindow;
    Qt5DockWidget* m_parentDock;
    Qt5BaseView* m_parent;
    Qt5ViewType m_type;

public slots:
    virtual void contextMenuProxy(const QPoint& position);
    virtual void applySettings() {}
    virtual void buildLayout() = 0;
    virtual void applyLayout(Qt5Layout* layout) = 0;

private slots:
    void delayedSetCentralWidget(QWidget* widget);

signals:
    void signalSettings();
    void signalBuildLayout();
    void signalApplyLayout(Qt5Layout* layout);
    void signalDelayedSetCentralWidget(QWidget* widget);

private:
    bool hasSplitter() const;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
