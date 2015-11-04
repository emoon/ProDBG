#ifndef _PD_HOST_H_
#define _PD_HOST_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDMESSAGEFUNCS_GLOBAL "Info Messages 1"

typedef struct PDMessageFuncs {
	void (*info)(const char* title, const char* message);
	void (*error)(const char* title, const char* message);
	void (*warning)(const char* title, const char* message);
} PDMessageFuncs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDDIALOGS_GLOBAL "Dialogs 1"

typedef struct PDDialogFuncs {
	int (*openFile)(char* dest);
	int (*saveFile)(char* dest);
	int (*select_directory)(char* dest);
} PDDialogFuncs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDSETTINGS_GLOBAL "Settings 1"

typedef struct PDSettingsFuncs {
	int64_t (*get_int)(const char* category, const char* value);
	double (*get_real)(const char* category, const char* value);
	const char* (*get_string)(const char* category, const char* value);
	uint32_t (*get_shortcut)(const char* plugin_id, const char* operation);
} PDSettingsFuncs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif

