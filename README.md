ProDBG
======

Debugging the way it's meant to be done

Mac build status <img src="http://zenic.org:8111/app/rest/builds/buildType:(id:ProDBG_Mac)/statusIcon"/></a>

Windows Win64 build status <img src="http://zenic.org:8111/app/rest/builds/buildType:(id:ProDBG_Win64)/statusIcon"/></a>

### Status

The status of the project currently is that lots of things are moving around so it's very likely everything is very unstable and/or won't work.

### Build Instructions

Qt5.2.1 is required and a QT5 enviroment variable needs to look something like this

Mac: QT5=path/to/qt5/5.2.1/clang_64

Win64: QT5=path\tq\Qt5\5.2.1\msvc2012_64 (Only 64-bit is support on Windows)

### Mac

cd testbed
bin/macosx/tundra/tundra2 macosx-clang-debug

### Win64

cd testbed
bin\win32\tundra2 win64-msvc-debug
