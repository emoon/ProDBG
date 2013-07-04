#include <stdio.h>
#include <string.h>

// Code taken from https://bitbucket.org/elemental/emumaster with some slight changes + rewritten to C 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Dis6502Str
{
	const char* nimonic;
	int type;
} Dis6502Str;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dis6502str.type = AddressType | AccessType;

enum AddressType 
{
	ADT_NONE = 0,
	ADT_IMM,				//
	ADT_AREG,				// A Register
	ADT_ZEROP,				// Zero Page
	ADT_ZEROPX,				// Zero Page,X
	ADT_ZEROPY,				// Zero Page,Y
	ADT_REL,				//
	ADT_ABS,				//
	ADT_ABSX,				//
	ADT_ABSY,				//
	ADT_ABSIND,				//
	ADT_PREIND,				// (Zero Page,X)
	ADT_POSTIND,			// (Zero Page),Y
	ADT_MASK = 0xff
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum AccessType 
{
	ACT_NL		= 0x000,
	ACT_RD		= 0x100,
	ACT_WT		= 0x200,
	ACT_WD		= 0x400,
	ACT_RW		= ACT_RD|ACT_WT,
	ACT_MASK	= ACT_RW
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* adtString[] = 
{
	"",
	"#$%02x",
	"a",
	"$%02x",
	"$%02x,x",
	"$%02x,y",
	"$%04x",
	"$%04x",
	"$%04x,x",
	"$%04x,y",
	"($%04x)",
	"($%02x,x)",
	"($%02x),y",
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int opByteLength[] = { 1,2,1,2,2,2,2,3,3,3,3,2,2 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Dis6502Str dis6502[256] = 
{
	// 00-0f
	{ "brk", ACT_NL | ADT_NONE },
	{ "ora", ACT_RD | ADT_PREIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "ora", ACT_RD | ADT_ZEROP },
	{ "asl", ACT_WT | ADT_ZEROP },
	{ "und", ACT_NL | ADT_NONE },
	{ "php", ACT_WT | ADT_NONE },
	{ "ora", ACT_NL | ADT_IMM },
	{ "asl", ACT_NL | ADT_AREG },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "ora", ACT_RD | ADT_ABS },
	{ "asl", ACT_WT | ADT_ABS },
	{ "und", ACT_NL | ADT_NONE },
	// 10-1f
	{ "bpl", ACT_NL | ADT_REL },
	{ "ora", ACT_RD | ADT_POSTIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "ora", ACT_RD | ADT_ZEROPX },
	{ "asl", ACT_WT | ADT_ZEROPX },
	{ "und", ACT_NL | ADT_NONE },
	{ "clc", ACT_NL | ADT_NONE },
	{ "ora", ACT_RD | ADT_ABSY },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "ora", ACT_RD | ADT_ABSX },
	{ "asl", ACT_WT | ADT_ABSX },
	{ "und", ACT_NL | ADT_NONE },
	// 20-2f
	{ "jsr", ACT_NL | ADT_ABS },
	{ "and", ACT_RD | ADT_PREIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "bit", ACT_RD | ADT_ZEROP },
	{ "and", ACT_RD | ADT_ZEROP },
	{ "rol", ACT_WT | ADT_ZEROP },
	{ "und", ACT_RD | ADT_NONE },
	{ "plp", ACT_RD | ADT_NONE },
	{ "and", ACT_RD | ADT_IMM },
	{ "rol", ACT_NL | ADT_AREG },
	{ "und", ACT_NL | ADT_NONE },
	{ "bit", ACT_RD | ADT_ABS },
	{ "and", ACT_RD | ADT_ABS },
	{ "rol", ACT_WT | ADT_ABS },
	{ "und", ACT_NL | ADT_NONE },
	// 30-3f
	{ "bmi", ACT_NL | ADT_REL },
	{ "and", ACT_RD | ADT_POSTIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "and", ACT_RD | ADT_ZEROPX },
	{ "rol", ACT_WT | ADT_ZEROPX },
	{ "und", ACT_NL | ADT_NONE },
	{ "sec", ACT_NL | ADT_NONE },
	{ "and", ACT_RD | ADT_ABSY },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "and", ACT_RD | ADT_ABSX },
	{ "rol", ACT_WT | ADT_ABSX },
	{ "und", ACT_NL | ADT_NONE },
	// 40-4f
	{ "rti", ACT_NL | ADT_NONE },
	{ "eor", ACT_RD | ADT_PREIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "eor", ACT_RD | ADT_ZEROP },
	{ "lsr", ACT_WT | ADT_ZEROP },
	{ "und", ACT_NL | ADT_NONE },
	{ "pha", ACT_WT | ADT_NONE },
	{ "eor", ACT_NL | ADT_IMM },
	{ "lsr", ACT_NL | ADT_AREG },
	{ "und", ACT_NL | ADT_NONE },
	{ "jmp", ACT_NL | ADT_ABS },
	{ "eor", ACT_RD | ADT_ABS },
	{ "lsr", ACT_WT | ADT_ABS },
	{ "und", ACT_NL | ADT_NONE },
	// 50-5f
	{ "bvc", ACT_NL | ADT_REL },
	{ "eor", ACT_RD | ADT_POSTIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "eor", ACT_RD | ADT_ZEROPX },
	{ "lsr", ACT_WT | ADT_ZEROPX },
	{ "und", ACT_NL | ADT_NONE },
	{ "cli", ACT_NL | ADT_NONE },
	{ "eor", ACT_RD | ADT_ABSY },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "eor", ACT_RD | ADT_ABSX },
	{ "lsr", ACT_WT | ADT_ABSX },
	{ "und", ACT_NL | ADT_NONE },
	// 60-6f
	{ "rts", ACT_NL | ADT_NONE },
	{ "adc", ACT_RD | ADT_PREIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "adc", ACT_RD | ADT_ZEROP },
	{ "ror", ACT_WT | ADT_ZEROP },
	{ "und", ACT_NL | ADT_NONE },
	{ "pla", ACT_RD | ADT_NONE },
	{ "adc", ACT_NL | ADT_IMM },
	{ "ror", ACT_NL | ADT_AREG },
	{ "und", ACT_NL | ADT_NONE },
	{ "jmp", ACT_NL | ADT_ABSIND },
	{ "adc", ACT_RD | ADT_ABS },
	{ "ror", ACT_WT | ADT_ABS },
	{ "und", ACT_NL | ADT_NONE },
	// 70-7f
	{ "bvs", ACT_NL | ADT_REL },
	{ "adc", ACT_RD | ADT_POSTIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "adc", ACT_RD | ADT_ZEROPX },
	{ "ror", ACT_WT | ADT_ZEROPX },
	{ "und", ACT_NL | ADT_NONE },
	{ "sei", ACT_NL | ADT_NONE },
	{ "adc", ACT_RD | ADT_ABSY },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "adc", ACT_RD | ADT_ABSX },
	{ "ror", ACT_WT | ADT_ABSX },
	{ "und", ACT_NL | ADT_NONE },
	// 80-8f
	{ "und", ACT_NL | ADT_NONE },
	{ "sta", ACT_WT | ADT_PREIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "sty", ACT_WT | ADT_ZEROP },
	{ "sta", ACT_WT | ADT_ZEROP },
	{ "stx", ACT_WT | ADT_ZEROP },
	{ "und", ACT_NL | ADT_NONE },
	{ "dey", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "txa", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "sty", ACT_WT | ADT_ABS },
	{ "sta", ACT_WT | ADT_ABS },
	{ "stx", ACT_WT | ADT_ABS },
	{ "und", ACT_NL | ADT_NONE },
	// 90-9f
	{ "bcc", ACT_NL | ADT_REL },
	{ "sta", ACT_WT | ADT_POSTIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "sty", ACT_WT | ADT_ZEROPX },
	{ "sta", ACT_WT | ADT_ZEROPX },
	{ "stx", ACT_WT | ADT_ZEROPY },
	{ "und", ACT_NL | ADT_NONE },
	{ "tya", ACT_NL | ADT_NONE },
	{ "sta", ACT_WT | ADT_ABSY },
	{ "txs", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "sta", ACT_WT | ADT_ABSX },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	// a0-af
	{ "ldy", ACT_NL | ADT_IMM },
	{ "lda", ACT_RD | ADT_PREIND },
	{ "ldx", ACT_NL | ADT_IMM },
	{ "und", ACT_NL | ADT_NONE },
	{ "ldy", ACT_RD | ADT_ZEROP },
	{ "lda", ACT_RD | ADT_ZEROP },
	{ "ldx", ACT_RD | ADT_ZEROP },
	{ "und", ACT_NL | ADT_NONE },
	{ "tay", ACT_NL | ADT_NONE },
	{ "lda", ACT_NL | ADT_IMM },
	{ "tax", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "ldy", ACT_RD | ADT_ABS },
	{ "lda", ACT_RD | ADT_ABS },
	{ "ldx", ACT_RD | ADT_ABS },
	{ "und", ACT_NL | ADT_NONE },
	// b0-bf
	{ "bcs", ACT_NL | ADT_REL },
	{ "lda", ACT_RD | ADT_POSTIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "ldy", ACT_RD | ADT_ZEROPX },
	{ "lda", ACT_RD | ADT_ZEROPX },
	{ "ldx", ACT_RD | ADT_ZEROPY },
	{ "und", ACT_NL | ADT_NONE },
	{ "clv", ACT_NL | ADT_NONE },
	{ "lda", ACT_RD | ADT_ABSY },
	{ "tsx", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "ldy", ACT_RD | ADT_ABSX },
	{ "lda", ACT_RD | ADT_ABSX },
	{ "ldx", ACT_RD | ADT_ABSY },
	{ "und", ACT_NL | ADT_NONE },
	// c0-cf
	{ "cpy", ACT_NL | ADT_IMM },
	{ "cmp", ACT_RD | ADT_PREIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "cpy", ACT_RD | ADT_ZEROP },
	{ "cmp", ACT_RD | ADT_ZEROP },
	{ "dec", ACT_WT | ADT_ZEROP },
	{ "und", ACT_NL | ADT_NONE },
	{ "iny", ACT_NL | ADT_NONE },
	{ "cmp", ACT_NL | ADT_IMM },
	{ "dex", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "cpy", ACT_RD | ADT_ABS },
	{ "cmp", ACT_RD | ADT_ABS },
	{ "dec", ACT_WT | ADT_ABS },
	{ "und", ACT_NL | ADT_NONE },
	// d0-df
	{ "bne", ACT_NL | ADT_REL },
	{ "cmp", ACT_RD | ADT_POSTIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "cmp", ACT_RD | ADT_ZEROPX },
	{ "dec", ACT_WT | ADT_ZEROPX },
	{ "und", ACT_NL | ADT_NONE },
	{ "cld", ACT_NL | ADT_NONE },
	{ "cmp", ACT_RD | ADT_ABSY },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "cmp", ACT_RD | ADT_ABSX },
	{ "dec", ACT_WT | ADT_ABSX },
	{ "und", ACT_NL | ADT_NONE },
	// e0-ef
	{ "cpx", ACT_NL | ADT_IMM },
	{ "sbc", ACT_RD | ADT_PREIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "cpx", ACT_RD | ADT_ZEROP },
	{ "sbc", ACT_RD | ADT_ZEROP },
	{ "inc", ACT_WT | ADT_ZEROP },
	{ "und", ACT_NL | ADT_NONE },
	{ "inx", ACT_NL | ADT_NONE },
	{ "sbc", ACT_NL | ADT_IMM },
	{ "nop", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "cpx", ACT_RD | ADT_ABS },
	{ "sbc", ACT_RD | ADT_ABS },
	{ "inc", ACT_WT | ADT_ABS },
	{ "und", ACT_NL | ADT_NONE },
	// f0-ff
	{ "beq", ACT_NL | ADT_REL },
	{ "sbc", ACT_RD | ADT_POSTIND },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "sbc", ACT_RD | ADT_ZEROPX },
	{ "inc", ACT_WT | ADT_ZEROPX },
	{ "und", ACT_NL | ADT_NONE },
	{ "sed", ACT_NL | ADT_NONE },
	{ "sbc", ACT_RD | ADT_ABSY },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "und", ACT_NL | ADT_NONE },
	{ "sbc", ACT_RD | ADT_ABSX },
	{ "inc", ACT_WT | ADT_ABSX },
	{ "und", ACT_NL | ADT_NONE }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char disassembled[0x10000][64];
extern unsigned char* s_memory6502;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned char readMem8(unsigned short addr)
{
	return s_memory6502[addr];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned short readMem16(unsigned short addr)
{
	unsigned char lo = readMem8(addr + 0);
	unsigned char hi = readMem8(addr + 1);
	return lo | (hi<<8);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int disassemblyOne(unsigned short addr)
{
	unsigned char op = readMem8(addr);
	int adt = dis6502[op].type & ADT_MASK;

	char* dst = disassembled[addr];

	//instructions[addr].op = op;
	//instructions[addr].disassembled = dst;

	int pos = 0;
	int size;
	if (addr < 0xfffa) 
	{
		size = opByteLength[adt];
		switch (size) {
		case 1:
			if (!strcmp(dis6502[op].nimonic, "und"))
				adt = ADT_IMM;
			pos += sprintf(dst + pos, "%02x      ", op);
			pos += sprintf(dst + pos, "    %s ", dis6502[op].nimonic);
			pos += sprintf(dst + pos, adtString[adt], op);
			break;
		case 2:
			pos += sprintf(dst + pos, "%02x %02x   ", op, readMem8(addr+1));
			pos += sprintf(dst + pos, "    %s ", dis6502[op].nimonic);
			if (adt == ADT_REL) {
				unsigned char disp = readMem8(addr+1);
				if (disp < 0x80) {
					pos += sprintf(dst + pos, adtString[adt], addr+2+disp);
				} else {
					pos += sprintf(dst + pos, adtString[adt], addr+2-(0x100-disp));
				}
			} else {
				pos += sprintf(dst + pos, adtString[adt], readMem8(addr+1));
			}
			break;
		case 3:
			pos += sprintf(dst + pos, "%02x %02x %02x", op, readMem8(addr+1), readMem8(addr+2));
			pos += sprintf(dst + pos, "    %s ", dis6502[op].nimonic);
			pos += sprintf(dst + pos, adtString[adt], readMem16(addr+1));
			break;
		default:
			break;
		}
		pos += sprintf(dst + pos,"\n");
	} else {
		pos += sprintf(dst + pos," %02x      db #$%02x     \n", op, op);
		size = 1;
	}

	printf("%s", disassembled[addr]);
	
	return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void disassemble(unsigned short begin, unsigned short end)
{
	unsigned int i;

	for (i = begin; i <= end; )
		i += disassemblyOne(i);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

