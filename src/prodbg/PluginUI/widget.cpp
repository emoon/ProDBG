
#include "wrui.h"
#include "signal_wrappers.h"

#if 0

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int connect(void* sender, const char* id, void* reciver, void* func) {
	PUObject* object = (PUObject*)sender;
	QObject* q_obj = (QObject*)object->p;

	QSlotWrapperNoArgs* wrap = new QSlotWrapperNoArgs(reciver, (SignalNoArgs)func);

	QObject::connect(q_obj, id, wrap, SLOT(method()));
	/*
		return 1;
	} else {
		printf("wrui: unable to create connection between (%p - %s) -> (%p -> %p)\n");
		return 0;
	}
	*/
	return 0;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct PUObjectFuncs s_objFuncs = {
	0,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void object_setup(PUObject* object, void* data) {
	object->p = data;
	object->object_funcs = &s_objFuncs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void widget_setup(PUWidget* widget, void* data) {
	widget->object = new PUObject;
	object_setup(widget->object, data);
}

