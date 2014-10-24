#pragma once

#include <pd_backend.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session;
struct ViewPluginInstance;
struct UILayout;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session* Session_create();

struct Session* Session_createRemote(const char* target, int port);
struct Session* Session_startRemote(Session* session, const char* target, int port);

struct Session* Session_createLocal(PDBackendPlugin* backend, const char* filename);
struct Session* Session_startLocal(Session* session, PDBackendPlugin* backend, const char* filename);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_destroy(struct Session* session);

void Session_update(struct Session* session);
void Session_action(struct Session* session, PDAction action);
void Session_addViewPlugin(struct Session* session, struct ViewPluginInstance* instance);
bool Session_removeViewPlugin(struct Session* session, struct ViewPluginInstance* instance);

struct ViewPluginInstance** Session_getViewPlugins(struct Session* session, int* count);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_getLayout(Session* session, UILayout* layout, float width, float height);
void Session_setLayout(Session* session, UILayout* layout, float width, float height);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_stepIn(Session* session);
void Session_stepOver(Session* session);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: TEMP

void Session_loadSourceFile(Session* session, const char* filename);
void Session_toggleBreakpointCurrentLine(Session* s);

