#include "wrui.h"
#include "signal_wrappers.h"
#include "widget_private.h"
#include <QApplication>
#include <QPushButton>
#include <QMainWindow>
#include <QTabWidget>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PrivateData {
    QWidget* parent;
    void* user_data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void widget_set_size(struct PUWidget* widget, int width, int height) {
	QObject* q_obj = (QObject*) widget->object->p;
	QWidget* q_widget = static_cast<QWidget*>(q_obj);
	q_widget->resize(width, height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void widget_show(struct PUWidget* widget) {
	QObject* q_obj = (QObject*) widget->object->p;
	QWidget* q_widget = static_cast<QWidget*>(q_obj);
	q_widget->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUWidgetFuncs s_widgetFuncs = {
	widget_set_size,
	widget_show,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_button_title(struct PUPushButton* button, const char* text) {
	QPushButton* qt_button = (QPushButton*)button->base->object->p;
	printf("Push button ptr (call) %p\n", (void*)qt_button);
	qt_button->setText(QString::fromUtf8(text));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUPushButtonFuncs s_pushButtonFuncs {
	set_button_title,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PUPushButton* push_button_create(void* priv) {
    PrivateData* priv_data = (PrivateData*)priv;

	QPushButton* qt_button = new QPushButton(QStringLiteral("Test"), priv_data->parent);
	qt_button->show();

	printf("Push button ptr %p\n", (void*)qt_button);

	// TODO: Smarter allocator than just using new all the time

	PUPushButton* button = new PUPushButton;

	button->base = new PUWidget;

	widget_setup(button->base, (void*) static_cast<QObject*>(qt_button));

	return button;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PU s_pu = {
	PU_VERSION(0, 0, 1),

	// user facing

	push_button_create,

	// funcs

	&s_widgetFuncs,
	&s_pushButtonFuncs,

	// private data
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" PU* pu_get_instance(void* parent) {
    PrivateData* priv_data = new PrivateData;
    PU* pu_inst = new PU;

    priv_data->parent = (QWidget*)parent;
    priv_data->user_data = nullptr;

    memcpy(pu_inst, &s_pu, sizeof(PU));

    pu_inst->privd = (void*)priv_data;

    return pu_inst;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


