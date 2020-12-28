/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Pavel Strakhov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include "FastDock.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDrag>
#include <QEvent>
#include <QMetaMethod>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QSplitter>
#include <QTabBar>
#include <QVBoxLayout>
#include "FastDockArea.h"
#include "FastDockSplitter.h"
#include "FastDockWrapper.h"

template <class T>
T findClosestParent(QWidget* widget) {
    while (widget) {
        if (qobject_cast<T>(widget)) {
            return static_cast<T>(widget);
        }
        widget = widget->parentWidget();
    }
    return 0;
}

FastDock::FastDock(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    FastDockWrapper* wrapper = new FastDockWrapper(this, false);
    wrapper->setWindowFlags(wrapper->windowFlags() & ~Qt::Tool);
    mainLayout->addWidget(wrapper);
    m_allowFloatingWindow = true;
    m_createCallback = NULL;
    m_lastUsedArea = NULL;

    m_draggedWrapper = NULL;
    m_hoverArea = NULL;

    QPalette pal = palette();
    pal.setColor(QPalette::Background, pal.color(QPalette::Highlight));

    m_previewOverlay = new QWidget(NULL);
    m_previewOverlay->setAutoFillBackground(true);
    m_previewOverlay->setPalette(pal);
    m_previewOverlay->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                                     Qt::X11BypassWindowManagerHint);
    m_previewOverlay->setStyleSheet(tr("background-color: #64b7ff;"));
    m_previewOverlay->setWindowOpacity(0.3);
    m_previewOverlay->setAttribute(Qt::WA_ShowWithoutActivating);
    m_previewOverlay->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_previewOverlay->hide();

    m_previewTabOverlay = new QWidget(NULL);
    m_previewTabOverlay->setAutoFillBackground(true);
    m_previewTabOverlay->setPalette(pal);
    m_previewTabOverlay->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                                        Qt::X11BypassWindowManagerHint);
    m_previewTabOverlay->setStyleSheet(tr("background-color: #64b7ff;"));
    m_previewTabOverlay->setWindowOpacity(0.3);
    m_previewTabOverlay->setAttribute(Qt::WA_ShowWithoutActivating);
    m_previewTabOverlay->setAttribute(Qt::WA_AlwaysStackOnTop);
    m_previewTabOverlay->hide();

    for (int i = 0; i < NumReferenceTypes; i++)
        m_dropHotspots[i] = NULL;

    m_dropHotspotDimension = 32;
    m_dropHotspotMargin = 4;

    drawHotspotPixmaps();

    for (AreaReferenceType type :
         {AddTo, TopOf, LeftOf, RightOf, BottomOf, TopWindowSide, LeftWindowSide, RightWindowSide, BottomWindowSide}) {
        m_dropHotspots[type] = new QLabel(NULL);
        m_dropHotspots[type]->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                                             Qt::X11BypassWindowManagerHint);
        m_dropHotspots[type]->setAttribute(Qt::WA_ShowWithoutActivating);
        m_dropHotspots[type]->setAttribute(Qt::WA_AlwaysStackOnTop);
        m_dropHotspots[type]->setPixmap(m_pixmaps[type]);
        m_dropHotspots[type]->setFixedSize(m_dropHotspotDimension, m_dropHotspotDimension);
    }
}

FastDock::~FastDock() {
    delete m_previewOverlay;
    delete m_previewTabOverlay;
    for (QWidget* hotspot : m_dropHotspots)
        delete hotspot;
    while (!m_areas.isEmpty()) {
        delete m_areas.first();
    }
    while (!m_wrappers.isEmpty()) {
        delete m_wrappers.first();
    }
}

void FastDock::setToolWindowProperties(QWidget* toolWindow, FastDock::ToolWindowProperty properties) {
    m_toolWindowProperties[toolWindow] = properties;
    FastDockArea* area = areaOf(toolWindow);
    if (area)
        area->updateToolWindow(toolWindow);
}

FastDock::ToolWindowProperty FastDock::toolWindowProperties(QWidget* toolWindow) {
    return m_toolWindowProperties[toolWindow];
}

void FastDock::addToolWindow(QWidget* toolWindow, const AreaReference& area, FastDock::ToolWindowProperty properties) {
    addToolWindows(QList<QWidget*>() << toolWindow, area, properties);
}

void FastDock::addToolWindows(QList<QWidget*> toolWindows, const FastDock::AreaReference& area,
                              FastDock::ToolWindowProperty properties) {
    for (QWidget* toolWindow : toolWindows) {
        if (!toolWindow) {
            qWarning("cannot add null widget");
            continue;
        }
        if (m_toolWindows.contains(toolWindow)) {
            qWarning("this tool window has already been added");
            continue;
        }
        toolWindow->hide();
        toolWindow->setParent(0);
        m_toolWindows << toolWindow;
        m_toolWindowProperties[toolWindow] = properties;

        QObject::connect(toolWindow, &QWidget::windowTitleChanged, this, &FastDock::windowTitleChanged);
    }
    moveToolWindows(toolWindows, area);
}

FastDockArea* FastDock::areaOf(QWidget* toolWindow) {
    return findClosestParent<FastDockArea*>(toolWindow);
}

FastDockWrapper* FastDock::wrapperOf(QWidget* toolWindow) {
    return findClosestParent<FastDockWrapper*>(toolWindow);
}

void FastDock::moveToolWindow(QWidget* toolWindow, AreaReference area) {
    moveToolWindows(QList<QWidget*>() << toolWindow, area);
}

