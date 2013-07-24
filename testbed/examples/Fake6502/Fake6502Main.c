#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "Debugger6502.h"
#include <ProDBGAPI.h>

uint8_t* s_memory6502; //[65536];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Some exters from the 6502 emulator that we need to control it

extern void reset6502();
extern void execute6502();
extern struct PDBackendPlugin s_debuggerPlugin;
extern uint32_t clockticks6502;
extern uint32_t clockgoal6502;
void exec6502(uint32_t tickcount);
void step6502();
extern void disassemble(unsigned short begin, unsigned short end);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read function  for the emulated 6502 CPU

uint8_t read6502(uint16_t address)
{
	return s_memory6502[address];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Write function for the 6402

void write6502(uint16_t address, uint8_t value)
{
	s_memory6502[address] = value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char* argv[])
{
	FILE* f;
	int size;

	(void)argc;
	(void)argv;
	reset6502();

	if (argc < 2)
	{
		printf("Usage: Fake6502 image.bin (max 64k in size)\n");
		return 0;
	}

	if ((f = fopen(argv[1], "rb")) != 0)
	{
		printf("Unable to open %s\n", argv[1]);
		return -1;
	}

	// offset with 6 due to stupid compiler

	fseek(f, 0, SEEK_END);
	size = ftell(f) - 6;
	fseek(f, 6, SEEK_SET);

	s_memory6502 = malloc(65536);
	memset(s_memory6502, 0, 65536);

	fread(s_memory6502, 1, size, f);
	fclose(f);
	printf("size %d\n", (unsigned int)size);

	disassemble(0, (unsigned short)size);

	if (!PDRemote_create(&s_debuggerPlugin, 0))
	{
		printf("Unable to setup debugger connection\n");
	}

	for (;;)	
	{
		execute6502();
	}

	//return 0;
}

