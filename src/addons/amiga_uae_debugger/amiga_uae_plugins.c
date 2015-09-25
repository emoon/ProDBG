#include "pd_backend.h"

extern PDBackendPlugin g_backendPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* privateData)
{
    registerPlugin(PD_BACKEND_API_VERSION, &g_backendPlugin, privateData);
}

