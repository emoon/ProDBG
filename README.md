ProDBG
======

Debugging the way it's meant to be done

Mac build status <img src="http://zenic.org:8111/app/rest/builds/buildType:(id:ProDBG_Mac)/statusIcon"/></a>

Windows Win64 build status <img src="http://zenic.org:8111/app/rest/builds/buildType:(id:ProDBG_Win64)/statusIcon"/></a>

### Status

Major redesign of the ProDBG is currently happening so be aware that not much is currently working.

## How to compile and run on Mac

Building the code on Mac requires that Clang is installed on your system. The easiest way to do this is to get Xcode and install the commandline tools.

After doing that open a terminal and run scripts/mac_build_debug.sh and you should have an executable that you can open in t2-output/macosx-clang-debug-default/ProDBG.app

## How to compile and run on Windows

On Windows Visual Studio 2013 is required (2012 or earlier will not work as parts of the code uses C99) With VS 2013 installed do this:

Open a cmd window and first run scripts\vcvarsx86_amd64.bat and then scripts\win64_build_debug.cmd and you will have an executable in t2-output\win64-msvc-debug-default\prodbg.exe

## Misc

The current misc work is being tracked on a trello borad which you can find here https://trello.com/b/blg2yGPv/prodbg


