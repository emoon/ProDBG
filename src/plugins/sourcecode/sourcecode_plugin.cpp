#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <scintilla/include/Scintilla.h>

struct SourceCodeData
{
    int dummy;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   static void* readFileFromDisk(const char* file, size_t* size)
   {
    size_t fileSize;
    char* data;
    FILE* f = fopen(file, "rb");

    if (!f)
    {
        printf("sourcecode_plugin: Unable to open file %s\n", file);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    fileSize = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    // pad the size a bit so we make sure to have the data null terminated
    data = (char*)malloc(fileSize + 16);
    data[fileSize] = 0;

    if ((fread((void*)data, 1, fileSize, f)) != fileSize)
    {
        free(data);
        fclose(f);
        printf("sourcecode_plugin: Unable to read the whole file %s to memory\n", file);
        return 0;
    }

 * size = fileSize;
    fclose(f);

    return data;
   }
 */


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;
    SourceCodeData* userData = (SourceCodeData*)malloc(sizeof(SourceCodeData));
    memset(userData, 0, sizeof(SourceCodeData));

    // TODO: Temp testing code

    //parseFile(&userData->file, "examples/crashing_native/crash2.c");

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExceptionLocation(SourceCodeData* data, PDReader* inEvents)
{
    const char* filename;
    uint32_t line;

    (void)data;

    // TODO: How to show this? Tell user to switch to disassembly view?

    if (PDRead_findString(inEvents, &filename, "filename", 0) == PDReadStatus_notFound)
        return;

    if (PDRead_findU32(inEvents, &line, "line", 0) == PDReadStatus_notFound)
        return;

    //parseFile(&data->file, filename);

    //data->line = line;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(SourceCodeData* data, PDUI* uiFuncs)
{
    (void)data;
    //uiFuncs->columns(1, "sourceview", true);
    PDSCInterface* scFuncs = uiFuncs->scInputText("test", 800, 700, 0, 0);

	const char* testText = "Test\nTest2\nTest3\n\0";

	static bool hasSentText = false;

	if (!hasSentText)
	{
		PDUI_SCSendCommand(scFuncs, SCI_ADDTEXT, strlen(testText), (intptr_t)testText);
		hasSentText = true;
	}

	PDUI_SCUpdate(scFuncs);
	PDUI_SCDraw(scFuncs);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateKeyboard(SourceCodeData* data, PDUI* uiFuncs)
{
    (void)data;
    (void)uiFuncs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void toggleBreakpointCurrentLine(SourceCodeData* data, PDWriter* writer)
{
    (void)data;
    (void)writer;
    /*
       // TODO: Currenty we don't handly if we set breakpoints on a line we can't

       PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
       PDWrite_string(writer, "filename", data->file.filename);
       PDWrite_u32(writer, "line", (unsigned int)data->cursorPos + 1);
       PDWrite_eventEnd(writer);

       data->file.lines[data->cursorPos].breakpoint = !data->file.lines[data->cursorPos].breakpoint;
     */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* writer)
{
    uint32_t event;

    (void)uiFuncs;

    SourceCodeData* data = (SourceCodeData*)userData;

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setExceptionLocation:
            {
                setExceptionLocation(data, inEvents);
                break;
            }

            case PDEventType_toggleBreakpointCurrentLine:
            {
                toggleBreakpointCurrentLine(data, writer);
                break;
            }
        }
    }

    updateKeyboard(data, uiFuncs);

    showInUI(data, uiFuncs);

    PDWrite_eventBegin(writer, PDEventType_getExceptionLocation);
    PDWrite_eventEnd(writer);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Source Code View",
    createInstance,
    destroyInstance,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* privateData)
{
	registerPlugin(PD_VIEW_API_VERSION, &plugin, privateData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