void FastDock::moveToolWindows(QList<QWidget*> toolWindows, FastDock::AreaReference area) {
    QList<FastDockWrapper*> wrappersToUpdate;
    for (QWidget* toolWindow : toolWindows) {
        if (!m_toolWindows.contains(toolWindow)) {
            qWarning("unknown tool window");
            return;
        }
        FastDockWrapper* oldWrapper = wrapperOf(toolWindow);
        if (toolWindow->parentWidget() != 0) {
            releaseToolWindow(toolWindow);
        }
        if (oldWrapper && !wrappersToUpdate.contains(oldWrapper))
            wrappersToUpdate.push_back(oldWrapper);
    }
    // if we don't have a reference area, we can't use any types that need a reference
    if (area.area() == NULL &&
        (area.type() == AddTo || area.type() == LeftOf || area.type() == RightOf || area.type() == TopOf ||
         area.type() == BottomOf || area.type() == LeftWindowSide || area.type() == RightWindowSide ||
         area.type() == TopWindowSide || area.type() == BottomWindowSide))

    {
        // if the last area is available, use that.
        if (m_lastUsedArea)
            area = AreaReference(AddTo, m_lastUsedArea);
        // if we have no tool windows at all, add into empty space
        else if (m_toolWindows.isEmpty() || m_toolWindows == toolWindows)
            area = AreaReference(EmptySpace);
        // otherwise we have to make it a new floating area
        else
            area = AreaReference(NewFloatingArea);
    }
    if (area.type() == LastUsedArea && !m_lastUsedArea) {
        FastDockArea* foundArea = findChild<FastDockArea*>();
        if (foundArea) {
            area = AreaReference(AddTo, foundArea);
        } else {
            area = EmptySpace;
        }
    }

    if (area.type() == NoArea) {
        // do nothing
    } else if (area.type() == NewFloatingArea) {
        FastDockArea* floatArea = createArea();
        floatArea->addToolWindows(toolWindows);
        FastDockWrapper* wrapper = new FastDockWrapper(this, true);
        wrapper->layout()->addWidget(floatArea);
        wrapper->move(QCursor::pos());
        wrapper->updateTitle();
        wrapper->show();
    } else if (area.type() == AddTo) {
        int idx = -1;
        if (area.dragResult) {
            idx = area.area()->tabBar()->tabAt(area.area()->tabBar()->mapFromGlobal(QCursor::pos()));
        }
        //    area.area()->setTabPosition(QTabWidget::South);
        area.area()->addToolWindows(toolWindows, idx);
    } else if (area.type() == LeftWindowSide || area.type() == RightWindowSide || area.type() == TopWindowSide ||
               area.type() == BottomWindowSide) {
        FastDockWrapper* wrapper = findClosestParent<FastDockWrapper*>(area.area());
        if (!wrapper) {
            qWarning("couldn't find wrapper");
            return;
        }

        if (wrapper->layout()->count() > 1) {
            qWarning("wrapper has multiple direct children");
            return;
        }

        QLayoutItem* item = wrapper->layout()->takeAt(0);

        QSplitter* splitter = createSplitter();
        if (area.type() == TopWindowSide || area.type() == BottomWindowSide) {
            splitter->setOrientation(Qt::Vertical);
        } else {
            splitter->setOrientation(Qt::Horizontal);
        }

        splitter->addWidget(item->widget());
        area.widget()->show();

        delete item;

        FastDockArea* newArea = createArea();
        newArea->addToolWindows(toolWindows);

        if (area.type() == TopWindowSide || area.type() == LeftWindowSide) {
            splitter->insertWidget(0, newArea);
        } else {
            splitter->addWidget(newArea);
        }

        wrapper->layout()->addWidget(splitter);

        QRect areaGeometry = area.widget()->geometry();

        // Convert area percentage desired to relative sizes.
        const int totalStretch = (area.type() == TopWindowSide || area.type() == BottomWindowSide)
                                     ? areaGeometry.height()
                                     : areaGeometry.width();
        int pct = int(totalStretch * area.percentage());

        int a = pct;
        int b = totalStretch - pct;

        if (area.type() == BottomWindowSide || area.type() == RightWindowSide)
            std::swap(a, b);

        splitter->setSizes({a, b});
    } else if (area.type() == LeftOf || area.type() == RightOf || area.type() == TopOf || area.type() == BottomOf) {
        QSplitter* parentSplitter = qobject_cast<QSplitter*>(area.widget()->parentWidget());
        FastDockWrapper* wrapper = qobject_cast<FastDockWrapper*>(area.widget()->parentWidget());
        if (!parentSplitter && !wrapper) {
            qWarning("unknown parent type");
            return;
        }
        bool useParentSplitter = false;
        int indexInParentSplitter = 0;
        QList<int> parentSplitterSizes;
        if (parentSplitter) {
            indexInParentSplitter = parentSplitter->indexOf(area.widget());
            parentSplitterSizes = parentSplitter->sizes();
            if (parentSplitter->orientation() == Qt::Vertical) {
                useParentSplitter = area.type() == TopOf || area.type() == BottomOf;
            } else {
                useParentSplitter = area.type() == LeftOf || area.type() == RightOf;
            }
        }
        if (useParentSplitter) {
            int insertIndex = indexInParentSplitter;
            if (area.type() == BottomOf || area.type() == RightOf) {
                insertIndex++;
            }
            FastDockArea* newArea = createArea();
            newArea->addToolWindows(toolWindows);
            parentSplitter->insertWidget(insertIndex, newArea);

            if (parentSplitterSizes.count() > indexInParentSplitter && parentSplitterSizes[0] != 0) {
                int availSize = parentSplitterSizes[indexInParentSplitter];

                parentSplitterSizes[indexInParentSplitter] = int(availSize * (1.0f - area.percentage()));
                parentSplitterSizes.insert(insertIndex, int(availSize * area.percentage()));

                parentSplitter->setSizes(parentSplitterSizes);
            }
        } else {
            area.widget()->hide();
            area.widget()->setParent(0);
            QSplitter* splitter = createSplitter();
            if (area.type() == TopOf || area.type() == BottomOf) {
                splitter->setOrientation(Qt::Vertical);
            } else {
                splitter->setOrientation(Qt::Horizontal);
            }

            FastDockArea* newArea = createArea();

            // inherit the size policy from the widget we are wrapping
            splitter->setSizePolicy(area.widget()->sizePolicy());

            // store old geometries so we can restore them
            QRect areaGeometry = area.widget()->geometry();
            QRect newGeometry = newArea->geometry();

            splitter->addWidget(area.widget());
            area.widget()->show();

            if (area.type() == TopOf || area.type() == LeftOf) {
                splitter->insertWidget(0, newArea);
            } else {
                splitter->addWidget(newArea);
            }

            if (parentSplitter) {
                parentSplitter->insertWidget(indexInParentSplitter, splitter);

                if (parentSplitterSizes.count() > 0 && parentSplitterSizes[0] != 0) {
                    parentSplitter->setSizes(parentSplitterSizes);
                }
            } else {
                wrapper->layout()->addWidget(splitter);
            }

            newArea->addToolWindows(toolWindows);

            area.widget()->setGeometry(areaGeometry);
            newArea->setGeometry(newGeometry);

            // Convert area percentage desired to relative sizes.
            const int totalStretch = (area.type() == TopOf || area.type() == BottomOf) ? areaGeometry.height()
                                                                                       : areaGeometry.width();
            int pct = int(totalStretch * area.percentage());

            int a = pct;
            int b = totalStretch - pct;

            if (area.type() == BottomOf || area.type() == RightOf)
                std::swap(a, b);

            splitter->setSizes({a, b});
        }
    } else if (area.type() == EmptySpace) {
        FastDockArea* newArea = createArea();
        findChild<FastDockWrapper*>()->layout()->addWidget(newArea);
        newArea->addToolWindows(toolWindows);
    } else if (area.type() == LastUsedArea) {
        m_lastUsedArea->addToolWindows(toolWindows);
    } else {
        qWarning("invalid type");
    }
    simplifyLayout();
    for (QWidget* toolWindow : toolWindows) {
        toolWindowVisibilityChanged(toolWindow, toolWindow->parent() != 0);
        FastDockWrapper* wrapper = wrapperOf(toolWindow);
        if (wrapper && !wrappersToUpdate.contains(wrapper))
            wrappersToUpdate.push_back(wrapper);
    }
    for (FastDockWrapper* wrapper : wrappersToUpdate) {
        wrapper->updateTitle();
    }
}

