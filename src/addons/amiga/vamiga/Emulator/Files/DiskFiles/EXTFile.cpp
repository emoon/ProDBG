// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

const u8 EXTFile::extAdfHeaders[2][8] = {

    { 'U', 'A', 'E', '-', '-', 'A', 'D', 'F' },
    { 'U', 'A', 'E', '-', '1', 'A', 'D', 'F' }
};
    
EXTFile::EXTFile()
{
    setDescription("EXTFile");
}

bool
EXTFile::isEXTBuffer(const u8 *buffer, size_t length)
{
    assert(buffer != nullptr);
    
    size_t len = sizeof(extAdfHeaders[0]);
    size_t cnt = sizeof(extAdfHeaders) / len;

    if (length < len) return false;
    
    for (size_t i = 0; i < cnt; i++) {
        if (matchingBufferHeader(buffer, extAdfHeaders[i], len)) return true;
    }
    return false;
}

bool
EXTFile::isEXTFile(const char *path)
{
    assert(path != nullptr);

    int len = sizeof(extAdfHeaders[0]);
    int cnt = sizeof(extAdfHeaders) / len;

    if (!checkFileSizeRange(path, len, -1)) return false;
    
    for (int i = 0; i < cnt; i++) {
        if (matchingFileHeader(path, extAdfHeaders[i], len)) return true;
    }
    return false;
}

EXTFile *
EXTFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    EXTFile *result = new EXTFile();
    
    if (!result->readFromBuffer(buffer, length)) {
        delete result;
        return NULL;
    }
    
    return result;
}

EXTFile *
EXTFile::makeWithFile(const char *path)
{
    EXTFile *result = new EXTFile();
    
    if (!result->readFromFile(path)) {
        delete result;
        return NULL;
    }
    
    return result;
}

EXTFile *
EXTFile::makeWithFile(FILE *file)
{
    EXTFile *result = new EXTFile();
    
    if (!result->readFromFile(file)) {
        delete result;
        return NULL;
    }
    
    return result;
}

bool
EXTFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
        
    return isEXTBuffer(buffer, length);
}

