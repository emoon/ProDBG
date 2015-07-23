#include "pd_view.h"
#include "pd_backend.h"
#include "pd_host.h"
#include <stdlib.h>
#include <stdio.h>
#include <foundation/foundation.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDialogFuncs* s_dialogFuncs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Entry
{
	const char* name;
	bool isDirectory;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TreeEntry
{
	int count;
	bool* foldedState;
	Entry* entries;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct WorkspaceData
{
    application_t app;
	int dummy;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* reader, PDWriter* writer)
{
    uint32_t event = 0;

    (void)userData;
    (void)uiFuncs;
    (void)reader;
    (void)writer;

    if (uiFuncs->button("...", (PDVec2) { 0.0f, 0.0f }))
	{
		char outputPath[4096];
		if (s_dialogFuncs->selectDirectory(outputPath))
		{
			char** files = fs_matching_files(outputPath, "^.*$", true);

			for (int i = 0; i < array_size(files); ++i)
			{
				if (files[i][0] == '.')
					continue;

				printf("%s\n", files[i]);
			}
		}
	}

    while ((event = PDRead_getEvent(reader)) != 0)
    {
    	(void)event;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;
    WorkspaceData* userData = (WorkspaceData*)malloc(sizeof(WorkspaceData));

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
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Workspace",
    createInstance,
    destroyInstance,
    update,
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


