extern void callMe2();

typedef struct Data2 {
	int foo;
	int bar;
} Data2;

typedef struct Data {
	Data2 data;
} Data;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void callMe(int test0, float test2) {
	Data2 data;
	data.foo = 0;
	data.bar = 1;

	for (int i = 0; i < 10; ++i) {
		test2 += 1.0f;
		data.foo += 2;
		data.bar += 4;
	}

    callMe2();
}
