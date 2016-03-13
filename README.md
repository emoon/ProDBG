ProDBG
======

ProDBG is a new debugger under development that will support a variety of targets and operating systems. Currently it's in very early development and primary focusing on Mac as primary target.
Right now lots of the code are being restructured and rewritten in [Rust](https://www.rust-lang.org). There is a blog post about the move to Rust over [here](http://prodbg.com/ProDBG-switches-to-Rust)

I did a presentation on 2014-11 about the project for the awesome rendering team at Frostbite/EA and it can be viewed [here](https://dl.dropboxusercontent.com/u/5205843/ProDBG-Presentation.pdf) (notice that some of the information is a bit out-dated by now) 

## Build status

[![Travis Status](https://travis-ci.org/emoon/ProDBG.svg?branch=master)](https://travis-ci.org/emoon/ProDBG)
[![Appveyor status](https://ci.appveyor.com/api/projects/status/ne1jeu7t8aba5nok?svg=true)](https://ci.appveyor.com/project/emoon/prodbg)

### Status

As the rewrite of ProDBG is currently under way no debugging is working currently and just some basic UI is up and running.

## How to compile and build

Latest stable version of Rust needs to be present on the system and can be downloaded from [here](https://www.rust-lang.org/downloads.html)

## Mac

Building the code on Mac requires that Clang is installed on your system. The easiest way to do this is to get Xcode and install the commandline tools.

After doing that open a terminal and run ```scripts/mac_build_debug.sh``` and you should have an executable that you can open in ```t2-output/macosx-clang-debug-default/ProDBG.app/Content/MacOS/prodbg```

## Windows

On Windows Visual Studio 2013 is required (2012 or earlier will not work as parts of the code uses C99) With VS 2013 installed do this:

Open a cmd window and first run ```scripts\vcvarsx86_amd64.bat``` and then ```scripts\win64_build_debug.cmd``` and you will have an executable in ```t2-output\win64-msvc-debug-default\prodbg.exe```

## Linux

Currently the Linux build isn't supported but the code complies and all unit-tests should be working correctly. To build use scripts/unix_gcc_build_debug.sh libx11-dev libgl1-mesa-dev is required to be installed in order for ProDBG to compile and link.


