#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const float margins = 5.0f;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HexMemoryData
{
    unsigned char* data;
    int dataSize;
    int addressSize;
    uint64_t startAddress;
    uint64_t endAddress;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;

    HexMemoryData* userData = (HexMemoryData*)malloc(sizeof(HexMemoryData));
    userData->data = (unsigned char*)malloc(1024 * 1024);
    userData->startAddress = 0x6000;
    userData->addressSize = 2;

    for (int i = 0; i < 1024 * 1024; ++i)
        userData->data[i] = rand() & 0xff;

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void printAddressLine(PDUI* uiFuncs, uint64_t address, int adressSize)
{
    char tempBuffer[128] = { 0 };

    switch (adressSize)
    {
        case 1:
            sprintf(tempBuffer, "0x%02x", (uint8_t)address); break;
        case 2:
            sprintf(tempBuffer, "0x%04x", (uint16_t)address); break;
        case 4:
            sprintf(tempBuffer, "0x%08x", (uint32_t)address); break;
        case 8:
            sprintf(tempBuffer, "0x%16llx", (int64_t)address); break;
    }

    uiFuncs->text(tempBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawAddresses(HexMemoryData* data, PDUI* uiFuncs, int lineCount, unsigned int charsPerLine)
{
    uint64_t address = data->startAddress;
    int adressSize = data->addressSize;

    for (int i = 0; i < lineCount; ++i)
    {
        printAddressLine(uiFuncs, address, adressSize);
        address += charsPerLine;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawNumbers(HexMemoryData* userData, PDUI* uiFuncs, int lineCount, unsigned int charsPerLine, float margin)
{
    const unsigned char* data = userData->data;

    for (int i = 0; i < lineCount; ++i)
    {
        PDVec2 textStart = uiFuncs->getCursorPos();
        textStart.x = margin;

        for (unsigned int p = 0; p < charsPerLine; ++p)
        {
            uiFuncs->setCursorPos(textStart);
            uiFuncs->text("%02x ", *data++);
            textStart.x += 26.0f;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawChars(HexMemoryData* userData, PDUI* uiFuncs, int lineCount, unsigned int charsPerLine, float margin)
{
    const unsigned char* data = userData->data;

    for (int i = 0; i < lineCount; ++i)
    {
        PDVec2 textStart = uiFuncs->getCursorPos();
        textStart.x = margin;

        for (unsigned int p = 0; p < charsPerLine; ++p)
        {
            unsigned char c = *data++;
            uiFuncs->setCursorPos(textStart);
            if (c >= 32 && c < 128)
                uiFuncs->text("%c", c);
            else
                uiFuncs->text(".", c);
            textStart.x += 10.0f;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void drawUI(HexMemoryData* data, PDUI* uiFuncs)
{
    PDVec2 textStart = uiFuncs->getCursorPos();
    PDVec2 windowSize = uiFuncs->getWindowSize();
    windowSize.y -= textStart.y;

    if (!data->data)
        return;

    const float fontHeight = uiFuncs->getFontHeight();
    const float fontWidth = uiFuncs->getFontWidth();

    int drawableLineCount = (int)(windowSize.y / fontHeight);
    float drawableChars = (float)(int)(windowSize.x / fontWidth);

    // margins between the rows

    drawableChars -= margins * 3.0f;
    //int addressTextSize = ((int)fontWidth) * (data->addressSize * 2) + 2;

    //printf("%d %d\n", addressTextSize, (int)drawableChars);

    //if (addressTextSize > drawableChars)
    //	return;

    drawAddresses(data, uiFuncs, drawableLineCount, 8);

    uiFuncs->setCursorPos(textStart);

    drawNumbers(data, uiFuncs, drawableLineCount, 8, 76.0f);

    uiFuncs->setCursorPos(textStart);

    drawChars(data, uiFuncs, drawableLineCount, 8, 300.0f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateMemory(HexMemoryData* userData, PDReader* reader)
{
    void* data;
    uint64_t size = 0;

    //printf("%s(%d) update memory\n", __FILE__, __LINE__);


    if (PDRead_findData(reader, &data, &size, "data", 0) == PDReadStatus_notFound)
        return;

    printf("%s(%d) update memory\n", __FILE__, __LINE__);

    free(userData->data);

    userData->data = (unsigned char*)malloc((size_t)size);
    memcpy(userData->data, data, (size_t)size);

    printf("updating data %p %d\n", userData->data, (int)size);

    userData->dataSize = (int)size;
    userData->data = (unsigned char*)data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* inEvents, PDWriter* outEvents)
{
    uint32_t event;

    (void)outEvents;

    HexMemoryData* data = (HexMemoryData*)userData;

    /*
       PDVec2 windowSize = uiFuncs->getWindowSize();
       const float fontHeight = uiFuncs->getFontHeight();
       const float fontWidth = uiFuncs->getFontWidth();

       int drawableLineCount = (int)(windowSize.y / fontHeight);
       int drawableChars = (int)(windowSize.x / fontWidth);
     */

    // Loop over all the in events

    while ((event = PDRead_getEvent(inEvents)) != 0)
    {
        switch (event)
        {
            case PDEventType_setMemory:
            {
                updateMemory(data, inEvents);
                break;
            }
        }
    }

    drawUI(data, uiFuncs);

    /*
       PDWrite_eventBegin(outEvents, PDEventType_getMemory);
       PDWrite_u16(outEvents, "address", 0x6000);
       PDWrite_u16(outEvents, "size", (uint16_t)(drawableLineCount * drawableChars));
       PDWrite_eventEnd(outEvents);
     */

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDViewPlugin plugin =
{
    "Hex Memory View",
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

