#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

static uint8_t s_memory6502[65536];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Some exters from the 6502 emulator that we need to control it

extern void reset6502();
extern uint32_t clockticks6502;
extern uint32_t clockgoal6502;
void exec6502(uint32_t tickcount);
void step6502();

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
	//FILE* f;
	//int cycleCount;

	(void)argc;
	(void)argv;
	reset6502();

	usleep(100000);

	(*(volatile int*)0) = 0x666;

	while (1)
	{
		printf("Doing stuff\n");
		usleep(10000);
	}

	/*
	if (argc < 2)
	{
		printf("Usage: Fake6502 image.bin (max 64k in size) cyclecount\n");
	}

	if (!(f = fopen(argv[2], "rb")))
	{
		printf("Unable to open %s\n", argv[2]);
		return -1;
	}

	fread(s_memory6502, 1, 65536, f);
	fclose(f);

	cycleCount = atoi(argv[3]);

	// exec the CPU

	exec6502(cycleCount);
	*/

	//return 0;
}

