#include "ui_layout.h"
#include "core/log.h"
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

