#pragma once

#include <pd_backend.h>
#include <stdbool.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session;
struct ViewPluginInstance;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session* Session_create();
struct Session* Session_createRemote(const char* target, int port);
struct Session* Session_createLocal(PDBackendPlugin* backend, const char* filename);
void Session_destroy(struct Session* session);

void Session_update(struct Session* session);
void Session_action(struct Session* session, PDAction action);
void Session_addViewPlugin(struct Session* session, struct ViewPluginInstance* instance);
bool Session_removeViewPlugin(struct Session* session, struct ViewPluginInstance* instance);

struct ViewPluginInstance** Session_getViewPlugins(struct Session* session, int* count);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

