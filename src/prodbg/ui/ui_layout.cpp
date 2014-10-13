#include "ui_layout.h"
#include "core/log.h"
#include <yaml.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int yaml_add_scalar(yaml_document_t* document, const char* key)
{
	return yaml_document_add_scalar(document, (yaml_char_t*)YAML_STR_TAG, (yaml_char_t*)key, (int)strlen(key), YAML_ANY_SCALAR_STYLE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int addMapping(yaml_document_t* document, int mapping, const char* key, const char* value)
{
    int keyId;
    int valueId;

    keyId = yaml_add_scalar(document, key); 
    valueId = yaml_add_scalar(document, value); 

    if (!keyId || !valueId)
    	return -1;

    return yaml_document_append_mapping_pair(document, mapping, keyId, valueId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int addFloatMapping(yaml_document_t* document, int mapping, const char* key, float value)
{
	char tempBuffer[64];
	sprintf(tempBuffer, "%2.4f", value);
	return addMapping(document, mapping, key, tempBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int addIntMapping(yaml_document_t* document, int mapping, const char* key, int value)
{
	char tempBuffer[64];
	sprintf(tempBuffer, "%d", value);
	return addMapping(document, mapping, key, tempBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addInfo(UILayout* layout, yaml_document_t* document, int root)
{
   	int name = yaml_add_scalar(document, "info");
	int info = yaml_document_add_sequence(document, NULL, YAML_BLOCK_SEQUENCE_STYLE);
	int infoMap = yaml_document_add_mapping(document, NULL, YAML_BLOCK_MAPPING_STYLE);

	yaml_document_append_mapping_pair(document, root, name, info);

	addIntMapping(document, infoMap, "base_path_count", layout->basePathCount);
	addIntMapping(document, infoMap, "layout_items_count", layout->layoutItemCount);

	yaml_document_append_sequence_item(document, info, infoMap);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addBasePaths(UILayout* layout, yaml_document_t* document, int root)
{
   	int name = yaml_add_scalar(document, "base_paths");
	int info = yaml_document_add_sequence(document, NULL, YAML_BLOCK_SEQUENCE_STYLE);

	yaml_document_append_mapping_pair(document, root, name, info);

	int basePaths = yaml_document_add_mapping(document, NULL, YAML_BLOCK_MAPPING_STYLE);

	for (int i = 0; i < layout->basePathCount; ++i)
		addMapping(document, basePaths, "path", layout->pluginBasePaths[i]);

	yaml_document_append_sequence_item(document, info, basePaths);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addLayoutItems(UILayout* layout, yaml_document_t* document, int root)
{
   	int name = yaml_add_scalar(document, "layout_items");
	int info = yaml_document_add_sequence(document, NULL, YAML_BLOCK_SEQUENCE_STYLE);

	yaml_document_append_mapping_pair(document, root, name, info);

	for (int i = 0; i < layout->layoutItemCount; ++i)
	{
		const LayoutItem* item = &layout->layoutItems[i];

		int lay = yaml_document_add_mapping(document, NULL, YAML_BLOCK_MAPPING_STYLE);

		addMapping(document, lay, "plugin_file", item->pluginFile); 
		addMapping(document, lay, "plugin_name", item->pluginName); 
		addFloatMapping(document, lay, "x", item->x); 
		addFloatMapping(document, lay, "y", item->y); 
		addFloatMapping(document, lay, "width", item->width); 
		addFloatMapping(document, lay, "height", item->height); 

		yaml_document_append_sequence_item(document, info, lay);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UILayout_saveLayout(UILayout* layout, const char* filename)
{
	yaml_emitter_t emitter;
	yaml_document_t document;
	FILE* f;
	int root, info;

	(void)layout;

	if (!(f = fopen(filename, "wt")))
	{
		log_error("YAML: Unable to open %s for write\n", filename); 
		return false;
	}

	if (!yaml_emitter_initialize(&emitter)) 
	{
		log_error("YAML: Could not inialize the emitter object\n");
		return false;
	}

    yaml_emitter_set_output_file(&emitter, f);
    yaml_emitter_set_canonical(&emitter, 0);
    yaml_emitter_set_unicode(&emitter, 0);

    if (!yaml_emitter_open(&emitter))
        goto emitter_error;

    if (!yaml_document_initialize(&document, NULL, NULL, NULL, 0, 0))
        goto document_error;

    if (!(root = yaml_document_add_mapping(&document, NULL, YAML_BLOCK_MAPPING_STYLE)))
    	goto document_error;

	if (!(info = yaml_document_add_sequence(&document, NULL, YAML_BLOCK_SEQUENCE_STYLE)))
    	goto document_error;

	addInfo(layout, &document, root);
	addBasePaths(layout, &document, root);
	addLayoutItems(layout, &document, root);

	/*
   	name = yaml_document_add_scalar(&document, NULL, (unsigned char*) "info", 4, YAML_PLAIN_SCALAR_STYLE);

	yaml_document_append_mapping_pair(&document, root, name, info );
	app_map = yaml_document_add_mapping(&document, NULL, YAML_BLOCK_MAPPING_STYLE);


	addIntMapping(&document, app_map , "base_path_count", layout->basePathCount);
	addIntMapping(&document, app_map , "layout_items_count", layout->layoutItemCount);

	yaml_document_append_sequence_item(&document, info, app_map);
	*/

	

	yaml_emitter_dump(&emitter, &document);

	return true;

	/// Error handling

emitter_error:

    switch (emitter.error)
    {
        case YAML_MEMORY_ERROR:
            log_error("YAML: Memory error: Not enough memory for emitting\n");
            break;

        case YAML_WRITER_ERROR:
            log_error("YAML: Writer error: %s\n", emitter.problem);
            break;

        case YAML_EMITTER_ERROR:
            log_error("YAML: Emitter error: %s\n", emitter.problem);
            break;

        default:
            log_error("YAML: Internal error\n");
            break;
    }

	fclose(f);
    yaml_document_delete(&document);
    yaml_emitter_delete(&emitter);

    return false;

document_error:

    fprintf(stderr, "Memory error: Not enough memory for creating a document\n");

	fclose(f);
    yaml_document_delete(&document);
    yaml_emitter_delete(&emitter);

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UILayout_loadLayout(UILayout* layout, const char* filename)
{
	(void)layout;
	(void)filename;
	return false;
}