void FastDock::removeToolWindow(QWidget* toolWindow, bool allowCloseAlreadyChecked) {
    if (!m_toolWindows.contains(toolWindow)) {
        qWarning("unknown tool window");
        return;
    }

    // search up to find the first parent manager
    FastDock* manager = findClosestParent<FastDock*>(toolWindow);

    if (!manager) {
        qWarning("unknown tool window");
        return;
    }

    if (!allowCloseAlreadyChecked) {
        if (!manager->allowClose(toolWindow))
            return;
    }

    moveToolWindow(toolWindow, NoArea);
    m_toolWindows.removeOne(toolWindow);
    m_toolWindowProperties.remove(toolWindow);
    delete toolWindow;
}

bool FastDock::isFloating(QWidget* toolWindow) {
    FastDockWrapper* wrapper = wrapperOf(toolWindow);
    if (wrapper) {
        return wrapper->floating();
    }
    return false;
}

FastDock* FastDock::managerOf(QWidget* toolWindow) {
    if (!toolWindow) {
        qWarning("NULL tool window");
        return NULL;
    }

    return findClosestParent<FastDock*>(toolWindow);
}

void FastDock::closeToolWindow(QWidget* toolWindow) {
    if (!toolWindow) {
        qWarning("NULL tool window");
        return;
    }

    // search up to find the first parent manager
    FastDock* manager = findClosestParent<FastDock*>(toolWindow);

    if (manager) {
        manager->removeToolWindow(toolWindow);
        return;
    }

    qWarning("window not child of any tool window");
}

void FastDock::raiseToolWindow(QWidget* toolWindow) {
    if (!toolWindow) {
        qWarning("NULL tool window");
        return;
    }

    // if the parent is a FastDockArea, switch tabs
    QWidget* parent = toolWindow->parentWidget();
    FastDockArea* area = qobject_cast<FastDockArea*>(parent);
    if (area == NULL && parent)
        parent = parent->parentWidget();

    area = qobject_cast<FastDockArea*>(parent);

    if (area)
        area->setCurrentWidget(toolWindow);
    else
        qWarning("parent is not a tool window area");
}

QWidget* FastDock::createToolWindow(const QString& objectName) {
    if (m_createCallback) {
        QWidget* toolWindow = m_createCallback(objectName, window());
        if (toolWindow) {
            m_toolWindows << toolWindow;
            m_toolWindowProperties[toolWindow] = ToolWindowProperty(0);
            QObject::connect(toolWindow, &QWidget::windowTitleChanged, this, &FastDock::windowTitleChanged);
            return toolWindow;
        }
    }

    return NULL;
}

void FastDock::setDropHotspotMargin(int pixels) {
    m_dropHotspotMargin = pixels;
    drawHotspotPixmaps();
}

void FastDock::setDropHotspotDimension(int pixels) {
    m_dropHotspotDimension = pixels;

    for (QLabel* hotspot : m_dropHotspots) {
        if (hotspot)
            hotspot->setFixedSize(m_dropHotspotDimension, m_dropHotspotDimension);
    }
}

void FastDock::setAllowFloatingWindow(bool allow) {
    m_allowFloatingWindow = allow;
}

QVariantMap FastDock::saveState() {
    QVariantMap result;
    result[QStringLiteral("FastDockStateFormat")] = 1;
    FastDockWrapper* mainWrapper = findChild<FastDockWrapper*>();
    if (!mainWrapper) {
        qWarning("can't find main wrapper");
        return QVariantMap();
    }
    result[QStringLiteral("mainWrapper")] = mainWrapper->saveState();
    QVariantList floatingWindowsData;
    for (FastDockWrapper* wrapper : m_wrappers) {
        if (!wrapper->isWindow()) {
            continue;
        }
        floatingWindowsData << wrapper->saveState();
    }
    result[QStringLiteral("floatingWindows")] = floatingWindowsData;
    return result;
}

void FastDock::restoreState(const QVariantMap& dataMap) {
    if (dataMap.isEmpty()) {
        return;
    }
    if (dataMap[QStringLiteral("FastDockStateFormat")].toInt() != 1) {
        qWarning("state format is not recognized");
        return;
    }
    moveToolWindows(m_toolWindows, NoArea);
    FastDockWrapper* mainWrapper = findChild<FastDockWrapper*>();
    if (!mainWrapper) {
        qWarning("can't find main wrapper");
        return;
    }
    mainWrapper->restoreState(dataMap[QStringLiteral("mainWrapper")].toMap());
    QVariantList floatWins = dataMap[QStringLiteral("floatingWindows")].toList();
    for (QVariant windowData : floatWins) {
        FastDockWrapper* wrapper = new FastDockWrapper(this, true);
        wrapper->restoreState(windowData.toMap());
        wrapper->updateTitle();
        wrapper->show();
        if (wrapper->windowState() & Qt::WindowMaximized) {
            wrapper->setWindowState(Qt::WindowMaximized);
        }
    }
    simplifyLayout();
    for (QWidget* toolWindow : m_toolWindows) {
        toolWindowVisibilityChanged(toolWindow, toolWindow->parentWidget() != 0);
    }
}

