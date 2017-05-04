#include <QWidget>
#include "PluginUI.h"
#include "PluginUI_internal.h"

extern "C" void plugin_create(PDUI* ui);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginInstance* PluginUI_createTestPlugin(QWidget* parent) {
	PluginInstance* inst = new PluginInstance;
	QWidget* w = new QWidget(parent);
	inst->priv.parent = w;
	inst->ui_inst = PDUI_getInstance();
	inst->ui_inst->priv = &inst->priv;

	plugin_create(inst->ui_inst);

	return inst;
}

