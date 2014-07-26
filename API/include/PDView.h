#ifndef _PDVIEW_H_
#define _PDVIEW_H_

#include "PDCommon.h"
#include "PDReadWrite.h"
#include "PDUI.h"

#ifdef _cplusplus
extern "C" {
#endif

struct PDUI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PD_VIEW_API_VERSION "ProDBG View 1"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDViewPlugin
{
    const char* version;
    const char* name;

    void* (*createInstance)(PDUI* uiFuncs, ServiceFunc* serviceFunc);
    void (*destroyInstance)(void* userData);

    // Updates and Returns the current state of the plugin.
    int (*update)(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents);

} PDViewPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _cplusplus
}
#endif

#endif
