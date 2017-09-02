#include <QWidget>
#include "PluginUI/generated/c_api.h"
#include "PluginUI_internal.h"

//extern "C" void plugin_create(PU* ui);
extern struct PU* PU_create_instance(void* user_data, QWidget* parent);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginInstance* PluginUI_createTestPlugin(QWidget* parent) {
	PluginInstance* inst = new PluginInstance;
	QWidget* w = new QWidget(parent);
	inst->priv.parent = w;
	inst->ui_inst = PU_create_instance(0, w);
	//inst->ui_inst->priv = &inst->priv;

	//plugin_create(inst->ui_inst);

	return inst;
}

