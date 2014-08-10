#include "PluginUI.h"
#include "CustomView.h"
#include <PDUI.h>
#include <QTreeWidget>
#include <QDockWidget>
#include <QLayout>
#include <QVBoxLayout>
#include "core/Log.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PrivateData
{
	QDockWidget* dock;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDUIListView listview_create(void* privateData, const char** names, int id)
{
	(void)names;
	(void)id;
	PrivateData* data = (PrivateData*)privateData;
	QTreeWidget* listWidget = new QTreeWidget(data->dock);
	QStringList headers;

	while (*names)
	{
		headers << *names;
		names++;
	}

	listWidget->setHeaderLabels(headers);
	data->dock->setWidget(listWidget);

	log_info("creating listview");
	return (PDUIListView)listWidget; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int listview_clear(void*, PDUIListView handle)
{
	QTreeWidget* listWidget = (QTreeWidget*)handle;
	listWidget->clear();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int listview_item_add(void*, PDUIListView handle, const char** items)
{
	QTreeWidget* listWidget = (QTreeWidget*)handle;
    QTreeWidgetItem* newItem = new QTreeWidgetItem;
	QStringList list;

	while (*items)
	{
		list << *items;
		items++;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem((QTreeWidget*)0, list);
	listWidget->insertTopLevelItem(0, item);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int listview_item_remove(void*, PDUIListView handle, int index)
{
	QTreeWidget* listWidget = (QTreeWidget*)handle;
	//listWidget->removeItemWidget(listWidget->itemAt(0, index));
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDUICustomView customview_create(void* privateData, void* userData, PDCustomDrawCallback callback)
{
	PrivateData* privData = (PrivateData*)privateData; 
	CustomView* customView = new CustomView(privData->dock, userData, callback);
	privData->dock->setWidget(customView);
	return (PDUICustomView)customView; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void customview_repaint(void*, PDUICustomView view)
{
	CustomView* customView = (CustomView*)view;
	customView->update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI_init(const char* type, QWidget* parent, PDUI* uiInstance)
{
	PrivateData* privData = new PrivateData;
	privData->dock = new QDockWidget(type, parent);
	privData->dock->resize(200, 200);
	privData->dock->show();

	log_info("PluginUI_init\n");

	memset(uiInstance, 0, sizeof(PDUI));

	uiInstance->listview_create = listview_create;
	uiInstance->listview_clear = listview_clear;
	uiInstance->listview_item_add = listview_item_add;
	uiInstance->customview_create = customview_create;
	uiInstance->customview_repaint = customview_repaint;

	//uiInstance->listview_item_remove = listview_item_remove;
	//uiInstance->listview_item_text_get = 0;
	uiInstance->privateData = privData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

