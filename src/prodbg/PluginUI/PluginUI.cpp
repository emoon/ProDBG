#include "PluginUI.h"
#include "PluginUI_internal.h"
#include <QWidget>
#include <QPushButton>
#include "signal_wrappers.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void connect_no_args(QObject* sender, const char* id, void* reciver, void (*callback)(void* user_data)) {
	QSlotWrapperNoArgs* wrap = new QSlotWrapperNoArgs(reciver, (SignalNoArgs)callback);
	QObject::connect(sender, id, wrap, SLOT(method()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PUHandle button(void* priv, const char* name, int name_len, void (*callback)(void* user_data)) {
	PrivData* p = (PrivData*)priv;
	QPushButton* qt_button = new QPushButton(QString::fromLatin1(name, name_len), p->parent);

	if (callback) {
		connect_no_args(qt_button, SIGNAL (pressed()), p->user_data, callback); 
	}

	return (PUHandle)qt_button;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDUI s_pluginUI = {
	button,
	nullptr,
	nullptr,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Return an instance of the plugin UI. We take a copy of this se we can moddify
// the priv pointer which needs to be unique for each instance

PDUI* PDUI_getInstance() {
	PDUI* ui = new PDUI;
	memcpy(ui, &s_pluginUI, sizeof(PDUI));

	return ui;
}




