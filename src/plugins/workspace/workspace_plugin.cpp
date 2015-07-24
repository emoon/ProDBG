#include "pd_view.h"
#include "pd_backend.h"
#include "pd_host.h"
#include <stdlib.h>
#include <stdio.h>
#include <foundation/foundation.h>
#include <foundation/string.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDialogFuncs* s_dialogFuncs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct DirEntry
{
	char* directoryName;
	struct DirEntry** dirs;
	char** files;
	char** filesBase;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct WorkspaceData
{
    application_t app;
    char** paths;
	DirEntry** dirs;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Lots of (temp) memory allocs in this code, would be good to cleaan up

static void populatePathRecursive(DirEntry* entry, char* fullPath, char* sub)
{
	entry->directoryName = sub; 

	char** files = entry->filesBase = fs_files(fullPath);

	for (int i = 0; i < array_size(files); ++i)
		array_push(entry->files, path_merge(fullPath, files[i]));

	char** dirs = fs_subdirs(fullPath);

	int subDirCount = array_size(dirs);

	for (int i = 0; i < subDirCount; ++i)
	{
		if (dirs[i][0] == '.')
			continue;

		DirEntry* newEntry = (DirEntry*)malloc(sizeof(DirEntry));
		memset(newEntry, 0, sizeof(DirEntry));
		array_push(entry->dirs, newEntry);
		char* full = path_merge(fullPath, dirs[i]);
		populatePathRecursive(newEntry, full, dirs[i]);
		string_deallocate(full);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void populatePath(WorkspaceData* data, char* path)
{
	DirEntry* dirs = (DirEntry*)malloc(sizeof(DirEntry));
	memset(dirs, 0, sizeof(DirEntry));

	array_push(data->dirs, dirs);

	uint32_t offset = string_rfind(path, '/', STRING_NPOS);

#ifdef _WIN32
	if (offset == STRING_NPOS)
		offset = string_rfind(path, '\\', 0);
#endif

	if (offset == STRING_NPOS)
	{
		printf("Path %s looks broken\n", path);
		return;
	}

	// must clone here as the recursive function will keep the pointer
	char* subPath = string_clone(path + offset + 1);

	path[offset] = 0;

	populatePathRecursive(dirs, path, subPath); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void recursiveDrawTree(DirEntry* entry, PDUI* uiFuncs, PDWriter* writer)
{
	int dirCount = array_size(entry->dirs);

	for (int i = 0; i < dirCount; ++i)
	{
		DirEntry* dir = entry->dirs[i];

		if (uiFuncs->treeNodeStr(dir->directoryName, dir->directoryName))
		{
			recursiveDrawTree(dir, uiFuncs, writer);
			uiFuncs->treePop();
		}
	}

	int fileCount = array_size(entry->filesBase);

	for (int i = 0; i < fileCount; ++i)
	{
		PDVec2 size = { 0.0f, 0.0f };

		if (uiFuncs->selectable(entry->filesBase[i], false, 0, size))
		{
       		PDWrite_eventBegin(writer, PDEventType_setSourceCodeFile);
       		PDWrite_string(writer, "filename", entry->files[i]);
       		PDWrite_eventEnd(writer);

       		printf("sending event\n");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* reader, PDWriter* writer)
{
    uint32_t event = 0;

    WorkspaceData* data = (WorkspaceData*)userData;

    (void)uiFuncs;
    (void)reader;
    (void)writer;

	PDVec2 size = { 0.0f, 0.0f };

    if (uiFuncs->button("...", size))
	{
		char outputPath[4096];
		if (s_dialogFuncs->selectDirectory(outputPath))
		{
			array_push(data->paths, string_clone(outputPath));
			populatePath(data, outputPath);
		}
	}

	for (int i = 0; i < array_size(data->dirs); ++i)
		recursiveDrawTree(data->dirs[i], uiFuncs, writer);

    while ((event = PDRead_getEvent(reader)) != 0)
    {
    	(void)event;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)uiFuncs;
    WorkspaceData* userData = (WorkspaceData*)malloc(sizeof(WorkspaceData));
    memset(userData, 0, sizeof(WorkspaceData));

    userData->app.name = "ProDBG_workspace";
    userData->app.short_name = "ProDBG_workspace";
    userData->app.config_dir = "ProDBG_workspace";
    userData->app.version = foundation_version();
    userData->app.flags = APPLICATION_UTILITY;
    userData->app.dump_callback = 0;

    foundation_initialize(memory_system_malloc(), userData->app);

	s_dialogFuncs = (PDDialogFuncs*)serviceFunc(PDDIALOGS_GLOBAL);

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
	WorkspaceData* data = (WorkspaceData*)userData;

	for (int i = 0; i < array_size(data->paths); ++i)
		string_deallocate(data->paths[i]);

	array_deallocate(data->paths);

    free(data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int saveState(void* userData, PDSaveState* ss)
{
	WorkspaceData* data = (WorkspaceData*)userData;

	for (int i = 0; i < array_size(data->paths); ++i)
	{
		PDIO_writeString(ss, "path");
		PDIO_writeString(ss, data->paths[i]);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int loadState(void* userData, PDLoadState* ls)
{
	char stringData[4096];

	WorkspaceData* data = (WorkspaceData*)userData;

	while (PDIO_readString(ls, stringData, sizeof(stringData)) != PDLoadStatus_outOfData)
	{
		if (string_equal(stringData, "path"))
		{
			PDIO_readString(ls, stringData, sizeof(stringData));
			array_push(data->paths, string_clone(stringData)); 
			populatePath(data, stringData);
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Workspace",
    createInstance,
    destroyInstance,
    update,
    saveState,
    loadState,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// workaround for foundation

int main(int, char**) { return 0; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* privateData)
{
	registerPlugin(PD_VIEW_API_VERSION, &plugin, privateData);
}

}


