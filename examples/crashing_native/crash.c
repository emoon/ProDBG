#include <stdio.h>

extern void callMe();

#ifdef __clang__

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* dummyThreadFunc(void* dummy)
{
	pthread_setname_np("dummy_thread");
	(void)dummy;

	if (((uintptr_t)dummy) == 1)
		return 0;
 
 	while (1)
	{
		usleep(1000);
	}

	//return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void createDummyThread()
{
	pthread_t thread = 0;

	pthread_create(&thread, 0, dummyThreadFunc, 0);
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    int i;

#ifdef __clang__
	createDummyThread();
#endif

    for (i = 0; i < 20; ++i)
        printf("%d\n", i);

    callMe();

    for (i = 0; i < 20; ++i)
        printf("%d\n", i);

    return 0;
}