FastDockArea* FastDock::createArea() {
    FastDockArea* area = new FastDockArea(this, 0);
    connect(area, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));
    return area;
}

void FastDock::releaseToolWindow(QWidget* toolWindow) {
    FastDockArea* previousTabWidget = findClosestParent<FastDockArea*>(toolWindow);
    if (!previousTabWidget) {
        qWarning("cannot find tab widget for tool window");
        return;
    }
    previousTabWidget->removeTab(previousTabWidget->indexOf(toolWindow));
    toolWindow->hide();
    toolWindow->setParent(0);
}

void FastDock::simplifyLayout() {
    for (FastDockArea* area : m_areas) {
        if (area->parentWidget() == 0) {
            if (area->count() == 0) {
                if (area == m_lastUsedArea) {
                    m_lastUsedArea = 0;
                }
                // QTimer::singleShot(1000, area, SLOT(deleteLater()));
                area->deleteLater();
            }
            continue;
        }
        QSplitter* splitter = qobject_cast<QSplitter*>(area->parentWidget());
        QSplitter* validSplitter = 0;    // least top level splitter that should remain
        QSplitter* invalidSplitter = 0;  // most top level splitter that should be deleted
        while (splitter) {
            if (splitter->count() > 1) {
                validSplitter = splitter;
                break;
            } else {
                invalidSplitter = splitter;
                splitter = qobject_cast<QSplitter*>(splitter->parentWidget());
            }
        }
        if (!validSplitter) {
            FastDockWrapper* wrapper = findClosestParent<FastDockWrapper*>(area);
            if (!wrapper) {
                qWarning("can't find wrapper");
                return;
            }
            if (area->count() == 0 && wrapper->isWindow()) {
                wrapper->hide();
                // can't deleteLater immediately (strange MacOS bug)
                // QTimer::singleShot(1000, wrapper, SLOT(deleteLater()));
                wrapper->deleteLater();
            } else if (area->parent() != wrapper) {
                wrapper->layout()->addWidget(area);
            }
        } else {
            if (area->count() > 0) {
                if (validSplitter && area->parent() != validSplitter) {
                    int index = validSplitter->indexOf(invalidSplitter);
                    validSplitter->insertWidget(index, area);
                }
            }
        }
        if (invalidSplitter) {
            invalidSplitter->hide();
            invalidSplitter->setParent(0);
            // QTimer::singleShot(1000, invalidSplitter, SLOT(deleteLater()));
            invalidSplitter->deleteLater();
        }
        if (area->count() == 0) {
            area->hide();
            area->setParent(0);
            if (area == m_lastUsedArea) {
                m_lastUsedArea = 0;
            }
            // QTimer::singleShot(1000, area, SLOT(deleteLater()));
            area->deleteLater();
        }
        // search up the stack looking for splitters that have only one child which is a splitter
        splitter = qobject_cast<QSplitter*>(area->parentWidget());
        QSplitter* parentSplitter = splitter ? qobject_cast<QSplitter*>(splitter->parentWidget()) : NULL;
        while (splitter && parentSplitter) {
            // this splitter has only one child, and its direct parent is a splitter. Move our child
            // widget
            // into the parent and delete.
            if (splitter->count() == 1) {
                int idx = parentSplitter->indexOf(splitter);
                if (idx == -1) {
                    qCritical() << "Couldn't find splitter in parent widget";
                    break;
                }

                QWidget* child = splitter->widget(0);

                parentSplitter->insertWidget(idx, child);
                child->show();

                splitter->setParent(NULL);
                splitter->hide();
                splitter->deleteLater();
            }

            // move up the stack
            splitter = parentSplitter;
            parentSplitter = qobject_cast<QSplitter*>(splitter->parentWidget());
        }
    }
}

void FastDock::startDrag(const QList<QWidget*>& toolWindows, FastDockWrapper* wrapper) {
    if (dragInProgress()) {
        qWarning("FastDock::execDrag: drag is already in progress");
        return;
    }
    for (QWidget* toolWindow : toolWindows) {
        if (toolWindowProperties(toolWindow) & DisallowUserDocking) {
            return;
        }
    }
    if (toolWindows.isEmpty()) {
        return;
    }

    m_draggedWrapper = wrapper;
    m_draggedToolWindows = toolWindows;
    qApp->installEventFilter(this);
}

QVariantMap FastDock::saveSplitterState(QSplitter* splitter) {
    QVariantMap result;
    result[QStringLiteral("state")] = QString::fromLatin1(splitter->saveState().toBase64());
    result[QStringLiteral("type")] = QStringLiteral("splitter");
    QVariantList items;
    for (int i = 0; i < splitter->count(); i++) {
        QWidget* item = splitter->widget(i);
        QVariantMap itemValue;
        FastDockArea* area = qobject_cast<FastDockArea*>(item);
        if (area) {
            itemValue = area->saveState();
        } else {
            QSplitter* childSplitter = qobject_cast<QSplitter*>(item);
            if (childSplitter) {
                itemValue = saveSplitterState(childSplitter);
            } else {
                qWarning("unknown splitter item");
            }
        }
        items << itemValue;
    }
    result[QStringLiteral("items")] = items;
    return result;
}

QSplitter* FastDock::restoreSplitterState(const QVariantMap& savedData) {
    if (savedData[QStringLiteral("items")].toList().count() < 2) {
        qWarning("invalid splitter encountered");
    }
    QSplitter* splitter = createSplitter();

    QVariantList itemList = savedData[QStringLiteral("items")].toList();
    for (QVariant itemData : itemList) {
        QVariantMap itemValue = itemData.toMap();
        QString itemType = itemValue[QStringLiteral("type")].toString();
        if (itemType == QStringLiteral("splitter")) {
            splitter->addWidget(restoreSplitterState(itemValue));
        } else if (itemType == QStringLiteral("area")) {
            FastDockArea* area = createArea();
            area->restoreState(itemValue);
            splitter->addWidget(area);
        } else {
            qWarning("unknown item type");
        }
    }
    splitter->restoreState(QByteArray::fromBase64(savedData[QStringLiteral("state")].toByteArray()));
    return splitter;
}

