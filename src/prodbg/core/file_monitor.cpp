#include "file_monitor.h"
#include <foundation/memory.h>
#include <foundation/array.h>
#include <foundation/event.h>
#include <foundation/fs.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FileNotificationData
{
	const char* path;
	const char* fileFilters;
	FMCallback callback;
	void* userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct FileNotificationData* s_notficationData = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileMonitor_addPath(const char* path, const char* fileFilters, FMCallback callback, void* userData)
{
	FileNotificationData data = { path, fileFilters, callback, userData };

	array_push(s_notficationData, data);

	fs_monitor(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileMonitor_removePath(const char* path)
{
	int size = array_size(s_notficationData);

	for (int i = 0; i < size; ++i)
	{
		if (!strcmp(s_notficationData[i].path, path))
		{
			array_erase(s_notficationData, i);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateCallbacks(event_t* event)
{
	int size = array_size(s_notficationData);
	const char* filename = event->payload;

	for (int i = 0; i < size; ++i)
	{
		FileNotificationData* noteData = &s_notficationData[i];

		// TODO: handle file filters

		if (!strstr(filename, noteData->path))
			continue;

		noteData->callback(noteData->userData, filename, event->id);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileMonitor_update()
{
	event_block_t* block = 0;
	event_t* event = 0;

	block = event_stream_process(fs_event_stream()); 

 	while ((event = event_next(block, event)))
	{
		switch (event->id)
		{
			case FOUNDATIONEVENT_FILE_CREATED:
			case FOUNDATIONEVENT_FILE_DELETED:
			case FOUNDATIONEVENT_FILE_MODIFIED:
				updateCallbacks(event);
				break;
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
void FileMonitor_close()
{
	array_deallocate(s_notficationData);  
}


