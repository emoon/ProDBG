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

typedef struct PDViewPlugin
{
    const char* name;

    void* (*createInstance)(PDUI* uiFuncs, ServiceFunc* serviceFunc);
    void (*destroyInstance)(void* userData);

    // Updates and Returns the current state of the plugin.
    int (*update)(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents);

    // save/load state
	int (*saveState)(void* userData, struct PDSaveState* saveState);
	int (*loadState)(void* userData, struct PDLoadState* loadState);

} PDViewPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _cplusplus
}
#endif

#endif
