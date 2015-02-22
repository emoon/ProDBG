#include "ui_dock_layout.h"
#include "ui_dock.h"
#include "ui_dock_private.h"
#include "api/include/pd_view.h"
#include "core/log.h"
#include "core/alloc.h"
#include "core/plugin_handler.h"
#include <jansson.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addInfo(UILayout* layout, json_t* root)
{
    json_t* info = json_pack("{s:i, s:i}",
                             "base_path_count",    layout->basePathCount,
                             "layout_items_count", layout->layoutItemCount);

    json_object_set_new(root, "info", info);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addBasePaths(UILayout* layout, json_t* root)
{
    json_t* basePathsArray = json_array();

    for (int i = 0; i < layout->basePathCount; ++i)
        json_array_append_new(basePathsArray, json_string(layout->pluginBasePaths[i]));

    json_object_set_new(root, "base_paths", basePathsArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addLayoutItems(UILayout* layout, json_t* root)
{
    json_t* layoutItemsArray = json_array();

    for (int i = 0; i < layout->layoutItemCount; ++i)
    {
        const LayoutItem* item = &layout->layoutItems[i];

        json_t* layoutItem = json_pack("{s:s, s:s, s:f, s:f, s:f, s:f}",
                                       "plugin_file", item->pluginFile,
                                       "plugin_name", item->pluginName,
                                       "x",           item->rect.x,
                                       "y",           item->rect.y,
                                       "width",       item->rect.width,
                                       "height",      item->rect.height);

        json_array_append_new(layoutItemsArray, layoutItem);
    }

    json_object_set_new(root, "layout_items", layoutItemsArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UILayout_saveLayout(UILayout* layout, const char* filename)
{
    json_t* root = json_object();

    addInfo(layout, root);
    addBasePaths(layout, root);
    addLayoutItems(layout, root);

    if (json_dump_file(root, filename, JSON_INDENT(4)) != 0)
    {
        log_error("JSON: Unable to open %s for write\n", filename);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool loadObjectInt(json_t* object, const char* key, int& result)
{
    void* iter = json_object_iter_at(object, key);
    if (!iter)
        return false;

    json_t* value = json_object_iter_value(iter);
    if (!value || !json_is_integer(value))
        return false;

    result = (int)json_integer_value(value);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool loadObjectFloat(json_t* object, const char* key, float& result)
{
    void* iter = json_object_iter_at(object, key);
    if (!iter)
        return false;

    json_t* value = json_object_iter_value(iter);
    if (!value || !json_is_real(value))
        return false;

    result = (float)json_real_value(value);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool loadObjectString(json_t* object, const char* key, char** result)
{
    void* iter = json_object_iter_at(object, key);
    if (!iter)
        return false;

    json_t* value = json_object_iter_value(iter);
    if (!value || !json_is_string(value))
        return false;

    *result = strdup(json_string_value(value));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UILayout_loadLayout(UILayout* layout, const char* filename)
{
    memset(layout, 0, sizeof(UILayout));

    json_error_t error;

    json_t* root = json_load_file(filename, 0, &error);
    if (!root || !json_is_object(root))
    {
        log_error("JSON: Unable to open %s for read\n", filename);
        return false;
    }

    // Load info

    json_t* info = json_object_get(root, "info");
    if (!info || !json_is_object(info))
    {
        log_error("JSON: Unable to load info object\n");
        return false;
    }

    if (!loadObjectInt(info, "base_path_count", layout->basePathCount))
    {
        log_error("JSON: Unable to load info object : base_path_count\n");
        return false;
    }

    layout->pluginBasePaths = (const char**)malloc((sizeof(void*) * (size_t)layout->basePathCount));

    if (!loadObjectInt(info, "layout_items_count", layout->layoutItemCount))
    {
        log_error("JSON: Unable to load info object : layout_items_count\n");
        return false;
    }

    layout->layoutItems = (LayoutItem*)malloc((sizeof(LayoutItem) * (size_t)layout->layoutItemCount));

    // Load base paths

    json_t* basePaths = json_object_get(root, "base_paths");
    if (!basePaths || !json_is_array(basePaths))
    {
        log_error("JSON: Unable to load base paths object\n");
        return false;
    }

    for (int i = 0; i < layout->basePathCount; ++i)
    {
        json_t* basePath = json_array_get(basePaths, (size_t)i);
        if (!basePath || !json_is_string(basePath))
        {
            log_error("JSON: Unable to load base path entry\n");
            return false;
        }

        layout->pluginBasePaths[i] = strdup(json_string_value(basePath));
    }

    // Load layout items

    json_t* layoutItems = json_object_get(root, "layout_items");
    if (!layoutItems || !json_is_array(layoutItems))
    {
        log_error("JSON: Unable to load layout items object\n");
        return false;
    }

    for (int i = 0; i < layout->layoutItemCount; ++i)
    {
        json_t* layoutItem = json_array_get(layoutItems, (size_t)i);
        if (!layoutItem || !json_is_object(layoutItem))
        {
            log_error("JSON: Unable to load layout item entry\n");
            return false;
        }

        LayoutItem* item = &layout->layoutItems[i];

        if (!loadObjectString(layoutItem, "plugin_file", (char**)&item->pluginFile))
        {
            log_error("JSON: Unable to load layout item : plugin_file\n");
            return false;
        }

        if (!loadObjectString(layoutItem, "plugin_name", (char**)&item->pluginName))
        {
            log_error("JSON: Unable to load layout item : plugin_name\n");
            return false;
        }

        if (!loadObjectFloat(layoutItem, "x", item->rect.x))
        {
            log_error("JSON: Unable to load layout item : x\n");
            return false;
        }

        if (!loadObjectFloat(layoutItem, "y", item->rect.y))
        {
            log_error("JSON: Unable to load layout item : y\n");
            return false;
        }

        if (!loadObjectFloat(layoutItem, "width", item->rect.width))
        {
            log_error("JSON: Unable to load layout item : width\n");
            return false;
        }

        if (!loadObjectFloat(layoutItem, "height", item->rect.height))
        {
            log_error("JSON: Unable to load layout item : height\n");
            return false;
        }
    }

    return true;
}

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

static void writeSizer(UIDockSizer* sizer, json_t* root, float xScale, float yScale)
{
	(void)xScale;
	(void)yScale;

	json_t* sizerItem = json_pack("{s:i, s:i, s:f, s:f, s:f, s:f}",
									"dir", (int)sizer->dir,
									"id", (int)sizer->id,
									"x",  sizer->rect.x * xScale,
									"y", sizer->rect.y * yScale,
									"width", sizer->rect.width * xScale,
									"height", sizer->rect.height * yScale);

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

static void writeSizers(UIDockingGrid* grid, json_t* root, float xScale, float yScale)
{
    json_t* sizersArray = json_array();

	writeSizer(&grid->topSizer, sizersArray, xScale, yScale);
	writeSizer(&grid->bottomSizer, sizersArray, xScale, yScale);
	writeSizer(&grid->rightSizer, sizersArray, xScale, yScale);
	writeSizer(&grid->leftSizer, sizersArray, xScale, yScale);

    for (UIDockSizer* sizer : grid->sizers)
		writeSizer(sizer, sizersArray, xScale, yScale);

    json_object_set_new(root, "sizers", sizersArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeDocks(UIDockingGrid* grid, json_t* root, float xScale, float yScale)
{
	(void)xScale;
	(void)yScale;

    json_t* docksArray = json_array();

    for (UIDock* dock : grid->docks)
	{
		const char* pluginName = "";
		const char* filename = "";

		if (dock->view->plugin)
		{
        	PluginData* pluginData = PluginHandler_getPluginData(dock->view->plugin);

			pluginName = dock->view->plugin->name;
			filename = pluginData->filename;
		}

        json_t* dockItem = json_pack("{s:s, s:s, s:i, s:i, s:f, s:f, s:f, s:f, s:i, s:i, s:i, s:i}",
                                       "plugin_name", pluginName, 
                                       "plugin_file", filename, 
                                       "type", (int)dock->type,
                                       "id", dock->id,
                                       "x",  dock->view->rect.x * xScale,
                                       "y", dock->view->rect.y * yScale,
                                       "width", dock->view->rect.width * xScale,
                                       "height", dock->view->rect.height * yScale,
                                       "s0", dock->sizers[0]->id,
                                       "s1", dock->sizers[1]->id,
                                       "s2", dock->sizers[2]->id,
                                       "s3", dock->sizers[3]->id);
        
        json_array_append_new(docksArray, dockItem);
	}

    json_object_set_new(root, "docks", docksArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIDock_saveLayout(UIDockingGrid* grid, const char* filename, float xScale, float yScale)
{
	xScale = 1.0f / xScale;
	yScale = 1.0f / yScale;

	assignIds(grid);

    json_t* root = json_object();

	writeSizers(grid, root, xScale, yScale);
	writeDocks(grid, root, xScale, yScale);

    if (json_dump_file(root, filename, JSON_INDENT(4) | JSON_PRESERVE_ORDER) != 0)
    {
        log_error("JSON: Unable to open %s for write\n", filename);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static UIDockSizer* findSizer(UIDockingGrid* grid, const int id)
{
	switch (id)
	{
		case 0 : return &grid->topSizer;
		case 1 : return &grid->bottomSizer;
		case 2 : return &grid->rightSizer;
		case 3 : return &grid->leftSizer; 
	}

	for (UIDockSizer* sizer : grid->sizers)
	{
		if (sizer->id == id)
			return sizer;
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void loadDocks(UIDockingGrid* grid, json_t* root, float xScale, float yScale)
{
	const size_t count = json_array_size(root);

	grid->docks.reserve(count);

	for (size_t i = 0; i < count; ++i)
	{
		double x, y, width, height; 
		const char* pluginName;
		const char* filename;
		int id, type;
		int ids[4];

        json_t* item = json_array_get(root, i);

        json_unpack(item, "{s:s, s:s, s:i, s:i, s:f, s:f, s:f, s:f, s:i, s:i, s:i, s:i}",
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

		FloatRect rect = {{{ (float)x * xScale, (float)y * yScale, (float)width * xScale, (float)height * yScale }}};

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
				view = PluginInstance_createViewPlugin(pluginData);
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

static void loadSizers(UIDockingGrid* grid, json_t* root, float xScale, float yScale)
{
	const size_t count = json_array_size(root);

	for (size_t i = 0; i < count; ++i)
	{
		double x, y, width, height; 
		int dir, id;

        json_t* item = json_array_get(root, i);

		int s = json_unpack(item, "{s:i, s:i, s:f, s:f, s:f, s:f}",
									"dir", &dir, 
									"id", &id,
									"x", &x,
									"y", &y,
									"width", &width, 
									"height", &height);

		FloatRect rect = {{{ (float)x * xScale, (float)y * yScale, (float)width * xScale, (float)height * yScale }}};

		printf("s %d - %d %d - %f %f %f %f\n", s, dir, id, rect.x, rect.y, rect.width, rect.height);

		UIDockSizer* sizer = 0;

		// ID range 0 - 3 = border sizers

		switch (id)
		{
			case UIDock::Top : sizer = &grid->topSizer; break;
			case UIDock::Bottom : sizer = &grid->bottomSizer; break;
			case UIDock::Right : sizer = &grid->rightSizer; break;
			case UIDock::Left : sizer = &grid->leftSizer; break;
		}

		if (!sizer)
			sizer = new UIDockSizer;

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

		// if id isn't in range 0 - 3 it's a regular (non-reserved sizer) so we need to add it 

		if (id >= 4)
			grid->sizers.push_back(sizer);
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

UIDockingGrid* UIDock_loadLayout(const char* filename, float xSize, float ySize)
{
	UIDockingGrid* grid = 0;
    json_error_t error;

    json_t* root = json_load_file(filename, 0, &error);

    if (!root || !json_is_object(root))
    {
        log_error("JSON: Unable to open %s for read\n", filename);
        return 0;
    }

	FloatRect rect = {{{ 0.0f, 0.0f, xSize, ySize }}};

    grid = UIDock_createGrid(&rect);

    // Load sizers 

    json_t* sizers = json_object_get(root, "sizers");

    if (!sizers || !json_is_array(sizers))
    {
        log_error("JSON: Unable to load sizers object\n");
        return 0;
    }

	loadSizers(grid, sizers, xSize, ySize);

    json_t* docks = json_object_get(root, "docks");

    if (!docks || !json_is_array(docks))
    {
        log_error("JSON: Unable to load docks object\n");
        return 0;
    }

	loadDocks(grid, docks, xSize, ySize); 

	// map up the docks with the sizers

	mapDockSizers(grid);

	return grid;
}

