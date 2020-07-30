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
	Data data2;
	data2.data.foo = 0;
	data2.data.bar = 1;

	for (int i = 0; i < 10; ++i) {
		test2 += 1.0f;
		data2.data.foo += 2;
		data2.data.bar += 4;
	}

    callMe2();
}