void FastDock::updateDragPosition() {
    if (!dragInProgress()) {
        return;
    }
    if (!(qApp->mouseButtons() & Qt::LeftButton)) {
        finishDrag();
        return;
    }

    QPoint pos = QCursor::pos();
    m_hoverArea = NULL;
    FastDockWrapper* hoverWrapper = NULL;

    bool bCurWrapperDragging = false;

    for (FastDockArea* area : m_areas) {
        // don't allow dragging a whole wrapper into a subset of itself
        if (m_draggedWrapper && area->window() == m_draggedWrapper->window()) {
            continue;
        }
        QRect globalAreaRect(area->mapToGlobal(area->rect().topLeft()), area->mapToGlobal(area->rect().bottomRight()));
        if (globalAreaRect.contains(pos)) {
            m_hoverArea = area;
            break;
        }
    }

    if (m_hoverArea == NULL) {
        for (FastDockWrapper* wrapper : m_wrappers) {
            // don't allow dragging a whole wrapper into a subset of itself
            if (wrapper == m_draggedWrapper) {
                continue;
            }
            if (wrapper->rect().contains(wrapper->mapFromGlobal(pos))) {
                hoverWrapper = wrapper;
                break;
            }
        }

        // if we found a wrapper and it's not empty, then we fill into a gap between two areas in a
        // splitter. Search down the hierarchy until we find a splitter whose handle intersects the
        // cursor and pick an area to map to.
        if (hoverWrapper) {
            QLayout* layout = hoverWrapper->layout();
            QLayoutItem* layoutitem = layout ? layout->itemAt(0) : NULL;
            QWidget* layoutwidget = layoutitem ? layoutitem->widget() : NULL;
            QSplitter* splitter = qobject_cast<QSplitter*>(layoutwidget);

            while (splitter) {
                QSplitter* previous = splitter;

                for (int h = 1; h < splitter->count(); h++) {
                    QSplitterHandle* handle = splitter->handle(h);

                    if (handle->rect().contains(handle->mapFromGlobal(pos))) {
                        QWidget* a = splitter->widget(h);
                        QWidget* b = splitter->widget(h + 1);

                        // try the first widget, if it's an area stop
                        m_hoverArea = qobject_cast<FastDockArea*>(a);
                        if (m_hoverArea)
                            break;

                        // then the second widget
                        m_hoverArea = qobject_cast<FastDockArea*>(b);
                        if (m_hoverArea)
                            break;

                        // neither widget is an area - let's search for a splitter to recurse to
                        splitter = qobject_cast<QSplitter*>(a);
                        if (splitter)
                            break;

                        splitter = qobject_cast<QSplitter*>(b);
                        if (splitter)
                            break;

                        // neither side is an area or a splitter - should be impossible, but stop recursing
                        // and treat this like a floating window
                        qWarning("Couldn't find splitter or area at terminal side of splitter");
                        splitter = NULL;
                        hoverWrapper = NULL;
                        break;
                    }
                }

                // if we still have a splitter, and didn't find an area, find which widget contains the
                // cursor and recurse to that splitter
                if (previous == splitter && !m_hoverArea) {
                    for (int w = 0; w < splitter->count(); w++) {
                        QWidget* widget = splitter->widget(w);

                        if (widget->rect().contains(widget->mapFromGlobal(pos))) {
                            splitter = qobject_cast<QSplitter*>(widget);
                            if (splitter)
                                break;

                            // if this isn't a splitter, and it's not an area (since that would have been found
                            // before any of this started) then bail out
                            qWarning("cursor inside unknown child widget that isn't a splitter or area");
                            splitter = NULL;
                            hoverWrapper = NULL;
                            break;
                        }
                    }
                }

                // we found an area to use! stop now
                if (m_hoverArea)
                    break;

                // if we still haven't found anything, bail out
                if (previous == splitter) {
                    qWarning("Couldn't find cursor inside any child of wrapper");
                    splitter = NULL;
                    hoverWrapper = NULL;
                    break;
                }
            }
        }
    }

    if (m_hoverArea || hoverWrapper) {
        for (QWidget* curWidget : m_draggedToolWindows) {
            QList<QWidget*> toolWindows = m_hoverArea->toolWindows();
            for (QWidget* widget : toolWindows) {
                if (widget == curWidget) {
                    bCurWrapperDragging = true;
                }
            }
        }

        FastDockWrapper* wrapper = hoverWrapper;
        if (m_hoverArea)
            wrapper = findClosestParent<FastDockWrapper*>(m_hoverArea);
        QRect wrapperGeometry;
        wrapperGeometry.setSize(wrapper->rect().size());
        wrapperGeometry.moveTo(wrapper->mapToGlobal(QPoint(0, 0)));

        const int margin = m_dropHotspotMargin;

        const int size = m_dropHotspotDimension;
        const int hsize = size / 2;

        if (m_hoverArea) {
            QRect areaClientRect;

            // calculate the rect of the area
            areaClientRect.setTopLeft(m_hoverArea->mapToGlobal(QPoint(0, 0)));
            areaClientRect.setSize(m_hoverArea->rect().size());

            // subtract the rect for the tab bar.
            areaClientRect.adjust(0, m_hoverArea->tabBar()->rect().height(), 0, 0);

            QPoint c = areaClientRect.center();

            if (m_dropHotspotDimension == 100) {
                int width = areaClientRect.width() / 3;
                int height = areaClientRect.height() / 3;

                m_dropHotspots[AddTo]->move(c + QPoint(-width / 2, -height / 2));
                m_dropHotspots[AddTo]->show();
                m_dropHotspots[AddTo]->setFixedSize(width, height);

                m_dropHotspots[TopOf]->move(c + QPoint(-width / 2, -height / 2 - margin - height));
                m_dropHotspots[TopOf]->show();
                m_dropHotspots[TopOf]->setFixedSize(width, height);

                m_dropHotspots[LeftOf]->move(c + QPoint(-width / 2 - margin - width, -height / 2));
                m_dropHotspots[LeftOf]->show();
                m_dropHotspots[LeftOf]->setFixedSize(width, height);

                m_dropHotspots[RightOf]->move(c + QPoint(width / 2 + margin, -height / 2));
                m_dropHotspots[RightOf]->show();
                m_dropHotspots[RightOf]->setFixedSize(width, height);

                m_dropHotspots[BottomOf]->move(c + QPoint(-width / 2, height / 2 + margin));
                m_dropHotspots[BottomOf]->show();
                m_dropHotspots[BottomOf]->setFixedSize(width, height);

                c = wrapperGeometry.center();

                m_dropHotspots[TopWindowSide]->move(QPoint(c.x() - hsize, wrapperGeometry.y() + margin * 2));
                m_dropHotspots[TopWindowSide]->show();

                m_dropHotspots[LeftWindowSide]->move(QPoint(wrapperGeometry.x() + margin * 2, c.y() - hsize));
                m_dropHotspots[LeftWindowSide]->show();

                m_dropHotspots[RightWindowSide]->move(
                    QPoint(wrapperGeometry.right() - size - margin * 2, c.y() - hsize));
                m_dropHotspots[RightWindowSide]->show();

                m_dropHotspots[BottomWindowSide]->move(
                    QPoint(c.x() - hsize, wrapperGeometry.bottom() - size - margin * 2));
                m_dropHotspots[BottomWindowSide]->show();

                if (m_dropHotspotDimension == 100) {
                    m_dropHotspots[AddTo]->setWindowOpacity(0);
                    m_dropHotspots[TopOf]->setWindowOpacity(0);
                    m_dropHotspots[LeftOf]->setWindowOpacity(0);
                    m_dropHotspots[RightOf]->setWindowOpacity(0);
                    m_dropHotspots[BottomOf]->setWindowOpacity(0);
                    m_dropHotspots[TopWindowSide]->setWindowOpacity(0);
                    m_dropHotspots[LeftWindowSide]->setWindowOpacity(0);
                    m_dropHotspots[RightWindowSide]->setWindowOpacity(0);
                    m_dropHotspots[BottomWindowSide]->setWindowOpacity(0);
                }
            } else {
                m_dropHotspots[AddTo]->move(c + QPoint(-hsize, -hsize));
                m_dropHotspots[AddTo]->show();

                m_dropHotspots[TopOf]->move(c + QPoint(-hsize, -hsize - margin - size));
                m_dropHotspots[TopOf]->show();

                m_dropHotspots[LeftOf]->move(c + QPoint(-hsize - margin - size, -hsize));
                m_dropHotspots[LeftOf]->show();

                m_dropHotspots[RightOf]->move(c + QPoint(hsize + margin, -hsize));
                m_dropHotspots[RightOf]->show();

                m_dropHotspots[BottomOf]->move(c + QPoint(-hsize, hsize + margin));
                m_dropHotspots[BottomOf]->show();

                c = wrapperGeometry.center();

                m_dropHotspots[TopWindowSide]->move(QPoint(c.x() - hsize, wrapperGeometry.y() + margin * 2));
                m_dropHotspots[TopWindowSide]->show();

                m_dropHotspots[LeftWindowSide]->move(QPoint(wrapperGeometry.x() + margin * 2, c.y() - hsize));
                m_dropHotspots[LeftWindowSide]->show();

                m_dropHotspots[RightWindowSide]->move(
                    QPoint(wrapperGeometry.right() - size - margin * 2, c.y() - hsize));
                m_dropHotspots[RightWindowSide]->show();

                m_dropHotspots[BottomWindowSide]->move(
                    QPoint(c.x() - hsize, wrapperGeometry.bottom() - size - margin * 2));
                m_dropHotspots[BottomWindowSide]->show();
            }
        } else {
            m_dropHotspots[AddTo]->move(wrapperGeometry.center() + QPoint(-hsize, -hsize));
            m_dropHotspots[AddTo]->show();

            m_dropHotspots[TopOf]->hide();
            m_dropHotspots[LeftOf]->hide();
            m_dropHotspots[RightOf]->hide();
            m_dropHotspots[BottomOf]->hide();

            m_dropHotspots[TopWindowSide]->hide();
            m_dropHotspots[LeftWindowSide]->hide();
            m_dropHotspots[RightWindowSide]->hide();
            m_dropHotspots[BottomWindowSide]->hide();
        }

        for (QWidget* hotspot : m_dropHotspots)
            if (hotspot)
                hotspot->show();
    } else {
        for (QWidget* hotspot : m_dropHotspots)
            if (hotspot)
                hotspot->hide();
    }

    AreaReferenceType hotspot = currentHotspot();
    if ((m_hoverArea || hoverWrapper) &&
        (hotspot == AddTo || hotspot == LeftOf || hotspot == RightOf || hotspot == TopOf || hotspot == BottomOf)) {
        QWidget* parent = m_hoverArea;
        if (parent == NULL)
            parent = hoverWrapper;

        QRect g = parent->geometry();
        g.moveTopLeft(parent->parentWidget()->mapToGlobal(g.topLeft()));

        if (hotspot == LeftOf)
            g.adjust(0, 0, -g.width() / 2, 0);
        else if (hotspot == RightOf)
            g.adjust(g.width() / 2, 0, 0, 0);
        else if (hotspot == TopOf)
            g.adjust(0, 0, 0, -g.height() / 2);
        else if (hotspot == BottomOf)
            g.adjust(0, g.height() / 2, 0, 0);

        QRect tabGeom;

        if (hotspot == AddTo && m_hoverArea && m_hoverArea->count() > 1) {
            QTabBar* tb = m_hoverArea->tabBar();
            g.adjust(0, tb->rect().height(), 0, 0);

            int idx = tb->tabAt(tb->mapFromGlobal(pos));

            if (idx == -1) {
                tabGeom = tb->tabRect(m_hoverArea->count() - 1);
                tabGeom.moveTo(tb->mapToGlobal(QPoint(0, 0)) + tabGeom.topLeft());

                // move the tab one to the right, to indicate the tab is being added after the last one.
                tabGeom.moveLeft(tabGeom.left() + tabGeom.width());

                // clamp from the right, to ensure we don't display any tab off the end of the range
                if (tabGeom.right() > g.right())
                    tabGeom.moveLeft(g.right() - tabGeom.width());
            } else {
                tabGeom = tb->tabRect(idx);
                tabGeom.moveTo(tb->mapToGlobal(QPoint(0, 0)) + tabGeom.topLeft());
            }
        }

        m_previewOverlay->setGeometry(g);

        m_previewTabOverlay->setGeometry(tabGeom);
    } else if ((m_hoverArea || hoverWrapper) && (hotspot == LeftWindowSide || hotspot == RightWindowSide ||
                                                 hotspot == TopWindowSide || hotspot == BottomWindowSide)) {
        FastDockWrapper* wrapper = hoverWrapper;
        if (m_hoverArea)
            wrapper = findClosestParent<FastDockWrapper*>(m_hoverArea);

        QRect g;
        g.moveTopLeft(wrapper->mapToGlobal(QPoint()));
        g.setSize(wrapper->rect().size());

        if (hotspot == LeftWindowSide)
            g.adjust(0, 0, -(g.width() * 5) / 6, 0);
        else if (hotspot == RightWindowSide)
            g.adjust((g.width() * 5) / 6, 0, 0, 0);
        else if (hotspot == TopWindowSide)
            g.adjust(0, 0, 0, -(g.height() * 3) / 4);
        else if (hotspot == BottomWindowSide)
            g.adjust(0, (g.height() * 3) / 4, 0, 0);

        m_previewOverlay->setGeometry(g);
        m_previewTabOverlay->setGeometry(QRect());
    } else {
        bool allowFloat = m_allowFloatingWindow;

        for (QWidget* w : m_draggedToolWindows)
            allowFloat &= !(toolWindowProperties(w) & DisallowFloatWindow);

        // no hotspot highlighted, draw geometry for a float window if previewing a tear-off, or draw
        // nothing if we're dragging a float window as it moves itself.
        // we also don't render any preview tear-off when floating windows are disallowed
        if (m_draggedWrapper || !allowFloat) {
            m_previewOverlay->setGeometry(QRect());
        } else {
            QRect r;
            for (QWidget* w : m_draggedToolWindows) {
                if (w->isVisible())
                    r = r.united(w->rect());
            }
            m_previewOverlay->setGeometry(pos.x(), pos.y(), r.width(), r.height());
        }
        m_previewTabOverlay->setGeometry(QRect());
    }

    if (bCurWrapperDragging) {
        m_previewOverlay->hide();
        m_previewTabOverlay->hide();
        for (QWidget* h : m_dropHotspots)
            if (h && h->isVisible())
                h->hide();
    } else {
        m_previewOverlay->show();
        m_previewTabOverlay->show();
        for (QWidget* h : m_dropHotspots)
            if (h && h->isVisible())
                h->raise();
    }
}

