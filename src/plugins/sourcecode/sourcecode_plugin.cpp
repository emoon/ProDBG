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

    // TODO: Temp testing code

    parseFile(&userData->file, "examples/crashing_native/crash2.c");

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
// Count how many numbers we have in lineCount

int countNumbers(int maxLineCount)
{
    int count = 0;

    while (maxLineCount != 0)
    {
        maxLineCount /= 10;
        count++;
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This code will calculate the width of the max line count in the file and use that as the width
// for the line area

static float calcLineAreaWidth(PDUI* uiFuncs, int lineCount)
{
    char lineBuffer[128];
    sprintf(lineBuffer, "%d", lineCount);
    return uiFuncs->getTextWidth(lineBuffer, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawLineAreaBG(PDUI* uiFuncs, float areaWidth)
{
    PDVec2 pos = uiFuncs->getCursorPos();
    PDRect rect = { pos.x - 4, pos.y - 14, areaWidth + 4, -1 };
    uiFuncs->fillRect(rect, PD_COLOR_32(100, 100, 100, 127));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawLineArea(PDUI* uiFuncs, int offset, int lineCount, int maxLineCount)
{
    char formatString[64];
    int numCount = countNumbers(maxLineCount);
    sprintf(formatString, "%%%dd", numCount);

    for (int i = 0; i < lineCount; ++i)
    {
        uiFuncs->text(formatString, offset + i + 1);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawLines(PDUI* uiFuncs, SourceCodeData* data, float lineStart, int offset, int lineCount)
{
    const char** lines = (const char**)data->file.lines;

    for (int i = offset; i < (offset + lineCount); ++i)
    {
        uiFuncs->setCursorPosX(lineStart);

        if ((i + 1) == (int)data->line)
        {
            PDRect rect;
            PDVec2 pos = uiFuncs->getCursorPos();
            rect.x = pos.x;
            rect.y = pos.y - 11;
            rect.width = -1;
            rect.height = 14;
            uiFuncs->fillRect(rect, PD_COLOR_32(200, 0, 0, 127));
        }

        uiFuncs->text(lines[i]);
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showInUI(SourceCodeData* data, PDUI* uiFuncs)
{
    uiFuncs->columns(1, "sourceview", true);

    // calculate the range we can show

    PDVec2 textStart = uiFuncs->getCursorPos();
    PDVec2 windowSize = uiFuncs->getWindowSize();
    windowSize.y -= textStart.y;

    const float fontHeight = uiFuncs->getFontHeight();
    int drawableLineCount = (int)(windowSize.y / fontHeight);

    int lineCount = data->file.lineCount;

    // Draw the line

    float areaWidth = calcLineAreaWidth(uiFuncs, lineCount) + 4;

    // if we are fully with in the drawing range we just render as is

    if (lineCount < drawableLineCount)
    {
        drawLineAreaBG(uiFuncs, areaWidth);
        drawLineArea(uiFuncs, 0, lineCount, lineCount);

        uiFuncs->setCursorPos(textStart);

        drawLines(uiFuncs, data, areaWidth + 10, 0, lineCount);
    }
    else
    {
        // Here we need to find the starting point of the

        drawLineAreaBG(uiFuncs, areaWidth);
        drawLineArea(uiFuncs, 0, drawableLineCount, lineCount);

        uiFuncs->setCursorPos(textStart);

        drawLines(uiFuncs, data, areaWidth + 10, 0, drawableLineCount);
    }

    (void)textStart;

    // Draw line column



    uiFuncs->setCursorPos(textStart);

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
        }
    }

    showInUI(data, uiFuncs);

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

