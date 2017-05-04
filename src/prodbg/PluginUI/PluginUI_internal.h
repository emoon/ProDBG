#pragma once

struct PDUI;

class QWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PrivData {
	QWidget* parent;
	void* user_data;
};

// TODO: Should not be here
struct PluginInstance {
	PrivData priv;
	PDUI* ui_inst;
};

PDUI* PDUI_getInstance();
PluginInstance* PluginUI_createTestPlugin(QWidget* parent);

