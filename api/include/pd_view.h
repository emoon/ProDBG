#ifndef _PDVIEW_H_
#define _PDVIEW_H_

#include "pd_common.h"
#include "pd_readwrite.h"
#include "pd_ui.h"
#include "pd_io.h"

#ifdef _cplusplus
extern "C" {
#endif

struct PDUI;
struct PDUIPainter;
struct PDSaveState;
struct PDLoadState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PD_VIEW_API_VERSION "ProDBG View 1"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDViewPlugin {
    const char* name;

    void* (*create_instance)(PDUI* ui_funcs, ServiceFunc* servicefFunc);
    void (*destroy_instance)(void* user_data);

    // Updates and Returns the current state of the plugin.
    int (*update)(void* user_data, PDUI* uiFuncs, PDReader* reader, PDWriter* writer);

    // save/load state
	int (*save_state)(void* user_data, struct PDSaveState* save_state);
	int (*load_state)(void* user_data, struct PDLoadState* load_state);

} PDViewPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _cplusplus
}
#endif

#endif
