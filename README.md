ProDBG
======

ProDBG is a new debugger under development that will support a variety of targets and operating systems. Currently it's in very early development and primary focusing on Mac as primary target. This is how it currently looks.

![mac_screenshot](https://raw.githubusercontent.com/emoon/ProDBG/master/data/screens/mac_screenshot.png)

I did a presentation on 2014-11 about the project for the awesome rendering team at Frostbite/EA and it can be viewed here: https://dl.dropboxusercontent.com/u/5205843/ProDBG-Presentation.pdf 

## Build status

Buildbot status: http://zenic.org:8020/#/builders

[![Coverage Status](https://img.shields.io/coveralls/emoon/ProDBG.svg)](https://coveralls.io/r/emoon/ProDBG)
[![Coverity Status](https://scan.coverity.com/projects/3173/badge.svg)](https://scan.coverity.com/projects/3173)
[![Travis Status](https://travis-ci.org/emoon/ProDBG.svg?branch=master)](https://travis-ci.org/emoon/ProDBG)

### Status

Major redesign of the ProDBG is currently happening so be aware that not much is currently working.

## How to compile and run on Mac

Building the code on Mac requires that Clang is installed on your system. The easiest way to do this is to get Xcode and install the commandline tools.

After doing that open a terminal and run scripts/mac_build_debug.sh and you should have an executable that you can open in t2-output/macosx-clang-debug-default/ProDBG.app

## How to compile and run on Windows

On Windows Visual Studio 2013 is required (2012 or earlier will not work as parts of the code uses C99) With VS 2013 installed do this:

Open a cmd window and first run scripts\vcvarsx86_amd64.bat and then scripts\win64_build_debug.cmd and you will have an executable in t2-output\win64-msvc-debug-default\prodbg.exe

## How to compile and run on Linux

Currently the Linux build isn't supported but the code complies and all unit-tests should be working correctly. To build use scripts/unix_gcc_build_debug.sh libx11-dev libgl1-mesa-dev is required to be installed in order for ProDBG to compile and link.

## Misc

The tasks/issues tracking is being done here https://prodbg.phoreplay.com


