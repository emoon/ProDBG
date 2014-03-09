#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Qt5ContextMenu.h"
#include "Qt5BaseView.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
class QSplitter;
class QLabel;
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5MainWindow;
class Qt5DockWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5DynamicViewContextMenu : public Qt5ContextMenu
{
    Q_OBJECT

public:
    Qt5DynamicViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent = nullptr);
    virtual ~Qt5DynamicViewContextMenu();
};

class Qt5DynamicView : public Qt5BaseView
{
    Q_OBJECT;

    friend class Qt5BaseView;
    friend class Qt5MainWindow;

public:
    Qt5DynamicView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5BaseView* parent = nullptr);
    virtual ~Qt5DynamicView();

    virtual Qt5ViewType getViewType() const
    {
        return Qt5ViewType_Dynamic;
    }

    virtual QString getViewTypeName() const
    {
        return "Dynamic View";
    }

    QSplitter* getSplitter() const
    {
        return m_splitter;
    }

protected:
    void splitView(Qt::Orientation orientation);
    void assignView(Qt5BaseView* view);

    virtual Qt5ContextMenu* createContextMenu()
    {
        return new Qt5DynamicViewContextMenu(m_mainWindow, this);
    }

protected:
    QSplitter* m_splitter;
    Qt5BaseView* m_children[2];

    QLabel* m_statusLabel;

public slots:
    void applySettings();
    void buildLayout();
    void applyLayout(Qt5Layout* layout);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