void FastDock::abortDrag() {
    if (!dragInProgress())
        return;

    m_previewOverlay->hide();
    m_previewTabOverlay->hide();
    for (QWidget* hotspot : m_dropHotspots)
        if (hotspot)
            hotspot->hide();
    m_draggedToolWindows.clear();
    m_draggedWrapper = NULL;
    qApp->removeEventFilter(this);
}

void FastDock::finishDrag() {
    if (!dragInProgress()) {
        qWarning("unexpected finishDrag");
        return;
    }
    qApp->removeEventFilter(this);

    // move these locally to prevent re-entrancy
    QList<QWidget*> draggedToolWindows = m_draggedToolWindows;
    FastDockWrapper* draggedWrapper = m_draggedWrapper;

    m_draggedToolWindows.clear();
    m_draggedWrapper = NULL;

    AreaReferenceType hotspot = currentHotspot();

    m_previewOverlay->hide();
    m_previewTabOverlay->hide();
    for (QWidget* h : m_dropHotspots)
        if (h)
            h->hide();

    if (hotspot == NewFloatingArea) {
        // check if we're dragging a whole float window, if so we don't do anything as it's already
        // moved
        if (!draggedWrapper) {
            bool allowFloat = m_allowFloatingWindow;

            for (QWidget* w : draggedToolWindows)
                allowFloat &= !(toolWindowProperties(w) & DisallowFloatWindow);

            if (allowFloat) {
                QRect r;
                for (QWidget* w : draggedToolWindows) {
                    if (w->isVisible())
                        r = r.united(w->rect());
                }

                moveToolWindows(draggedToolWindows, NewFloatingArea);

                FastDockArea* area = areaOf(draggedToolWindows[0]);

                area->parentWidget()->resize(r.size());
            }
        }
    } else {
        if (m_hoverArea) {
            AreaReference ref(hotspot, m_hoverArea);
            ref.dragResult = true;
            moveToolWindows(draggedToolWindows, ref);
        } else {
            moveToolWindows(draggedToolWindows, AreaReference(EmptySpace));
        }
    }
}

