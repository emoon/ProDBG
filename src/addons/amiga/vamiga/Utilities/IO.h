// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include <dirent.h>
#include <fcntl.h>
#include <istream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace util {

//
// Handling file names
//

// Changes the capitalization of a string
string lowercased(const string& s);
string uppercased(const string& s);

// Extracts a certain component from a path
string extractPath(const string &path);
string extractName(const string &path);
string extractSuffix(const string &path);

// Strips a certain component from a path
string stripPath(const string &path);
string stripName(const string &path);
string stripSuffix(const string &path);

// Concatinates two path segments
string appendPath(const string &path, const string &path2);


//
// Handling files
//

// Returns the size of a file in bytes
isize getSizeOfFile(const string &path);
isize getSizeOfFile(const char *path);

// Checks if a file exists
bool fileExists(const string &path);

// Checks if a path points to a directory
bool isDirectory(const string &path);
bool isDirectory(const char *path);

// Returns the number of files in a directory
isize numDirectoryItems(const string &path);
isize numDirectoryItems(const char *path);

// Returns a list of files in a directory
std::vector<string> files(const string &path, const string &suffix = "");
std::vector<string> files(const string &path, std::vector <string> &suffixes);

// Checks the header signature (magic bytes) of a stream or buffer
bool matchingStreamHeader(std::istream &stream, const u8 *header, isize len);
bool matchingBufferHeader(const u8 *buffer, const u8 *header, isize len);

// Loads a file from disk
bool loadFile(const string &path, u8 **bufptr, isize *size);
bool loadFile(const string &path, const string &name, u8 **bufptr, isize *size);


//
// Handling streams
//

isize streamLength(std::istream &stream);

#define DEC std::dec
#define HEX8 std::hex << "0x" << std::setw(2) << std::setfill('0')
#define HEX16 std::hex << "0x" << std::setw(4) << std::setfill('0')
#define HEX32 std::hex << "0x" << std::setw(8) << std::setfill('0')
#define HEX64 std::hex << "0x" << std::setw(16) << std::setfill('0')
#define TAB(x) std::left << std::setw(x)
#define YESNO(x) ((x) ? "yes" : "no")
#define ONOFF(x) ((x) ? "on" : "off")
#define HILO(x) ((x) ? "high" : "low")
#define ISENABLED(x) ((x) ? "enabled" : "disabled")
#define ISSET(x) ((x) ? "set" : "not set")
#define EMULATED(x) ((x) ? "emulated" : "not emulated")
#define DUMP(x) std::setw(24) << std::right << std::setfill(' ') << (x) << " : "

}
