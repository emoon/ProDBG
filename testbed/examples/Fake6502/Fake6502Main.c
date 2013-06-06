#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

static uint8_t* s_memory6502; //[65536];

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

static void bar(int mehBa)
{
	int i;
	int me = 0;
	int baha = 2;
	float lala_la = 4.0f;
	
	(void)me;
	(void)baha;
	(void)lala_la;
	(void)mehBa;

	for (i = 0; i < 10; ++i)
	{
		printf("%d\n", i);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void foo()
{
	bar(1337);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void tempCall()
{
	foo();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char* argv[])
{
	int t = 0;
	//FILE* f;
	//int cycleCount;

	(void)argc;
	(void)argv;
	//reset6502();
	
	tempCall();

	*((volatile int*)0) = 0x666;

#ifndef _WIN32
	usleep(100000);
#endif

	t += 0;
	t += 1;
	t += 2;
	t += 3;

	//(*(volatile int*)0) = 0x666;

#ifndef _WIN32
	while (1)
#endif
	{
		printf("Doing stuff\n");
	#ifndef _WIN32
		usleep(10000);
	#endif
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

