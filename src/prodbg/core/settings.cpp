#include "settings.h"
#include <jansson.h>
#include "log.h"
#include "core/alloc.h"
#include <foundation/string.h>
#include <foundation/assert.h>

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

struct Category
{
	const char* name;
	uint32_t hash;
	Setting** settings;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Category** s_categories = 0;

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
		case JSON_STRING : setting->svalue = string_clone(json_string_value(value)); break; 
		case JSON_REAL : setting->dvalue = json_real_value(value); break;
		case JSON_INTEGER : setting->ivalue = json_integer_value(value); break;
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

static void insertOrUpdateSettings(Category* category, json_t* root)
{
	void* iter = json_object_iter(root);

	while (iter)
	{
		const char* key = json_object_iter_key(iter);
		json_t* value = json_object_iter_value(iter);

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


