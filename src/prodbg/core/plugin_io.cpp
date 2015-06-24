#include "plugin_io.h"
#include "api/include/pd_io.h"
#include <stdint.h>
#include <jansson.h>
#include <foundation/string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeJsonInt(void* privData, const int64_t v)
{
    json_t* root = (json_t*)privData;
    json_array_append_new(root, json_integer(v));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeJsonDouble(void* privData, const double v)
{
    json_t* root = (json_t*)privData;
    json_array_append_new(root, json_real(v));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeJsonString(void* privData, const char* str)
{
    json_t* root = (json_t*)privData;
    json_array_append_new(root, json_string(str));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDLoadStatus readJsonInt(void* privData, int64_t* dest)
{
    SessionLoadState* loadState = (SessionLoadState*)privData;
    json_t* item = json_array_get(loadState->root, (size_t)(loadState->arrayIter++));

    *dest = 0;

    if (!item)
        return PDLoadStatus_outOfData;

    if (json_typeof(item) == JSON_INTEGER)
    {
        *dest = json_integer_value(item);
        return PDLoadStatus_ok;
    }

    if (json_typeof(item) == JSON_REAL)
    {
        *dest = (int64_t)json_real_value(item);
        return PDLoadStatus_converted;
    }

    return PDLoadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDLoadStatus readJsonDouble(void* privData, double* dest)
{
    SessionLoadState* loadState = (SessionLoadState*)privData;
    json_t* item = json_array_get(loadState->root, (size_t)(loadState->arrayIter++));

    *dest = 0.0;

    if (!item)
        return PDLoadStatus_outOfData;

    if (json_typeof(item) == JSON_REAL)
    {
        *dest = json_real_value(item);
        return PDLoadStatus_ok;
    }

    if (json_typeof(item) == JSON_INTEGER)
    {
        *dest = (double)json_integer_value(item);
        return PDLoadStatus_converted;
    }

    return PDLoadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDLoadStatus readJsonString(void* privData, char* dest, int len)
{
    SessionLoadState* loadState = (SessionLoadState*)privData;

    json_t* item = json_array_get(loadState->root, (size_t)(loadState->arrayIter++));

    if (!item)
        return PDLoadStatus_outOfData;

    if (len == 0)
        return PDLoadStatus_fail;

    if (len == 1)
    {
        *dest = 0;
        return PDLoadStatus_truncated;
    }

    if (json_is_number(item))
        return PDLoadStatus_fail;

    if (json_is_string(item))
    {
        const char* jsonString = json_string_value(item);
        int jsonStringLength = (int)string_length(jsonString);

        if (jsonStringLength < len)
        {
            strcpy(dest, jsonString);
            return PDLoadStatus_ok;
        }

        // can't fit so need to truncate

        string_copy(dest, jsonString, (unsigned int)len);
        return PDLoadStatus_truncated;
    }

    return PDLoadStatus_fail;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDSaveState jsonSaveState =
{
    0,
    writeJsonInt,
    writeJsonDouble,
    writeJsonString,
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDLoadState loadSaveState =
{
    0,
    readJsonInt,
    readJsonDouble,
    readJsonString,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginIO_initLoadJson(PDLoadState* loadFuncs)
{
    *loadFuncs = loadSaveState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginIO_initSaveJson(PDSaveState* saveFuncs)
{
    *saveFuncs = jsonSaveState;
}

