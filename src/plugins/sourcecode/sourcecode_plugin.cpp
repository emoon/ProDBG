#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct File
{
    char** lines;
    int lineCount;
    char* startData;
    char* filename;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SourceCodeData
{
    uint32_t line;
    File file;  // TODO: support more files
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    *size = fileSize;
    fclose(f);

    return data;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Do not read whole file to memory?
// TODO: Have a cache of files

void parseFile(File* file, const char* filename)
{
    char* target;
    char* targetEnd;
    char** lines;
    size_t size;
    int lineCount = 0;

    if (file->filename)
    {
        if (!strcmp(filename, file->filename))
            return;
    }

    if (!(target = (char*)readFileFromDisk(filename, &size)))
        return;

    if (file->startData)
    {
        free(file->lines);
        free(file->startData);
        free(file->filename);
    }

    file->startData = target;
    targetEnd = target + size;

    // so this is really waste of memory but will do for now

    file->lines = lines = (char**)malloc(sizeof(char*) * size);
    lines[0] = target;
    lineCount++;

    while (target < targetEnd)
    {
        if (*target == '\r')
            *target++ = 0;

        if (*target == '\n')
        {
            *target++ = 0;
            lines[lineCount++] = target;
        }

        target++;
    }

    file->filename = strdup(filename);
    file->lineCount = lineCount;

    printf("found %d lines in %s\n", lineCount, filename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;
    SourceCodeData* userData = (SourceCodeData*)malloc(sizeof(SourceCodeData));
    memset(userData, 0, sizeof(SourceCodeData));

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

    // TODO: How to show this? Tell user to switch to disassembly view?

    if (PDRead_findString(inEvents, &filename, "filename", 0) == PDReadStatus_notFound)
        return;

    if (PDRead_findU32(inEvents, &line, "line", 0) == PDReadStatus_notFound)
        return;

    printf("filename %s\n", filename);

    parseFile(&data->file, filename);

    data->line = line;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(SourceCodeData* data, PDUI* uiFuncs)
{
    uiFuncs->columns(1, "sourceview", true);

    // TODO: Will be really bad with big files, need to handle proper range and such here

    const char** lines = (const char**)data->file.lines;
    int lineCount = data->file.lineCount;

    uiFuncs->text("");

    for (int i = 0; i < lineCount; ++i)
    {
        if ((i + 1) == (int)data->line)
		{
			PDRect rect;
			PDVec2 pos = uiFuncs->getCursorPos();
        	rect.x = pos.x;
        	rect.y = pos.y - 11;
        	rect.width = -1;
        	rect.height = 20;
        	uiFuncs->fillRect(rect, PD_COLOR_32(255, 0, 0, 127));
        	//uiFuncs->text(*lines);
			//uiFuncs->setCursorPos(pos);
        	//uiFuncs->text(">"); 
        } 

        uiFuncs->text(*lines);

        lines++;
    }
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
                showInUI(data, uiFuncs);
                break;
            }
        }
    }

    PDWrite_eventBegin(writer, PDEventType_getExceptionLocation);
    PDWrite_u8(writer, "dummy", 0); // TODO: Remove me
    PDWrite_eventEnd(writer);

    //uiFuncs->button("test test");
    //uiFuncs->text("funy text %f", 12.02f);

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

