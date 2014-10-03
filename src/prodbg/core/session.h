#pragma once

#include <pd_backend.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session;
struct ViewPluginInstance;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session* Session_createRemote(const char* target, int port);
struct Session* Session_createLocal(PDBackendPlugin* backend, const char* filename);
void Session_destroy(struct Session* session);

void Session_update(struct Session* session);
void Session_action(struct Session* session, PDAction action);
void Session_addViewPlugin(struct Session* session, struct ViewPluginInstance* instance);
void Session_removeViewPlugin(struct Session* session, struct ViewPluginInstance* instance);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif