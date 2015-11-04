#pragma once

#include <pd_backend.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum SessionStatus {
    SessionStatus_ok,
    SessionStatus_MenuBackendReserved,
    SessionStatus_MenuBackendCreated,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session;
struct ViewPluginInstance;
struct UILayout;
//struct UIDockingGrid;
struct Con;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_globalInit(bool reloadPlugins);
void Session_globalDestroy();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session* Session_create();
void Session_destroy(Session* session);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session** Session_getSessions();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session* Session_createRemote(const char* target, int port);
struct Session* Session_startRemote(Session* session, const char* target, int port);

struct Session* Session_createLocal(PDBackendPlugin* backend, const char* filename);
struct Session* Session_startLocal(Session* session, PDBackendPlugin* backend, const char* filename);

int Session_isConnected(Session* session);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_destroy(struct Session* session);

void Session_update(struct Session* session);
void Session_action(struct Session* session, PDAction action);
void Session_addViewPlugin(struct Session* session, struct ViewPluginInstance* instance);
bool Session_removeViewPlugin(struct Session* session, struct ViewPluginInstance* instance);

struct ViewPluginInstance** Session_getViewPlugins(struct Session* session, int* count);
struct ViewPluginInstance* Session_getViewAt(struct Session* session, int x, int y, int border);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_getLayout(Session* session, UILayout* layout, float width, float height);
void Session_setLayout(Session* session, UILayout* layout, float width, float height);
bool Session_loadLayout(Session* session, const char* filename, int width, int height);

SessionStatus Session_onMenu(Session* session, int eventId);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_stepIn(Session* session);
void Session_stepOver(Session* session);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: TEMP

void Session_loadSourceFile(Session* session, const char* filename);
void Session_toggleBreakpointCurrentLine(Session* s);

struct Con* Session_getDockingGrid(struct Session* session);
void Session_createDockingGrid(struct Session* session, int width, int height);



