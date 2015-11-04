void callMe2() {
    const int foo = 2;
    const int bar = 3;

    (void)foo;
    (void)bar;

    // BAM!

    *((volatile int*)0) = 0xdead;
}
