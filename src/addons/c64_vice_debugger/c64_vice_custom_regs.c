#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "pd_view.h"
#include "pd_backend.h"
#include "c64_vice_custom_regs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct CustomRegsData
{
    uint8_t regs[0x30];
    bool hasMemory;
    bool requestMemory;
    uint64_t location;
} CustomRegsData;

static PDColor s_colorRed = PDUI_COLOR(255, 0, 0, 0); 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printSpriteX(PDUI* uiFuncs, uint32_t v, uint32_t i, uint32_t d010)
{
    uiFuncs->text("$d0%02x - Sprite #%d X-coordinate", i * 2, i);
    uiFuncs->nextColumn();

    uint32_t v2 = v + (((d010 >> i) & 1) << 8);

    uiFuncs->textColored(s_colorRed, "%04d ($%02x)", v, v);
    uiFuncs->sameLine(0, -1);
    uiFuncs->text("(0-7 bits)");
    uiFuncs->sameLine(0, -1);
    uiFuncs->textColored(s_colorRed, "%04d ($%02x)", v2, v2);
    uiFuncs->sameLine(0, -1);
    uiFuncs->text("(0-8 bits) with bit from $d010");
    uiFuncs->sameLine(0, -1);
    uiFuncs->nextColumn();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printSpriteY(PDUI* uiFuncs, uint16_t v, uint32_t i)
{
    uiFuncs->text("$d0%02x - Sprite #%d Y-coordinate", 1 + i * 2, i);
    uiFuncs->nextColumn();

    uiFuncs->textColored(s_colorRed, "%04d ($%02x)", v, v);
    uiFuncs->sameLine(0, -1);
    uiFuncs->text("(0-7 bits)");
    uiFuncs->nextColumn();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(PDUI* uiFuncs, ServiceFunc* serviceFunc)
{
    (void)serviceFunc;
    CustomRegsData* userData = (CustomRegsData*)malloc(sizeof(CustomRegsData));

    static uint8_t tempData[] =
    {
        0x18, 0xda, 0x48, 0xda, 0x78, 0xda, 0xa8, 0xda,
        0xd8, 0xda, 0x08, 0xda, 0x38, 0xda, 0x00, 0x00,
        0x60, 0x1b, 0x00, 0x00, 0x00, 0x00, 0xc8, 0x7f,
        0xaf, 0x70, 0xf1, 0x00, 0x00, 0x7f, 0x00, 0x7f,
        0xf0, 0xfb, 0xf1, 0xf2, 0xf3, 0xf4, 0xf0, 0xfe,
        0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    };

    userData->hasMemory = true;
    memcpy(userData->regs, tempData, 0x30);

    (void)uiFuncs;
    (void)serviceFunc;

    return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
    free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawText(PDUI* uiFuncs, const char* startText, uint8_t value, uint32_t bitStart, uint32_t mask, const char* endText)
{
    uint32_t v = (value >> bitStart) & mask;
    uiFuncs->text(startText);
    uiFuncs->sameLine(0, -1);
    uiFuncs->textColored(s_colorRed, "%02d ($%02x)", v, v);
    uiFuncs->sameLine(0, -1);

    if (endText)
        uiFuncs->text(endText);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawSpritBits(PDUI* uiFuncs, const char* registerText, uint8_t reg, const char* descText)
{
    uiFuncs->text(registerText); uiFuncs->nextColumn();

    for (int i = 0; i < 8; ++i)
    {
        char t0[16];
        char t1[256];

        sprintf(t0, "Bit #%d:", i);
        sprintf(t1, descText, i);

        drawText(uiFuncs, t0, reg, 0, 0x1, t1);
    }

    uiFuncs->nextColumn();
    uiFuncs->separator();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawd018(PDUI* uiFuncs, uint8_t value, uint8_t bitmapMode)
{
    uiFuncs->text("$d018 - Memory setup"); uiFuncs->nextColumn();

    if (!bitmapMode)
    {
        uint32_t v = (value >> 1) & 0x7;
        uint32_t address = (v * 0x800);

        uiFuncs->text("Bit #1-#3:");
        uiFuncs->sameLine(0, -1);
        uiFuncs->textColored(s_colorRed, "%02d ($%02x) - $%04x-$%04x", v, v, address, address + 0x7ff);
        uiFuncs->sameLine(0, -1);
        uiFuncs->text("(Pointer to character memory, relative to VIC bank)");

        v = (value >> 4) & 0xf;
        address = (v * 0x400);

        uiFuncs->text("Bit #4-#7:");
        uiFuncs->sameLine(0, -1);
        uiFuncs->textColored(s_colorRed, "%02d ($%02x) - $%04x-$%04x", v, v, address, address + 0x3ff);
        uiFuncs->sameLine(0, -1);
        uiFuncs->text("(Pointer to screen memory, relative to VIC bank)");
    }
    else
    {
        uint32_t v = (value >> 3) & 0x1;
        uint32_t address = (v * 0x2000);

        uiFuncs->text("Bit #3:");
        uiFuncs->sameLine(0, -1);
        uiFuncs->textColored(s_colorRed, "%02d ($%02x) - $%04x-$%04x", v, v, address, address + 0x1fff);
        uiFuncs->sameLine(0, -1);
        uiFuncs->text("(Pointer to bitmap memory, relative to VIC bank)");
    }

    uiFuncs->nextColumn();
    uiFuncs->separator();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define color_rgb(r, g, b) (((uint32_t)r) << 24) | (((uint32_t)g) << 16) | (((uint32_t)b) << 8) | 255

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t s_colors[] =
{
    color_rgb(0, 0,  0),  // Black:
    color_rgb(255, 255, 255),  // White:
    color_rgb(224, 64, 64), // Red:
    color_rgb(96, 255, 255), // Cyan:
    color_rgb(224, 96, 224), // Magenta:
    color_rgb(64, 224, 64), // Green:
    color_rgb(64, 64, 224), // Blue:
    color_rgb(255, 255, 64), // Yellow:
    color_rgb(224, 160, 64), // Orange:
    color_rgb(156, 116, 72), // Brown:
    color_rgb(255, 160, 160), // Pink:
    color_rgb(84, 84, 84), // DGrey:
    color_rgb(136, 136, 136), // Grey:
    color_rgb(160, 255, 160), // LGreen:
    color_rgb(160, 160, 255), // LBlue:
    color_rgb(192, 192, 192) // LGrey:
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawWithColor(PDUI* uiFuncs, const char* regName, uint8_t value)
{
    PDRect rect;
    uint32_t v = value & 0xf;

    uiFuncs->text(regName); uiFuncs->nextColumn();

    uiFuncs->text("Bit #0-#3:");
    uiFuncs->sameLine(0, -1);
    uiFuncs->textColored(s_colorRed, "%04d ($%02x)", v, v);
    uiFuncs->sameLine(0, -1);

    PDVec2 pos = uiFuncs->getCursorPos();

    // actual color

    rect.x = pos.x;
    rect.y = pos.y;
    rect.width = 30;
    rect.height = 14;
    uiFuncs->fillRect(rect, s_colors[v]);

    uiFuncs->nextColumn();
    uiFuncs->separator();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void showUI(CustomRegsData* data, PDUI* uiFuncs)
{
    uiFuncs->text("");
    uiFuncs->columns(2, "registers", true);
    uiFuncs->text("Name"); uiFuncs->nextColumn();
    uiFuncs->text("Value"); uiFuncs->nextColumn();

    uint8_t* regs = (uint8_t*)&data->regs;

    for (uint32_t i = 0; i < 8; ++i)
    {
        printSpriteX(uiFuncs, regs[(i * 2) + i], i, regs[0x10]);
        printSpriteY(uiFuncs, regs[(i * 2) + i], i);
        uiFuncs->separator();
    }

    // d010

    uiFuncs->text("$d010 - Sprite #0-#7 X-coordinates (bit #8)");
    uiFuncs->nextColumn();
    uiFuncs->textColored(s_colorRed, "%04d ($%02x)", regs[0x10], regs[0x10]);
    uiFuncs->sameLine(0, -1);
    uiFuncs->nextColumn();
    uiFuncs->separator();

    // d011

    uiFuncs->text("$d011 - Screen control #1"); uiFuncs->nextColumn();
    drawText(uiFuncs, "Bits #0-#2:", regs[0x11], 0, 0x7, "(Vertical raster scroll)");
    drawText(uiFuncs, "Bit     #3:", regs[0x11], 3, 0x1, "(Screen height; 0 = 24 rows; 1 = 25 rows)");
    drawText(uiFuncs, "Bit     #4:", regs[0x11], 4, 0x1, "(0 = Screen off; 1 = Screen on)");
    drawText(uiFuncs, "Bit     #5:", regs[0x11], 5, 0x1, "(0 = Text mode; 1 = Bitmap mode)");
    drawText(uiFuncs, "Bit     #6:", regs[0x11], 6, 0x1, "(1 = Extended background mode on)");
    drawText(uiFuncs, "Bit     #7:", regs[0x11], 7, 0x1, "(Read: Current raster line (bit #8)");
    uiFuncs->nextColumn();

    // d012

    uiFuncs->text("$d012 - Screen control #1"); uiFuncs->nextColumn();
    drawText(uiFuncs, "Bits #0-#7:", regs[0x12], 0, 0xff, "(Current raster line)");
    uiFuncs->nextColumn();
    uiFuncs->separator();

    // d013

    uiFuncs->text("$d013 - Light pen X-coordinate"); uiFuncs->nextColumn();
    drawText(uiFuncs, "Bits #1-#8:", regs[0x13], 0, 0xff, 0);
    uiFuncs->nextColumn();
    uiFuncs->separator();

    // d014

    uiFuncs->text("$d014 - Light pen Y-coordinate"); uiFuncs->nextColumn();
    drawText(uiFuncs, "Bits #0-#7:", regs[0x14], 0, 0xff, 0);
    uiFuncs->nextColumn();
    uiFuncs->separator();

    // d015

    drawSpritBits(uiFuncs, "$d015 - Sprite enable", regs[0x15], "(Sprite %d Enabled)");

    // d016

    uiFuncs->text("$d016 - Screen control #2"); uiFuncs->nextColumn();
    drawText(uiFuncs, "Bits #0-#2:", regs[0x16], 0, 0x7, "(Horizontal raster scroll)");
    drawText(uiFuncs, "Bit     #3:", regs[0x16], 3, 0x1, "Screen width; 0 = 38 columns; 1 = 40 columns");
    drawText(uiFuncs, "Bit     #4:", regs[0x16], 4, 0x1, "1 = Multicolor mode on");
    uiFuncs->nextColumn();
    uiFuncs->separator();

    // d017

    drawSpritBits(uiFuncs, "$d017 - Sprite double height", regs[0x17], "(Sprite %d Enabled)");

    // d018

    drawd018(uiFuncs, regs[0x18], (regs[0x11] >> 5) & 0x1);

    // d019

    uiFuncs->text("$d019 - Interrupt status"); uiFuncs->nextColumn();
    drawText(uiFuncs, "Bit #0:", regs[0x19], 0, 0x1, "(Current raster line is equal to the raster line to generate interrupt at)");
    drawText(uiFuncs, "Bit #1:", regs[0x19], 1, 0x1, "(1 = Sprite-background collision occurred)");
    drawText(uiFuncs, "Bit #2:", regs[0x19], 2, 0x1, "(1 = Sprite-sprite collision occurred)");
    drawText(uiFuncs, "Bit #3:", regs[0x19], 3, 0x1, "(1 = Light pen signal arrived)");
    drawText(uiFuncs, "Bit #7:", regs[0x19], 7, 0x1, "(An event, that may generate an interrupt, occurred)");
    uiFuncs->nextColumn();
    uiFuncs->separator();

    // d01a

    uiFuncs->text("$d01a - Interrupt control"); uiFuncs->nextColumn();
    drawText(uiFuncs, "Bit #0:", regs[0x1a], 0, 0x1, "(Raster interrupt enabled)");
    drawText(uiFuncs, "Bit #1:", regs[0x1a], 1, 0x1, "(Sprite-background collision interrupt enabled)");
    drawText(uiFuncs, "Bit #2:", regs[0x1a], 2, 0x1, "(Sprite-sprite collision interrupt enabled)");
    drawText(uiFuncs, "Bit #3:", regs[0x1a], 3, 0x1, "(Light pen interrupt enabled)");
    uiFuncs->nextColumn();
    uiFuncs->separator();

    // d01b

    drawSpritBits(uiFuncs, "$d01b - Sprite priority", regs[0x1b], "(0 = Sprite #%d is drawn in front of screen contents; 1 = behind)");

    // d01c

    drawSpritBits(uiFuncs, "$d01b - Sprite multicolor mode", regs[0x1c], "(0 = Sprite #%d is single color; 1 = multicolor)");

    // d01d

    drawSpritBits(uiFuncs, "$d01d - Sprite double width", regs[0x1d], "(1 = Sprite #%d is stretched to double width)");

    // d01e

    drawSpritBits(uiFuncs, "$d01e - Sprite-sprite collusion", regs[0x1e], "(1 = Sprite #%d collided with another sprite)");

    // d01f

    drawSpritBits(uiFuncs, "$d01f - Sprite-background collision", regs[0x1f], "(1 = Sprite #%d collided with background)");

    // d020

    drawWithColor(uiFuncs, "$d020 - Border color", regs[0x20]);
    drawWithColor(uiFuncs, "$d021 - Background color", regs[0x21]);
    drawWithColor(uiFuncs, "$d022 - Extra background color #1", regs[0x22]);
    drawWithColor(uiFuncs, "$d023 - Extra background color #2", regs[0x23]);
    drawWithColor(uiFuncs, "$d024 - Extra background color #3", regs[0x24]);
    drawWithColor(uiFuncs, "$d025 - Sprite extra color #1", regs[0x25]);
    drawWithColor(uiFuncs, "$d026 - Sprite extra color #1", regs[0x26]);
    drawWithColor(uiFuncs, "$d027 - Sprite #0 color", regs[0x27]);
    drawWithColor(uiFuncs, "$d028 - Sprite #1 color", regs[0x28]);
    drawWithColor(uiFuncs, "$d029 - Sprite #2 color", regs[0x29]);
    drawWithColor(uiFuncs, "$d02a - Sprite #3 color", regs[0x2a]);
    drawWithColor(uiFuncs, "$d02b - Sprite #4 color", regs[0x2b]);
    drawWithColor(uiFuncs, "$d02c - Sprite #5 color", regs[0x2c]);
    drawWithColor(uiFuncs, "$d02d - Sprite #6 color", regs[0x2d]);
    drawWithColor(uiFuncs, "$d02e - Sprite #7 color", regs[0x2e]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateMemory(CustomRegsData* userData, PDReader* reader)
{
    void* data;
    uint64_t address = 0;
    uint64_t size = 0;

    PDRead_findU64(reader, &address, "address", 0);

    if (PDRead_findData(reader, &data, &size, "data", 0) == PDReadStatus_notFound)
        return;

    if (address != 0xd000 || size != 0x30)
        return;

    memcpy(userData->regs, data, 0x30);
    userData->hasMemory = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* reader, PDWriter* writer)
{
    uint32_t event;
    CustomRegsData* data = (CustomRegsData*)userData;

    data->requestMemory = false;

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_setExceptionLocation:
            {
                uint64_t location = 0;

                PDRead_findU64(reader, &location, "address", 0);

                if (location != data->location)
                {
                    data->location = location;
                    data->requestMemory = true;
                }

                break;
            }

            case PDEventType_setMemory:
            {
                updateMemory(data, reader);
                data->hasMemory = true;
                break;
            }
        }
    }

    showUI(data, uiFuncs);

    if (data->requestMemory)
    {
        PDWrite_eventBegin(writer, PDEventType_getMemory);
        PDWrite_u64(writer, "address_start", 0xd000);
        PDWrite_u64(writer, "size", 0x30);
        PDWrite_eventEnd(writer);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDViewPlugin g_c64CustomViewPlugin =
{
    "C64 VICE Custom Registers",
    createInstance,
    destroyInstance,
    update,
};
