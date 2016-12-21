![](https://dl.dropboxusercontent.com/u/5205843/prodbg_logo.png)
======

[![Join the chat at https://gitter.im/emoon/ProDBG](https://badges.gitter.im/emoon/ProDBG.svg)](https://gitter.im/emoon/ProDBG?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

ProDBG is a new debugger under development that will support a variety of targets and operating systems. Currently it's in very early development and primary focusing on Mac as primary target.

I did a presentation on 2014-11 about the project for the awesome rendering team at Frostbite/EA and it can be viewed [here](https://dl.dropboxusercontent.com/u/5205843/ProDBG-Presentation.pdf) (notice that some of the information is a bit out-dated by now)

## Build status

[![Travis Status](https://travis-ci.org/emoon/ProDBG.svg?branch=master)](https://travis-ci.org/emoon/ProDBG)
[![Appveyor status](https://ci.appveyor.com/api/projects/status/ne1jeu7t8aba5nok?svg=true)](https://ci.appveyor.com/project/emoon/prodbg)

### Status

As the rewrite of ProDBG (to using Qt in C++) is currently under way no debugging is working currently as everything is being brought up again. Notice that the intention is still to Rust as the main language when this has been tested out and a good API boundry can be setup.

## Cloning the repository

The ProDBG repository contains submodules. Clone it with `git clone --recursive`. If you forgot to clone it recursively the first time, from within the cloned repository run `git submodule update --init --recursive`.

## How to compile and build

Latest stable version of **Rust** (1.13+) needs to be present on the system. We recommend using [rustup](https://www.rustup.rs/) to install and manage your Rust toolchain(s). There are also other ways to [install rustup](https://github.com/rust-lang-nursery/rustup.rs/#other-installation-methods). If you already have rustup installed but aren't on the latest stable Rust, you can simply run `rustup update`.

### Prequisites

ProDBG requires [Qt](https://www.qt.io/) as it's used for the UI. Go and install the 5.7 version and pick the 64-bit version for your system.
You also need to set **QT5** env variable to point to the installation directory so the code build can find it.

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
On Windows Visual Studio 2015 or later is required (2012 or earlier will not work as parts of the code uses C99)

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
sudo apt-get install -y libx11-dev libgl1-mesa-dev libgtk-3-dev pkg-config qt57base
```

ProDBG uses Tundra to build the project the project. Binaries are supplied on Mac and Windows but needs to be built on Linux:
* `git clone https://github.com/deplinenoise/tundra.git`
* `cd tundra`
* `CXX=g++ make`
* `sudo make install`

### Rustup
Run: `rustup install stable-x86_64-unknown-linux-gnu` or `rustup override add stable-x86_64-unknown-linux-gnu`

### Build
Run: `tundra2 linux-gcc-debug`

### Run
The main executable is located at: `t2-output/linux-gcc-debug-default/prodbg`
