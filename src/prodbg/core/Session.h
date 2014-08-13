#pragma once

#include <PDBackend.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

struct Session;
struct ViewPluginInstance;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_createRemote(const char* target, int port);
Session* Session_createLocal(PDBackendPlugin* backend, const char* filename);
void Session_destroy(Session* session);

void Session_update(Session* session);
void Session_action(Session* session, PDAction action);
void Session_addViewPlugin(Session* session, ViewPluginInstance* instance);
void Session_removeViewPlugin(Session* session, ViewPluginInstance* instance);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

