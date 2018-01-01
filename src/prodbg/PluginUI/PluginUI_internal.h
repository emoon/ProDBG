#pragma once

struct PU;

class QWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PrivData {
	QWidget* parent;
	void* user_data;
};

// TODO: Should not be here
struct PluginInstance {
	PrivData priv;
	PU* ui_inst;
};

PluginInstance* PluginUI_createTestPlugin(QWidget* parent);