void FastDock::drawHotspotPixmaps() {
    for (AreaReferenceType ref : {AddTo, LeftOf, TopOf, RightOf, BottomOf}) {
        m_pixmaps[ref] =
            QPixmap(m_dropHotspotDimension * devicePixelRatio(), m_dropHotspotDimension * devicePixelRatio());
        m_pixmaps[ref].setDevicePixelRatio(devicePixelRatioF());

        QPainter p(&m_pixmaps[ref]);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::HighQualityAntialiasing);

        QRectF rect(0, 0, m_dropHotspotDimension, m_dropHotspotDimension);

        p.fillRect(rect, Qt::transparent);

        rect = rect.marginsAdded(QMarginsF(-1, -1, -1, -1));

        p.setPen(QPen(QBrush(Qt::darkGray), 1.5));
        p.setBrush(QBrush(Qt::lightGray));
        p.drawRoundedRect(rect, 1.5, 1.5, Qt::AbsoluteSize);

        rect = rect.marginsAdded(QMarginsF(-4, -4, -4, -4));

        QRectF fullRect = rect;

        if (ref == LeftOf)
            rect = rect.marginsAdded(QMarginsF(0, 0, -12, 0));
        else if (ref == TopOf)
            rect = rect.marginsAdded(QMarginsF(0, 0, 0, -12));
        else if (ref == RightOf)
            rect = rect.marginsAdded(QMarginsF(-12, 0, 0, 0));
        else if (ref == BottomOf)
            rect = rect.marginsAdded(QMarginsF(0, -12, 0, 0));

        p.setPen(QPen(QBrush(Qt::black), 1.0));
        p.setBrush(QBrush(Qt::white));
        p.drawRect(rect);

        // add a little title bar
        rect.setHeight(3);
        p.fillRect(rect, Qt::SolidPattern);

        // for the sides, add an arrow.
        if (ref != AddTo) {
            QPainterPath path;

            if (ref == LeftOf) {
                QPointF tip = fullRect.center() + QPointF(4, 0);

                path.addPolygon(QPolygonF({
                    tip,
                    tip + QPoint(3, 3),
                    tip + QPoint(3, -3),
                }));
            } else if (ref == TopOf) {
                QPointF tip = fullRect.center() + QPointF(0, 4);

                path.addPolygon(QPolygonF({
                    tip,
                    tip + QPointF(-3, 3),
                    tip + QPointF(3, 3),
                }));
            } else if (ref == RightOf) {
                QPointF tip = fullRect.center() + QPointF(-4, 0);

                path.addPolygon(QPolygonF({
                    tip,
                    tip + QPointF(-3, 3),
                    tip + QPointF(-3, -3),
                }));
            } else if (ref == BottomOf) {
                QPointF tip = fullRect.center() + QPointF(0, -4);

                path.addPolygon(QPolygonF({
                    tip,
                    tip + QPointF(-3, -3),
                    tip + QPointF(3, -3),
                }));
            }

            p.fillPath(path, QBrush(Qt::black));
        }
    }

    // duplicate these pixmaps by default
    m_pixmaps[LeftWindowSide] = m_pixmaps[LeftOf];
    m_pixmaps[RightWindowSide] = m_pixmaps[RightOf];
    m_pixmaps[TopWindowSide] = m_pixmaps[TopOf];
    m_pixmaps[BottomWindowSide] = m_pixmaps[BottomOf];
}

