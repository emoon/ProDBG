![](/data/prodbg_logo.png)
======

ProDBG is a new debugger under development that will support a variety of targets and operating systems. Currently it's in very early development and primary focusing on Mac as primary target.

## Build status

[![Travis Status](https://travis-ci.org/emoon/ProDBG.svg?branch=master)](https://travis-ci.org/emoon/ProDBG)
[![Appveyor status](https://ci.appveyor.com/api/projects/status/ne1jeu7t8aba5nok?svg=true)](https://ci.appveyor.com/project/emoon/prodbg)

### Status

As the rewrite of ProDBG (to using Qt in C++) is currently under way no debugging is working currently as everything is being brought up again. At this time C++ will be used for the UI but backends will be able to use different languages (such as Rust) as a C API is provided for this.

## Cloning the repository

The ProDBG repository contains submodules. Clone it with `git clone --recursive`. If you forgot to clone it recursively the first time, from within the cloned repository run `git submodule update --init --recursive`.

## How to compile and build

Latest stable version of **Rust** (1.36+) needs to be present on the system. We recommend using [rustup](https://www.rustup.rs/) to install and manage your Rust toolchain(s). There are also other ways to [install rustup](https://github.com/rust-lang-nursery/rustup.rs/#other-installation-methods). If you already have rustup installed but aren't on the latest stable Rust, you can simply run `rustup update`.

### Prequisites

ProDBG requires [Qt](https://www.qt.io/) as it's used for the UI. Go and install the 5.7 version and pick the 64-bit version for your system.
You also need to set three env variables: **QT5_LIB, QT5_BIN, QT5_INC** in order to build the code. See more details for each platform.
## Mac

### Prequisites

Building the code on Mac requires that **Clang** is installed on your system. The easiest way to do this is to get Xcode and install the commandline tools.

### Rustup
Run: `rustup install stable-x86_64-apple-darwin` or `rustup override add stable-x86_64-apple-darwin`

### Build
Run: `scripts/mac_build_debug.sh`

### Output
The main execeutable is located at: `t2-output/macosx-clang-debug-default/ProDBG.app/Content/MacOS/prodbg`

## Windows

### Prequisites
On Windows Visual Studio 2017 or later is required (2012 or earlier will not work as parts of the code uses C99)

## Env variables

Something similar to this

```
QT5_LIB=C:\Qt\5.12.4\msvc2017_64\lib
QT5_BIN=C:\Qt\5.12.4\msvc2017_64\bin
QT5_INC=C:\Qt\5.12.4\msvc2017_64\include
```

### Rustup
`rustup install stable-x86_64-pc-windows-msvc` or `rustup override add stable-x86_64-pc-windows-msvc`

### Build
Run: `scripts\vcvarsx86_amd64.bat` and then `scripts\win64_build_debug.cmd`

### Run
Run: `t2-output\win64-msvc-debug-default\prodbg.exe`

## Linux

### Prequisites
Building the code on Linux will require some prerequisites to be installed. Which prerequisites depends on the distribution being used.

For Ubuntu you can use the following:
```
sudo apt-get update
sudo apt-get install -y libx11-dev libgl1-mesa-dev libgtk-3-dev pkg-config qt512base
```

ProDBG uses Tundra to build the project the project. Binaries are supplied on Mac and Windows but needs to be built on Linux:
* `git clone https://github.com/deplinenoise/tundra.git`
* `cd tundra`
* `CXX=g++ make`
* `sudo make install`

### Env variables

These highly depends on how your system is setup. Here are two examples

```
export QT5_BIN=/usr/bin
export QT5_INC=/usr/include/x86_64-linux-gnu/qt5/
export QT5_LIB=/usr/lib/x86_64-linux-gnu
```

```
export QT5_BIN=/opt/qt512/bin
export QT5_INC=/opt/qt512/include
export QT5_LIB=/opt/qt512/lib
```

### Rustup
Run: `rustup install stable-x86_64-unknown-linux-gnu` or `rustup override add stable-x86_64-unknown-linux-gnu`

### Build
Run: `tundra2 linux-gcc-debug`

### Run
The main executable is located at: `t2-output/linux-gcc-debug-default/prodbg`
