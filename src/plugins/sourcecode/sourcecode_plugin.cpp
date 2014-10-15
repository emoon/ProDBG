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
    File file;  // todo: support more files
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* readFileFromDisk(const char* file, size_t* size)
{
    size_t fileSize;
    char* data;
    FILE* f = fopen(file, "rb");

    if (!f)
    {
        printf("SICO: Unable to open file %s\n", file);
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
        printf("SICO: Unable to read the whole file %s to memory\n", file);
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
//

#if 0

static void drawCallback(void* userData, PDRect* viewRect, PDUIPainter* painter)
{
    int fontX;
    int fontY;
    int exceptionLine = (int)data->line - 2;

    PDUIPaint_fontMetrics(painter, &fontX, &fontY);
    PDUIPaint_fillRect(painter, viewRect, 0xffffff);
    PDUIPaint_setPen(painter, 0);

    // calc how many lines we can render

    int maxLineCount = (viewRect->height - viewRect->y) / fontY;

    // in case file has less lines than we can display we render the whole file

    if (data->file.lineCount < maxLineCount)
    {
        int y = 20;

        for (int i = 0, end = data->file.lineCount; i < end; ++i)
        {
            PDUIPaint_drawText(painter, viewRect->x, y, data->file.lines[i]);

            if (i == exceptionLine)
            {
                printf("crash at %d -  %s\n", i, data->file.lines[i]);
                PDRect rect = { viewRect->x, y, viewRect->width, y + fontY };
                PDUIPaint_fillRect(painter, &rect, 0xf9afafaf);
            }

            y += fontX + 2;
        }
    }


    // figure out which line to start at so we can keep the exception line in the center

    //PDUIPaint_setPen(painter, 0x1fffffff);
    //PDUIPaint_drawText(painter, viewRect->x, y, pch);

    //y += fontX + 2;
    * /
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;
    SourceCodeData* userData = (SourceCodeData*)malloc(sizeof(SourceCodeData));
    memset(userData, 0, sizeof(SourceCodeData));

    //userData->view = PDUICustomView_create(uiFuncs, userData, drawCallback);

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

    parseFile(&data->file, filename);

    data->line = line;
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
                setExceptionLocation(data, inEvents); break;
        }
    }

    PDWrite_eventBegin(writer, PDEventType_getExceptionLocation);
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

