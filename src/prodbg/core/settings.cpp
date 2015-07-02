#include "settings.h"
#include <jansson.h>
#include "log.h"
#include "service.h"
#include "core/alloc.h"
#include "core/core.h"
#include "pd_host.h"
#include "pd_keys.h"
#include <foundation/string.h>
#include <foundation/assert.h>
#include <ctype.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Setting
{
    char* name;
    uint32_t hash;
    uint32_t type;

    union
    {
        double dvalue;
        char* svalue;
        int64_t ivalue;
    };
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Key
{
	char* name;
	char* keyCombo;
	uint32_t key;
	uint32_t hash;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Category
{
    char* name;
    uint32_t hash;
    Setting** settings;
    Key* keys;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Category** s_categories = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDSettingsFuncs s_settingsFuncs = 
{
	Settings_getInt,
	Settings_getReal,
	Settings_getString,
	Settings_getShortcut,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t jenkinsOneHash(const char* key)
{
    uint32_t hash = 0;
    uint32_t i = 0;

    while (key[i] != 0)
    {
        hash += (uint32_t)key[i++];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateSetting(Setting* setting, json_t* value)
{
    if (setting->type == JSON_STRING)
        string_deallocate(setting->svalue);

    int type = setting->type = json_typeof(value);
    FOUNDATION_ASSERT(type != JSON_STRING || type != JSON_INTEGER || type != JSON_REAL);

    switch (setting->type)
    {
        case JSON_STRING:
            setting->svalue = string_clone(json_string_value(value)); break;
        case JSON_REAL:
            setting->dvalue = json_real_value(value); break;
        case JSON_INTEGER:
            setting->ivalue = json_integer_value(value); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void insertOrUpdateSetting(Category* category, const char* key, json_t* value)
{
    uint32_t hash = jenkinsOneHash(key);
    int settingsCount = array_size(category->settings);

    for (int i  = 0; i < settingsCount; ++i)
    {
        Setting* setting = category->settings[i];

        if (setting->hash == hash && string_equal(setting->name, key))
            return updateSetting(setting, value);
    }

    Setting* setting = (Setting*)alloc_zero(sizeof(Setting));
    setting->hash = hash;
    setting->name = string_clone(key);

    updateSetting(setting, value);

    array_push(category->settings, setting);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t decodeKey(const char* keyCombo);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void updateKey(Key* key, const char* shortcut)
{
	if (key->keyCombo)
		string_deallocate(key->keyCombo);

	key->keyCombo = string_clone(shortcut);

	key->key = decodeKey(shortcut);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void insertOrUpdateKey(Category* category, const char* name, const char* keyCombo)
{
	if (!keyCombo)
		return;

    uint32_t hash = jenkinsOneHash(name);

    int keyCount = array_size(category->keys);

    for (int i  = 0; i < keyCount; ++i)
    {
        Key* key = &category->keys[i];

        if (key->hash == hash && string_equal(key->name, name))
            return updateKey(key, keyCombo);
    }

    Key key = { 0 };
    key.hash = hash; 
    key.name = string_clone(name);

    updateKey(&key, keyCombo);

    array_push(category->keys, key);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void insertOrUpdateKeys(Category* category, json_t* root)
{
	if (!json_is_object(root))
		return;

    void* iter = json_object_iter(root);

    while (iter)
    {
        const char* key = json_object_iter_key(iter);
        json_t* value = json_object_iter_value(iter);

		const char* shortcut = json_string_value(json_object_get(value, "Default"));

	#ifdef PRODBG_MAC
		if (json_t* t = json_object_get(value, "Mac"))
			shortcut = json_string_value(t);
	#elif PRODBG_WIN
		if (json_t* t = json_object_get(value, "Windows"))
			shortcut = json_string_value(t);
	#endif

		insertOrUpdateKey(category, key, shortcut);

        iter = json_object_iter_next(root, iter);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void insertOrUpdateSettings(Category* category, json_t* root)
{
    void* iter = json_object_iter(root);

    while (iter)
    {
        const char* key = json_object_iter_key(iter);
        json_t* value = json_object_iter_value(iter);

        if (string_equal(key, "keys"))
        	insertOrUpdateKeys(category, value);
        else
        	insertOrUpdateSetting(category, key, value);

        iter = json_object_iter_next(root, iter);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void insertOrUpdateCategory(const char* categoryName, json_t* root)
{
    uint32_t hash = jenkinsOneHash(categoryName);

    int categoryCount = array_size(s_categories);

    for (int i = 0; i < categoryCount; ++i)
    {
        Category* category = s_categories[i];

        if (category->hash == hash && string_equal(category->name, categoryName))
            return insertOrUpdateSettings(category, root);
    }

    Category* category = (Category*)alloc_zero(sizeof(Category));
    category->hash = hash;
    category->name = string_clone(categoryName);

    insertOrUpdateSettings(category, root);

    array_push(s_categories, category);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void traverseData(json_t* root)
{
    void* iter = json_object_iter(root);

    while (iter)
    {
        const char* key = json_object_iter_key(iter);
        json_t* value = json_object_iter_value(iter);

        insertOrUpdateCategory(key, value);

        iter = json_object_iter_next(root, iter);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Settings_loadSettings(const char* filename)
{
    json_error_t error;

    json_t* root = json_load_file(filename, 0, &error);

    if (!root)
    {
        pd_error("JSON Error: %s:(%d:%d) - %s\n", filename, error.line, error.column, error.text);
        return false;
    }

    traverseData(root);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Settings_registerService()
{
	Service_register(&s_settingsFuncs, PDSETTINGS_GLOBAL); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Settings_destroy()
{
    int categoryCount = array_size(s_categories);

    for (int ic = 0; ic < categoryCount; ++ic)
    {
        Category* category = s_categories[ic];

        int settingsCount = array_size(category->settings);

        for (int is = 0; is < settingsCount; ++is)
        {
            Setting* setting = category->settings[is];

            if (setting->type == JSON_STRING)
                string_deallocate(setting->svalue);

            string_deallocate(setting->name);
            free(setting);
        }

        int keyCount = array_size(category->keys);

        for (int ik = 0; ik < keyCount; ++ik)
        {
            Key* key = &category->keys[ik];

			if (key->name)
            	string_deallocate(key->name);

			if (key->keyCombo)
            	string_deallocate(key->keyCombo);
        }

        array_clear(category->keys);
        array_clear(category->settings);

        string_deallocate(category->name);
        free(category);
    }

    array_clear(s_categories);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const Setting* findSetting(const char* categoryName, const char* settingName)
{
    int categoryCount = array_size(s_categories);

    uint32_t hash = jenkinsOneHash(categoryName);

    for (int ic = 0; ic < categoryCount; ++ic)
    {
        const Category* category = s_categories[ic];

        if (category->hash != hash)
            continue;

        if (!string_equal(category->name, categoryName))
            continue;

        uint32_t settingHash = jenkinsOneHash(settingName);
        int settingsCount = array_size(category->settings);

        for (int is = 0; is < settingsCount; ++is)
        {
            const Setting* setting = category->settings[is];

            if (setting->hash == settingHash && string_equal(setting->name, settingName))
                return setting;
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int64_t Settings_getInt(const char* category, const char* value)
{
    const Setting* setting = findSetting(category, value);

    if (!setting || setting->type != JSON_INTEGER)
        return 0;

    return setting->ivalue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double Settings_getReal(const char* category, const char* value)
{
    const Setting* setting = findSetting(category, value);

    if (!setting || setting->type != JSON_REAL)
        return 0.0;

    return setting->dvalue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* Settings_getString(const char* category, const char* value)
{
    const Setting* setting = findSetting(category, value);

    if (!setting || setting->type != JSON_STRING)
        return 0;

    return setting->svalue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct KeyRemapTable
{
	const char* name;
	uint32_t id;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static KeyRemapTable s_remap[] = 
{
	{ "Escape", PDKEY_ESCAPE },
	{ "Enter", PDKEY_ENTER },
	{ "Tab", PDKEY_TAB },
	{ "Backspace", PDKEY_BACKSPACE },
	{ "Insert", PDKEY_INSERT },
	{ "Delete", PDKEY_DELETE },
	{ "Right", PDKEY_RIGHT },
	{ "Left", PDKEY_LEFT },
	{ "Down", PDKEY_DOWN },
	{ "Up", PDKEY_UP },
	{ "PageUp", PDKEY_PAGE_UP },
	{ "PageDown", PDKEY_PAGE_DOWN },
	{ "Home", PDKEY_HOME },
	{ "End", PDKEY_END },
	{ "CapsLock", PDKEY_CAPS_LOCK },
	{ "ScrollLock", PDKEY_SCROLL_LOCK },
	{ "NumLock", PDKEY_NUM_LOCK },
	{ "PrintScreen", PDKEY_PRINT_SCREEN },
	{ "F1", PDKEY_F1 },
	{ "F2", PDKEY_F2 },
	{ "F3", PDKEY_F3 },
	{ "F4", PDKEY_F4 },
	{ "F5", PDKEY_F5 },
	{ "F6", PDKEY_F6 },
	{ "F7", PDKEY_F7 },
	{ "F8", PDKEY_F8 },
	{ "F9", PDKEY_F9 },
	{ "F10", PDKEY_F10 },
	{ "F11", PDKEY_F11 },
	{ "F12", PDKEY_F12 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t decodeKey(const char* keyCombo)
{
	char temp[1024];

	string_copy(temp, keyCombo, sizeof(temp));

	char* pch = strtok(temp, "+"); 

	uint32_t key = 0;

	while (pch != NULL)
	{	
		if (string_length(pch) == 1)
			key |= ((uint32_t)pch[0]) << 4;
		else if (string_equal(pch, "Ctrl"))
			key |= PDKEY_CTRL;
		else if (string_equal(pch, "Super"))
			key |= PDKEY_SUPER;
		else if (string_equal(pch, "Alt"))
			key |= PDKEY_ALT;
		else if (string_equal(pch, "Shift"))
			key |= PDKEY_SHIFT;
		else
		{
			for (uint32_t i = 0; i < sizeof_array(s_remap); ++i)
			{
				KeyRemapTable* entry = &s_remap[i];
		
				if (string_equal(pch, entry->name))
				{
					key |= (entry->id << 4);
					break;
				}
			}
		}

		pch = strtok(NULL, "+"); 
	}

	return key;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t Settings_getShortcut(const char* categoryName, const char* keyName)
{
    int categoryCount = array_size(s_categories);

    uint32_t hash = jenkinsOneHash(categoryName);

    for (int ic = 0; ic < categoryCount; ++ic)
    {
        const Category* category = s_categories[ic];

        if (category->hash != hash)
            continue;

        if (!string_equal(category->name, categoryName))
            continue;

        uint32_t keyHash = jenkinsOneHash(keyName);
        int keysCount = array_size(category->keys);

        for (int ik = 0; ik < keysCount; ++ik)
        {
            const Key* key = &category->keys[ik];

            if (key->hash == keyHash && string_equal(key->name, keyName))
                return key->key;
        }
    }

    pd_info("Settings: Unable to find category %s\n", categoryName);

    return 0;
}



