#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// constants, margins, etc

const float s_markerMargin = 22.0f; // Area on left-side reserved for breakpoints, markers, etc
const float s_areaLinesToText = 10.0f;  // Area between line numbers and the text

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Line
{
    const char* text;
    bool breakpoint;        // TODO: Flags
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct File
{
    Line* lines;
    int lineCount;
    char* startData;
    char* filename;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SourceCodeData
{
    int cursorPos;
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
    Line* lines;
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

    file->lines = lines = (Line*)malloc(sizeof(Line) * size);
    memset(lines, 0, sizeof(Line) * size);

    lines[0].text = target;
    lineCount++;

    while (target < targetEnd)
    {
        char c = *target;

        if (*target == '\r')
        {
            *target++ = 0;
            c = *target;
        }

        if (*target == '\n')
        {
            *target = 0;
            lines[lineCount++].text = target + 1;
        }

        target++;
    }

    *target++ = 0;

    file->filename = strdup(filename);
    file->lineCount = lineCount;
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
    (void)uiFuncs;
    (void)areaWidth;

    /*
       PDVec2 pos = uiFuncs->getCursorPos();
       PDRect rect = { pos.x - 4, pos.y - 14, areaWidth + 4, -1 };
       uiFuncs->fillRect(rect, PD_COLOR_32(100, 100, 100, 127));
     */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawCursor(PDUI* uiFuncs, float lineAreaSize)
{
    PDRect rect;
    PDVec2 pos = uiFuncs->getCursorPos();
    rect.x = pos.x;
    rect.y = pos.y;
    rect.width = 10;
    rect.height = 14;

    // Cursor

    uiFuncs->fillRect(rect, PD_COLOR_32(200, 0, 0, 127));

    // Mark in line area

    rect.x = s_markerMargin;
    rect.y = pos.y;
    rect.width = lineAreaSize;
    rect.height = 14;

    uiFuncs->fillRect(rect, PD_COLOR_32(120, 120, 120, 127));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawLineArea(PDUI* uiFuncs, int offset, int lineCount, int maxLineCount)
{
    char formatString[64];
    int numCount = countNumbers(maxLineCount);
    sprintf(formatString, "%%%dd", numCount);

    for (int i = offset + 1; i < (offset + 1 + lineCount); ++i)
    {
        uiFuncs->setCursorPosX(s_markerMargin);
        uiFuncs->text(formatString, i);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawBreakpoint(PDUI* uiFuncs)
{
    PDRect rect;
    PDVec2 pos = uiFuncs->getCursorPos();
    rect.x = 2;
    rect.y = pos.y;
    rect.width = s_markerMargin - 2;
    rect.height = 14;
    uiFuncs->fillRect(rect, PD_COLOR_32(100, 100, 200, 127));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawLines(PDUI* uiFuncs, SourceCodeData* data, float lineStart, float lineAreaSize, int offset, int lineCount)
{
    Line* lines = data->file.lines;

    for (int i = offset; i < offset + lineCount; ++i)
    {
        uiFuncs->setCursorPosX(lineStart);

        if (i == data->cursorPos)
            drawCursor(uiFuncs, lineAreaSize);

        if ((i + 1) == (int)data->line)
        {
            PDRect rect;
            PDVec2 pos = uiFuncs->getCursorPos();
            rect.x = pos.x;
            rect.y = pos.y;
            rect.width = -1;
            rect.height = 14;
            uiFuncs->fillRect(rect, PD_COLOR_32(200, 0, 0, 127));
        }

        if (lines[i].breakpoint)
            drawBreakpoint(uiFuncs);

        //printf("%d - %d %d\n", i, lines[i][0], lines[i][1]);

        if (!lines[i].text)
            break;

        uiFuncs->text(lines[i].text);
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

    // Start position of the text

    float textStartX = s_markerMargin + s_areaLinesToText + areaWidth;

    // if we are fully with in the drawing range we just render as is

    if (lineCount < drawableLineCount)
    {
        uiFuncs->setCursorPos(textStart);
        drawLineArea(uiFuncs, 0, lineCount, lineCount);

        uiFuncs->setCursorPos(textStart);

        drawLines(uiFuncs, data, textStartX, areaWidth, 0, lineCount);
    }
    else
    {
        // We want the breakline to be in the center of the screen so calculate the pos as such

        int line = (int)data->line;

        if (line == 0)
            line = data->cursorPos;

        int lineStart = line - drawableLineCount / 2;

        if (lineStart < 0)
            lineStart = 0;

        // Make sure we don't draw too many lines

        if (lineStart + drawableLineCount > lineCount)
            drawableLineCount = lineCount;

        drawLineAreaBG(uiFuncs, areaWidth);
        drawLineArea(uiFuncs, lineStart, drawableLineCount, lineCount);

        uiFuncs->setCursorPos(textStart);

        drawLines(uiFuncs, data, textStartX, areaWidth, lineStart, drawableLineCount);
    }

    (void)textStart;

    // Draw line column

    uiFuncs->setCursorPos(textStart);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateKeyboard(SourceCodeData* data, PDUI* uiFuncs)
{
    int cursorPos = data->cursorPos;

    if (uiFuncs->isKeyDown(PDKEY_UP, 1))
    {
        cursorPos--;

        if (cursorPos < 0)
            cursorPos = 0;
    }

    if (uiFuncs->isKeyDown(PDKEY_DOWN, 1))
    {
        cursorPos++;

        const int lineCount = data->file.lineCount - 1;

        if (cursorPos > lineCount)
            cursorPos = lineCount;
    }

    data->cursorPos = cursorPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void toggleBreakpointCurrentLine(SourceCodeData* data, PDWriter* writer)
{
    // TODO: Currenty we don't handly if we set breakpoints on a line we can't

    PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
    PDWrite_string(writer, "filename", data->file.filename);
    PDWrite_u32(writer, "line", (unsigned int)data->cursorPos + 1);
    PDWrite_eventEnd(writer);

    data->file.lines[data->cursorPos].breakpoint = !data->file.lines[data->cursorPos].breakpoint;
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
    PDWrite_u8(writer, "dummy_get_location", 0); // TODO: Remove me
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

