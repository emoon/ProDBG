#include "PluginUI.h"
#include <QWidget>
#include <QPushButton>
#include "signal_wrappers.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PrivData {
	QWidget* parent;
	void* user_data;
};

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
