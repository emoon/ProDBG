#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
$D012
53266
Read: Current raster line (bits #0-#7).
Write: Raster line to generate interrupt at (bits #0-#7).

$D013
53267
Light pen X-coordinate (bits #1-#8).
Read-only.

$D014
53268
Light pen Y-coordinate.
Read-only.

$D015
53269
Sprite enable register. Bits:

Bit #x: 1 = Sprite #x is enabled, drawn onto the screen.

$D016
53270
Screen control register #2. Bits:

Bits #0-#2: Horizontal raster scroll.

Bit #3: Screen width; 0 = 38 columns; 1 = 40 columns.

Bit #4: 1 = Multicolor mode on.

Default: $C8, %11001000.

$D017
53271
Sprite double height register. Bits:

Bit #x: 1 = Sprite #x is stretched to double height.

$D018
53272
Memory setup register. Bits:

Bits #1-#3: In text mode, pointer to character memory (bits #11-#13), relative to VIC bank, memory address $DD00. Values:

%000, 0: $0000-$07FF, 0-2047.

%001, 1: $0800-$0FFF, 2048-4095.

%010, 2: $1000-$17FF, 4096-6143.

%011, 3: $1800-$1FFF, 6144-8191.

%100, 4: $2000-$27FF, 8192-10239.

%101, 5: $2800-$2FFF, 10240-12287.

%110, 6: $3000-$37FF, 12288-14335.

%111, 7: $3800-$3FFF, 14336-16383.

Values %010 and %011 in VIC bank #0 and #2 select Character ROM instead.
In bitmap mode, pointer to bitmap memory (bit #13), relative to VIC bank, memory address $DD00. Values:

%0xx, 0: $0000-$1FFF, 0-8191.

%1xx, 4: $2000-$3FFF, 8192-16383.

Bits #4-#7: Pointer to screen memory (bits #10-#13), relative to VIC bank, memory address $DD00. Values:

%0000, 0: $0000-$03FF, 0-1023.

%0001, 1: $0400-$07FF, 1024-2047.

%0010, 2: $0800-$0BFF, 2048-3071.

%0011, 3: $0C00-$0FFF, 3072-4095.

%0100, 4: $1000-$13FF, 4096-5119.

%0101, 5: $1400-$17FF, 5120-6143.

%0110, 6: $1800-$1BFF, 6144-7167.

%0111, 7: $1C00-$1FFF, 7168-8191.

%1000, 8: $2000-$23FF, 8192-9215.

%1001, 9: $2400-$27FF, 9216-10239.

%1010, 10: $2800-$2BFF, 10240-11263.

%1011, 11: $2C00-$2FFF, 11264-12287.

%1100, 12: $3000-$33FF, 12288-13311.

%1101, 13: $3400-$37FF, 13312-14335.

%1110, 14: $3800-$3BFF, 14336-15359.

%1111, 15: $3C00-$3FFF, 15360-16383.

$D019
53273
Interrupt status register. Read bits:

Bit #0: 1 = Current raster line is equal to the raster line to generate interrupt at.

Bit #1: 1 = Sprite-background collision occurred.

Bit #2: 1 = Sprite-sprite collision occurred.

Bit #3: 1 = Light pen signal arrived.

Bit #7: 1 = An event, that may generate an interrupt, occurred.

Write bits:

Bit #0: 0 = Acknowledge raster interrupt.

Bit #1: 0 = Acknowledge sprite-background collision interrupt.

Bit #2: 0 = Acknowledge sprite-sprite collision interrupt.

Bit #3: 0 = Acknowledge light pen interrupt.

$D01A
53274
Interrupt control register. Bits:

Bit #0: 1 = Raster interrupt enabled.

Bit #1: 1 = Sprite-background collision interrupt enabled.

Bit #2: 1 = Sprite-sprite collision interrupt enabled.

Bit #3: 1 = Light pen interrupt enabled.

$D01B
53275
Sprite priority register. Bits:

Bit #x: 0 = Sprite #x is drawn in front of screen contents; 1 = Sprite #x is behind screen contents.

$D01C
53276
Sprite multicolor mode register. Bits:

Bit #x: 0 = Sprite #x is single color; 1 = Sprite #x is multicolor.

$D01D
53277
Sprite double width register. Bits:

Bit #x: 1 = Sprite #x is stretched to double width.

$D01E
53278
Sprite-sprite collision register. Read bits:

Bit #x: 1 = Sprite #x collided with another sprite.

Write: Enable further detection of sprite-sprite collisions.

$D01F
53279
Sprite-background collision register. Read bits:

Bit #x: 1 = Sprite #x collided with background.

Write: Enable further detection of sprite-background collisions.

$D020
53280
Border color (only bits #0-#3).

$D021
53281
Background color (only bits #0-#3).

$D022
53282
Extra background color #1 (only bits #0-#3).

$D023
53283
Extra background color #2 (only bits #0-#3).

$D024
53284
Extra background color #3 (only bits #0-#3).

$D025
53285
Sprite extra color #1 (only bits #0-#3).

$D026
53286
Sprite extra color #1 (only bits #0-#3).

$D027
53287
Sprite #0 color (only bits #0-#3).

$D028
53288
Sprite #1 color (only bits #0-#3).

$D029
53289
Sprite #2 color (only bits #0-#3).

$D02A
53290
Sprite #3 color (only bits #0-#3).

$D02B
53291
Sprite #4 color (only bits #0-#3).

$D02C
53292
Sprite #5 color (only bits #0-#3).

$D02D
53293
Sprite #6 color (only bits #0-#3).

$D02E
53294
Sprite #7 color (only bits #0-#3).

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct CustomRegsData
{
	uint8_t regs[0x30];
	bool hasMemory;
} CustomRegsData;

static PDVec4 s_colorRed = { 1.0, 0.0f, 0.0f, 1.0f };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printSpriteX(PDUI* uiFuncs, uint16_t v, uint32_t i, uint8_t d010)
{
	uiFuncs->text("$d0%02x - Sprite #%d X-coordinate", i * 2, i); 
	uiFuncs->nextColumn();

	uint16_t v2 = v + (((d010 >> i) & 1) << 8);

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

static void showUI(CustomRegsData* data, PDUI* uiFuncs)
{
    uiFuncs->text("");
    uiFuncs->columns(2, "registers", true);
    uiFuncs->text("Name"); uiFuncs->nextColumn();
    uiFuncs->text("Value"); uiFuncs->nextColumn();

	uint8_t* regs = (uint8_t*)&data->regs;

	for (int i = 0; i < 8; ++i)
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

	uiFuncs->text("$d011 - Screen control register #1"); uiFuncs->nextColumn();
	drawText(uiFuncs, "Bits #0-#2:", regs[0x11], 0, 0x7, "(Vertical raster scroll)");
	drawText(uiFuncs, "Bit     #3:", regs[0x11], 3, 0x1, "(Screen height; 0 = 24 rows; 1 = 25 rows)");
	drawText(uiFuncs, "Bit     #4:", regs[0x11], 4, 0x1, "(0 = Screen off; 1 = Screen on)");
	drawText(uiFuncs, "Bit     #5:", regs[0x11], 5, 0x1, "(0 = Text mode; 1 = Bitmap mode)");
	drawText(uiFuncs, "Bit     #6:", regs[0x11], 6, 0x1, "(1 = Extended background mode on)");
	drawText(uiFuncs, "Bit     #7:", regs[0x11], 7, 0x1, "(Read: Current raster line (bit #8)");
	uiFuncs->nextColumn();

	// d012

	/*
$D012
53266
Read: Current raster line (bits #0-#7).
Write: Raster line to generate interrupt at (bits #0-#7).

$D013
53267
Light pen X-coordinate (bits #1-#8).
Read-only.

$D014
53268
Light pen Y-coordinate.
Read-only.

$D015
53269
Sprite enable register. Bits:

Bit #x: 1 = Sprite #x is enabled, drawn onto the screen.

$D016
53270
Screen control register #2. Bits:

Bits #0-#2: Horizontal raster scroll.

Bit #3: Screen width; 0 = 38 columns; 1 = 40 columns.

Bit #4: 1 = Multicolor mode on.
Default: $C8, %11001000.
*/



	/*

	uiFuncs->text("Bits #0-#2: "
	uiFuncs->sameLine(0, -1);
	uiFuncs->textColored(s_colorRed, "%02d - ($%02x)",  
		(Vertical raster scroll)
	Bit     #3: %d (Screen height; 0 = 24 rows; 1 = 25 rows)
	Bit     #4: %d (0 = Screen off; 1 = Screen on)
	Bit     #5: %d (0 = Text mode; 1 = Bitmap mode)
	Bit     #6: %d (1 = Extended background mode on)
	Bit     #7: %d (Read: Current raster line (bit #8)
	*/


	/*
	uiFuncs->text("$d000 - Sprite #0 X-coordinate");
	uiFuncs->nextColumn();
	uiFuncs->text("0-7 bits ($dd), including $d010\n, some more stuff\neven more!"); 
	uiFuncs->nextColumn();
	uiFuncs->separator();
	uiFuncs->text("$d001 - Sprite #1 X-coordinate");
	uiFuncs->nextColumn();
	uiFuncs->text("$dd - 222 (0-7 bits), $1dd (333) including $d010\n, some more stuff\neven more!");
	uiFuncs->nextColumn();
	*/
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

	if (address != 0xdd00 || size != 0x30)
		return;

	memcpy(userData->regs, data, 0x30);
	userData->hasMemory = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int update(void* userData, PDUI* uiFuncs, PDReader* reader, PDWriter* writer)
{
    uint32_t event;
    CustomRegsData* data = (CustomRegsData*)userData;

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_setMemory:
            {
                updateMemory(data, reader);
				data->hasMemory = true;
                break;
            }
        }
    }

    showUI(data, uiFuncs);

	PDWrite_eventBegin(writer, PDEventType_getMemory);
	PDWrite_u64(writer, "address_start", 0xdd00);
	PDWrite_u64(writer, "size", 0x30);
	PDWrite_eventEnd(writer);

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
