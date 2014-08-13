void callMe2()
{
	*((volatile int*)0) = 0xdead;
}
