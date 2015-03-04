#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HexMemoryData
{
    unsigned char* data;
    int dataSize;
    int addressSize;
    char startAddress[64];
    char endAddress[64];
    bool requestData;
    uint64_t sa;
    uint64_t ea;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    (void)uiFuncs;

    HexMemoryData* userData = (HexMemoryData*)malloc(sizeof(HexMemoryData));
    memset(userData, 0, sizeof(sizeof(HexMemoryData)));

    strcpy(userData->startAddress, "0x00000000");
    strcpy(userData->endAddress, "0x00001000");

    userData->sa = 0;
    userData->ea = 0x00000fff;

    userData->data = (unsigned char*)malloc(1024 * 1024);
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

static void getAddressLine(char* adressText, uint64_t address, int adressSize)
{
    switch (adressSize)
    {
        case 1:
            sprintf(adressText, "0x%02x", (uint8_t)address); break;
        case 2:
            sprintf(adressText, "0x%04x", (uint16_t)address); break;
        case 4:
            sprintf(adressText, "0x%08x", (uint32_t)address); break;
        case 8:
            sprintf(adressText, "0x%16llx", (int64_t)address); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawData(HexMemoryData* data, PDUI* uiFuncs, int lineCount, int charsPerLine)
{
    uint64_t address = (uint64_t)strtol(data->startAddress, 0, 16);
    int adressSize = data->addressSize;
    uint8_t* memoryData = data->data;

    if (charsPerLine > 1024)
    	charsPerLine = 1024;

    for (int i = 0; i < lineCount; ++i)
    {
    	char addressText[64] = { 0 };
    	char hexData[1024];
    	char charData[1024];

    	char* hexText = hexData;
    	char* charText = charData;

    	// Get Address

		getAddressLine(addressText, address, adressSize);

		// Get Hex and chars

    	for (int p = 0; p < charsPerLine; ++p)
		{
			uint8_t c = memoryData[p];

			sprintf(hexText, "%02x ", c); 

            if (c >= 32 && c < 128)
            	*charText++ = (char)c;
            else
            	*charText++ = '.';

			hexText += 3;
		}

		*hexText = 0; 
		*charText = 0; 

		uiFuncs->text("%s: %s %s", addressText, hexData, charData);

        address += (uint32_t)charsPerLine;
        memoryData += charsPerLine;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void drawUI(HexMemoryData* data, PDUI* uiFuncs)
{
	uiFuncs->pushItemWidth(100);
    uiFuncs->inputText("Start Address", data->startAddress, sizeof(data->startAddress), PDInputTextFlags_CharsHexadecimal, 0, 0);
    uiFuncs->sameLine(0, -1);
    uiFuncs->inputText("End Address", data->endAddress, sizeof(data->endAddress), 0, 0, 0);
    uiFuncs->popItemWidth();

    PDVec2 size = { 0.0f, 0.0f };

    long startAddress = strtol(data->startAddress, 0, 16);
    long endAddress = strtol(data->endAddress, 0, 16);

    if ((endAddress >= startAddress) && !data->data)
    	return;

	if (data->sa != (uint64_t)startAddress)
	{
		data->requestData = true;
		data->sa = (uint64_t)startAddress;
	}

	if (data->ea != (uint64_t)endAddress)
	{
		data->requestData = true;
		data->ea = (uint64_t)endAddress;
	}

    //PDVec2 textStart = uiFuncs->getCursorPos();
    PDVec2 windowSize = uiFuncs->getWindowSize();

    uiFuncs->beginChild("child", size, false, 0);

    const float fontWidth = uiFuncs->getFontWidth();

    float drawableChars = (float)(int)(windowSize.x / (fontWidth + 23));

    printf("%f\n", drawableChars);

    // margins between the rows

    //int addressTextSize = ((int)fontWidth) * (data->addressSize * 2) + 2;

    //printf("%d %d\n", addressTextSize, (int)drawableChars);

    //if (addressTextSize > drawableChars)
    //	return;

    int drawableLineCount = (int)((endAddress - startAddress) / (int)drawableChars); 

    //printf("%d %d %d %d\n", drawableLineCount, (int)endAddress, (int)startAddress, (int)drawableChars);

    drawData(data, uiFuncs, drawableLineCount, (int)drawableChars);

/*
    uiFuncs->setCursorPos(textStart);

    drawNumbers(data, uiFuncs, drawableLineCount, (uint32_t)drawableChars, 76.0f);

    float offset = drawableChars * (fontWidth + 20);

    uiFuncs->setCursorPos(textStart);

    drawChars(data, uiFuncs, drawableLineCount, (uint32_t)drawableChars, offset);

*/
    uiFuncs->endChild();
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

