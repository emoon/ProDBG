@echo off

start t2-output\Prodbg.sln

REM ======= Wait 3 seconds, hopefully the VS window has appeared by then =========
REM Without this delay, the VS window will be opened in the background
ping 127.0.0.1 -n 6 -w 1000 >NUL

