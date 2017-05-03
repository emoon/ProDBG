#include "PluginUI.h"

void plugin_create(PDUI* ui) {
	ui->button(ui->priv, "Test button", -1, 0);
}