FastDock::AreaReferenceType FastDock::currentHotspot() {
    QPoint pos = QCursor::pos();

    for (int i = 0; i < NumReferenceTypes; i++) {
        if (m_dropHotspots[i] && m_dropHotspots[i]->isVisible() && m_dropHotspots[i]->geometry().contains(pos)) {
            return (FastDock::AreaReferenceType)i;
        }
    }

    if (m_hoverArea) {
        QTabBar* tb = m_hoverArea->tabBar();
        if (tb->rect().contains(tb->mapFromGlobal(QCursor::pos())))
            return AddTo;
    }

    return NewFloatingArea;
}

bool FastDock::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        // right clicking aborts any drag in progress
        if (static_cast<QMouseEvent*>(event)->button() == Qt::RightButton)
            abortDrag();
    } else if (event->type() == QEvent::KeyPress) {
        // pressing escape any drag in progress
        QKeyEvent* ke = (QKeyEvent*)event;
        if (ke->key() == Qt::Key_Escape) {
            abortDrag();
        }
    }
    return QWidget::eventFilter(object, event);
}

bool FastDock::allowClose(QWidget* toolWindow) {
    if (!m_toolWindows.contains(toolWindow)) {
        qWarning("unknown tool window");
        return true;
    }
    int methodIndex = toolWindow->metaObject()->indexOfMethod(QMetaObject::normalizedSignature("checkAllowClose()"));

    if (methodIndex >= 0) {
        bool ret = true;
        toolWindow->metaObject()->method(methodIndex).invoke(toolWindow, Qt::DirectConnection, Q_RETURN_ARG(bool, ret));

        return ret;
    }

    return true;
}

void FastDock::tabCloseRequested(int index) {
    FastDockArea* tabWidget = qobject_cast<FastDockArea*>(sender());
    if (!tabWidget) {
        qWarning("sender is not a FastDockArea");
        return;
    }
    QWidget* toolWindow = tabWidget->widget(index);
    if (!m_toolWindows.contains(toolWindow)) {
        qWarning("unknown tab in tab widget");
        return;
    }

    if (!allowClose(toolWindow))
        return;

    if (toolWindowProperties(toolWindow) & FastDock::HideOnClose)
        hideToolWindow(toolWindow);
    else
        removeToolWindow(toolWindow, true);
}

void FastDock::windowTitleChanged(const QString&) {
    QWidget* toolWindow = qobject_cast<QWidget*>(sender());
    if (!toolWindow) {
        return;
    }
    FastDockArea* area = areaOf(toolWindow);
    if (area) {
        area->updateToolWindow(toolWindow);
    }
}

QSplitter* FastDock::createSplitter() {
    QSplitter* splitter = new FastDockSplitter();
    splitter->setChildrenCollapsible(false);
    return splitter;
}

FastDock::AreaReference::AreaReference(FastDock::AreaReferenceType type, FastDockArea* area, float percentage) {
    m_type = type;
    m_percentage = percentage;
    dragResult = false;
    setWidget(area);
}

void FastDock::AreaReference::setWidget(QWidget* widget) {
    if (m_type == LastUsedArea || m_type == NewFloatingArea || m_type == NoArea || m_type == EmptySpace) {
        if (widget != 0) {
            qWarning("area parameter ignored for this type");
        }
        m_widget = 0;
    } else if (m_type == AddTo) {
        m_widget = qobject_cast<FastDockArea*>(widget);
        if (!m_widget) {
            qWarning("only FastDockArea can be used with this type");
        }
    } else {
        if (!qobject_cast<FastDockArea*>(widget) && !qobject_cast<QSplitter*>(widget)) {
            qWarning("only FastDockArea or splitter can be used with this type");
            m_widget = 0;
        } else {
            m_widget = widget;
        }
    }
}

FastDockArea* FastDock::AreaReference::area() const {
    return qobject_cast<FastDockArea*>(m_widget);
}

FastDock::AreaReference::AreaReference(FastDock::AreaReferenceType type, QWidget* widget) {
    m_type = type;
    dragResult = false;
    setWidget(widget);
}