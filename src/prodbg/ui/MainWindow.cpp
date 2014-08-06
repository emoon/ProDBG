#include "MainWindow.h"
#include <core/Log.h>
#include <core/PluginHandler.h>
#include <api/PluginInstance.h>
#include <ui/PluginUI.h>
#include <QMenu>
#include <QMenuBar>
#include <QSignalMapper>
#include <PDView.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct MenuDescriptor
{
    const char* name;
    int id;
    bool active;
    const char* shortCut;
    MenuDescriptor* subMenu;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum MenuIds
{
    //

    MENU_SEPARATOR = 1,

    MENU_FILE_NEW_PROJECT = 0x1000,
    MENU_FILE_CLOSE_PROJECT,
    MENU_FILE_LOAD_PROJECT,
    MENU_FILE_LOAD_EXECUTABLE,
    MENU_FILE_LOAD_SOURCE,
    MENU_FILE_LOAD_DEFAULT_LAYOUT,

    // Debug

    MENU_DEBUG_GO,
    MENU_DEBUG_STOP,
    MENU_DEBUG_RELOAD,
    MENU_DEBUG_STEP,
    MENU_DEBUG_STEP_OVER,
    MENU_DEBUG_STEP_OUT,
    MENU_DEBUG_ATTACH_TO_PROCESS,
    MENU_DEBUG_ATTACH_TO_REMOTE_PROCESS,

    // Plugin start, no entries after this

    MENU_PLUGINS,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static MenuDescriptor s_loadMenu[] =
{
    { "Project...", MENU_FILE_LOAD_PROJECT, false, "Ctrl+L", 0 },
    { "Executable...", MENU_FILE_LOAD_EXECUTABLE, true, "Ctrl+F3", 0 },
    { "Source...", MENU_FILE_LOAD_SOURCE, true, "Ctrl+O", 0 },
    { "Default Layout", MENU_FILE_LOAD_DEFAULT_LAYOUT, false, "", 0 },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static MenuDescriptor s_fileMenu[] =
{
    { "New Project", MENU_FILE_NEW_PROJECT, false, "Ctrl+N", 0 },
    { "Load", 0, true, "", s_loadMenu },
    { "", MENU_SEPARATOR, true, "", 0 },
    { "Close Project", MENU_FILE_CLOSE_PROJECT, false, "Ctrl+D", 0 },
    { "", MENU_SEPARATOR, true, "", 0 },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static MenuDescriptor s_debugMenu[] =
{
    { "Go", MENU_DEBUG_GO, true, "F5", 0 },
    { "Stop", MENU_DEBUG_STOP, true, "Shift+F5", 0 },
    { "Reload", MENU_DEBUG_RELOAD, true, "Ctrl+F2", 0 },
    { "", MENU_SEPARATOR, true, "", 0 },
    { "Step", MENU_DEBUG_STEP, true, "F11", 0 },
    { "Step Over", MENU_DEBUG_STEP_OVER, true, "F10", 0 },
    { "Step Out", MENU_DEBUG_STEP_OVER, true, "Shift+F11", 0 },
    { "Attach process...", MENU_DEBUG_ATTACH_TO_PROCESS, false, "", 0 },
    { "Attach Remote process...", MENU_DEBUG_ATTACH_TO_REMOTE_PROCESS, true, "Ctrl+r", 0 },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onMenu(int id)
{
	if (id >= MENU_PLUGINS)
	{
		log_info("here\n");

		for (int i = 0, count = m_pluginCount; i < count; ++i)
		{
			log_info("here loop");

			if (m_pluginInfoArray[i].menuItem != id)
				continue;

			PDViewPlugin* viewPlugin = (PDViewPlugin*)m_pluginInfoArray[i].plugin->data;

			ViewPluginInstance* instance = PluginInstance_createViewPlugin();
			PluginUI_init(viewPlugin->name, this, &instance->ui);

			PluginInstance_init(instance, viewPlugin);

			//(PDViewPlugin*)m_pluginInfoArray[i].plugin->data;
			//log_info("Wants to create plugin of type %s\n", plugin->name);
		}
	}


	log_info("OnMenu %d\n", id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::createMenuItem(MenuDescriptor* desc, QMenu* menu)
{
	QAction* action = new QAction(desc->name, this);
	action->setShortcut(QKeySequence(QString(desc->shortCut)));
	action->setEnabled(desc->active);
	m_signalMapper->setMapping(action, desc->id);
	menu->addAction(action);
	connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::createMenu(MenuDescriptor* desc, QMenu* menu)
{
    while (desc->name)
    {
        if (desc->id == MENU_SEPARATOR)
        {
            menu->addSeparator();
        }
        else if (desc->subMenu)
        {
            QMenu* newMenu = menu->addMenu(desc->name);
            createMenu(desc->subMenu, newMenu);
        }
        else
        {
			createMenuItem(desc, menu); 
        }

        desc++;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::createWindowMenu()
{
	int pluginCount;
	int menuId = MENU_PLUGINS;

    QMenuBar* mainMenuBar = menuBar();
	QMenu* windowMenu = mainMenuBar->addMenu(tr("&Window"));
    QMenu* pluginMenu = windowMenu->addMenu("New");

	m_pluginCount = 0;
	delete [] m_pluginInfoArray;

	Plugin* plugins = PluginHandler_getPlugins(&pluginCount);

	m_pluginInfoArray = new PluginInfo[pluginCount];

	// TODO: better detection of plugin type and it's matching API

	for (int i = 0; i < pluginCount; ++i)
	{
		if (strstr(plugins[i].type, "View"))
		{
			int index = MENU_PLUGINS - menuId;
			PDViewPlugin* pluginData = (PDViewPlugin*)plugins[i].data;
			MenuDescriptor desc = { pluginData->name, menuId, true, "", 0 };
			createMenuItem(&desc, pluginMenu);
			m_pluginInfoArray[index].plugin = &plugins[i];
			m_pluginInfoArray[index].menuItem = menuId; 
			menuId++;
		}
	}

	m_pluginCount =  menuId - MENU_PLUGINS;
	m_pluginMenu = pluginMenu; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
{
	m_pluginInfoArray = nullptr;
	setStyleSheet("QMainWindow::separator{ background: rgb(200, 200, 200); width: 2px; height: 2px; }");
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    m_signalMapper = new QSignalMapper(this);

    QMenuBar* mainMenuBar = menuBar();

    // Connect file menu
    ///////////////////////////////////////////////////////////////////////////////////////////

    createMenu(s_fileMenu, mainMenuBar->addMenu(tr("&File")));
    createMenu(s_debugMenu, mainMenuBar->addMenu(tr("&Debug")));
	createWindowMenu();

    connect(m_signalMapper, SIGNAL(mapped(int)), SLOT(onMenu(int)));

    resize(1024, 768);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

