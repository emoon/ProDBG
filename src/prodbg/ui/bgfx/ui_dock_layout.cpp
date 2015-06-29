#ifdef PRODBG_MAC
#include <foundation/apple.h>
#endif
#include "ui_dock.h"
#include "ui_dock_private.h"
#include "../plugin.h"
#include "api/include/pd_view.h"
#include "core/alloc.h"
#include "core/log.h"
#include "core/plugin_handler.h"
#include "core/plugin_io.h"
#include <jansson.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <foundation/assert.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void assignIds(UIDockingGrid* grid)
{
    int index = 4;

    grid->topSizer.id = 0;
    grid->bottomSizer.id = 1;
    grid->rightSizer.id = 2;
    grid->leftSizer.id = 3;

    for (UIDockSizer* sizer : grid->sizers)
        sizer->id = index++;

    index = 0;

    for (UIDock* dock : grid->docks)
        dock->id = index++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void clampSize(UIDockingGrid* grid, IntRect* rect)
{
	int rectX = rect->x + rect->width;
	int rectY = rect->y + rect->height;

	FOUNDATION_ASSERT(grid->rect.x == 0);
	FOUNDATION_ASSERT(grid->rect.y == 0);

	if (rectX > grid->rect.width)
		rect->width -= rectX - grid->rect.width;

	if (rectY > grid->rect.height)
		rect->height -= rectY - grid->rect.height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeSizer(UIDockingGrid* grid, UIDockSizer* sizer, json_t* root)
{
	clampSize(grid, &sizer->rect);

    json_t* sizerItem = json_pack("{s:i, s:i, s:i, s:i, s:i, s:i}",
                                  "dir", sizer->dir,
                                  "id", sizer->id,
                                  "x",  sizer->rect.x,
                                  "y", sizer->rect.y,
                                  "width", sizer->rect.width,
                                  "height", sizer->rect.height);

    printf("writing sizer %d - %d %d - %d %d\n", sizer->id, sizer->rect.x, sizer->rect.y, sizer->rect.width, sizer->rect.height);

    json_t* consArray = json_array();

    for (UIDock* dock : sizer->cons)
    {
        json_t* consItem = json_integer(dock->id);
        json_array_append_new(consArray, consItem);
    }

    json_object_set_new(sizerItem, "cons", consArray );

    json_array_append_new(root, sizerItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeSizers(UIDockingGrid* grid, json_t* root)
{
    json_t* sizersArray = json_array();

    writeSizer(grid, &grid->topSizer, sizersArray);
    writeSizer(grid, &grid->bottomSizer, sizersArray);
    writeSizer(grid, &grid->rightSizer, sizersArray);
    writeSizer(grid, &grid->leftSizer, sizersArray);

    for (UIDockSizer* sizer : grid->sizers)
        writeSizer(grid, sizer, sizersArray);

    json_object_set_new(root, "sizers", sizersArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeDocks(UIDockingGrid* grid, json_t* root)
{
    json_t* docksArray = json_array();

    PDSaveState saveFuncs;
    PluginIO_initSaveJson(&saveFuncs);

    for (UIDock* dock : grid->docks)
    {
        PluginData* pluginData = 0;

        const char* pluginName = "";
        const char* filename = "";

        if (dock->view->plugin)
        {
            pluginData = PluginHandler_getPluginData(dock->view->plugin);
            pluginName = dock->view->plugin->name;
            filename = pluginData->filename;
        }

		clampSize(grid, &dock->view->rect);

        json_t* dockItem = json_pack("{s:s, s:s, s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i}",
                                     "plugin_name", pluginName,
                                     "plugin_file", filename,
                                     "type", (int)dock->type,
                                     "id", dock->id,
                                     "x",  dock->view->rect.x,
                                     "y", dock->view->rect.y,
                                     "width", dock->view->rect.width,
                                     "height", dock->view->rect.height,
                                     "s0", dock->sizers[0]->id,
                                     "s1", dock->sizers[1]->id,
                                     "s2", dock->sizers[2]->id,
                                     "s3", dock->sizers[3]->id);

        json_array_append_new(docksArray, dockItem);

        if (!pluginData)
            continue;

        PDViewPlugin* viewPlugin = (PDViewPlugin*)pluginData->plugin;

        if (!viewPlugin->saveState)
            continue;

        json_t* array = json_array();

        saveFuncs.privData = array;

        viewPlugin->saveState(dock->view->userData, &saveFuncs);

        json_object_set_new(dockItem, "plugin_data", array);
    }

    json_object_set_new(root, "docks", docksArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIDock_saveLayout(UIDockingGrid* grid, const char* filename)
{
    json_t* root = json_object();

    UIDock_saveLayoutJson(grid, root);

    if (json_dump_file(root, filename, JSON_INDENT(4) | JSON_PRESERVE_ORDER) != 0)
    {
        pd_error("JSON: Unable to open %s for write\n", filename);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIDock_saveLayoutJson(UIDockingGrid* grid, json_t* root)
{
    assignIds(grid);

    writeSizers(grid, root);
    writeDocks(grid, root);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static UIDockSizer* findSizer(UIDockingGrid* grid, const int id)
{
    switch (id)
    {
        case 0:
            return &grid->topSizer;
        case 1:
            return &grid->bottomSizer;
        case 2:
            return &grid->rightSizer;
        case 3:
            return &grid->leftSizer;
    }

    for (UIDockSizer* sizer : grid->sizers)
    {
        if (sizer->id == id)
            return sizer;
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void loadDocks(UIDockingGrid* grid, json_t* root)
{
    const size_t count = json_array_size(root);

    grid->docks.reserve(count);

    PDLoadState loadFuncs;
    PluginIO_initLoadJson(&loadFuncs);

    for (size_t i = 0; i < count; ++i)
    {
        int x, y, width, height;
        const char* pluginName;
        const char* filename;
        int id, type;
        int ids[4];

        json_t* item = json_array_get(root, i);

        json_unpack(item, "{s:s, s:s, s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i}",
                    "plugin_name", &pluginName,
                    "plugin_file", &filename,
                    "type", &type,
                    "id", &id,
                    "x", &x,
                    "y", &y,
                    "width", &width,
                    "height", &height,
                    "s0", &ids[0],
                    "s1", &ids[1],
                    "s2", &ids[2],
                    "s3", &ids[3]);

        IntRect rect = {{{ x, y, width, height }}};

        clampSize(grid, &rect);

        ViewPluginInstance* view = 0;

        // if this is the case we have no plugin created (empty window)

        if (!strcmp(pluginName, "") && !strcmp(filename, ""))
        {
            view = (ViewPluginInstance*)alloc_zero(sizeof(ViewPluginInstance));
        }
        else
        {
            PluginData* pluginData = PluginHandler_findPlugin(0, filename, pluginName, true);

            if (!pluginData)
                view = (ViewPluginInstance*)alloc_zero(sizeof(ViewPluginInstance));
            else
                view = g_pluginUI->createViewPlugin(pluginData);

            PDViewPlugin* viewPlugin = (PDViewPlugin*)pluginData->plugin;

            json_t* pluginJsonData = json_object_get(item, "plugin_data");

            if (pluginJsonData && viewPlugin && viewPlugin->loadState)
            {
                SessionLoadState loadState = { pluginJsonData, (int)json_array_size(pluginJsonData), 0 };
                loadFuncs.privData = &loadState;
                viewPlugin->loadState(view->userData, &loadFuncs);
            }
        }

        assert(view);

        UIDock* dock = new UIDock(view);

        for (size_t k = 0; k < UIDock::Sizers_Count; ++k)
            dock->sizers[k] = findSizer(grid, ids[k]);

        dock->view->rect = rect;
        dock->type = (UIDockType)type;
        dock->id = id;

        grid->docks.push_back(dock);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void loadSizers(UIDockingGrid* grid, json_t* root)
{
    const size_t count = json_array_size(root);

    for (size_t i = 0; i < count; ++i)
    {
        int x, y, width, height;
        int dir, id;

        json_t* item = json_array_get(root, i);

        json_unpack(item, "{s:i, s:i, s:i, s:i, s:i, s:i}",
                    "dir", &dir,
                    "id", &id,
                    "x", &x,
                    "y", &y,
                    "width", &width,
                    "height", &height);


        IntRect rect = {{{ x, y, width, height }}};

        clampSize(grid, &rect);

        UIDockSizer* sizer = 0;

        // ID range 0 - 3 = border sizers

        switch (id)
        {
            case UIDock::Top:
            {
                sizer = &grid->topSizer;
                printf("top sizer %d %d\n", sizer->rect.x, sizer->rect.width);
                break;
            }

            case UIDock::Bottom:
            {
                sizer = &grid->bottomSizer;
                break;
            }

            case UIDock::Right:
            {
                sizer = &grid->rightSizer;
                break;
            }

            case UIDock::Left:
            {
                sizer = &grid->leftSizer;
                break;
            }

            default:
            {
                sizer = new UIDockSizer;
                grid->sizers.push_back(sizer);
            }
        }

        sizer->dir = (UIDockSizerDir)dir;
        sizer->id = id;
        sizer->rect = rect;

        json_t* consArray = json_object_get(item, "cons");

        if (consArray && json_is_array(consArray))
        {
            const size_t consCount = json_array_size(consArray);

            for (size_t k = 0; k < consCount; ++k)
            {
                json_t* value = json_array_get(consArray, k);
                int res = (int)json_integer_value(value);
                sizer->dockIds.push_back(res);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static UIDock* findDock(UIDockingGrid* grid, const int id)
{
    for (UIDock* dock : grid->docks)
    {
        if (dock->id == id)
            return dock;
    }

    assert(false);

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void mapSizer(UIDockingGrid* grid, UIDockSizer* sizer)
{
    for (int i : sizer->dockIds)
        sizer->addDock(findDock(grid, i));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void mapDockSizers(UIDockingGrid* grid)
{
    mapSizer(grid, &grid->topSizer);
    mapSizer(grid, &grid->bottomSizer);
    mapSizer(grid, &grid->rightSizer);
    mapSizer(grid, &grid->leftSizer);

    for (UIDockSizer* sizer : grid->sizers)
        mapSizer(grid, sizer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockingGrid* UIDock_loadLayout(const char* filename, int width, int height)
{
    UIDockingGrid* grid = 0;
    json_error_t error;

    json_t* root = json_load_file(filename, 0, &error);

    if (!root || !json_is_object(root))
    {
        pd_error("JSON: Unable to open %s for read\n", filename);
        return 0;
    }

    return UIDock_loadLayoutJson(root, width, height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIDockingGrid* UIDock_loadLayoutJson(struct json_t* root, int width, int height)
{
    UIDockingGrid* grid = 0;

    IntRect rect = {{{ 0, 0, width, height }}};

    grid = UIDock_createGrid(&rect);

    // Load sizers

    json_t* sizers = json_object_get(root, "sizers");

    if (!sizers || !json_is_array(sizers))
    {
        pd_error("JSON: Unable to load sizers object\n");
        UIDock_destroyGrid(grid);
        return 0;
    }

    loadSizers(grid, sizers);

    json_t* docks = json_object_get(root, "docks");

    if (!docks || !json_is_array(docks))
    {
        pd_error("JSON: Unable to load docks object\n");
        UIDock_destroyGrid(grid);
        return 0;
    }

    loadDocks(grid, docks);

    // map up the docks with the sizers

    mapDockSizers(grid);

    return grid;
}

