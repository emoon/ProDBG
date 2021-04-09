// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AmigaFile.h"
#include "Snapshot.h"
#include "ADFFile.h"
#include "EXTFile.h"
#include "IMGFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "Folder.h"
#include "HDFFile.h"
#include "RomFile.h"
#include "ExtendedRomFile.h"

AmigaFile::AmigaFile(isize capacity)
{
    data = new u8[capacity]();
    size = capacity;
}

AmigaFile::~AmigaFile()
{
    if (data) delete[] data;
}

void
AmigaFile::flash(u8 *buffer, isize offset)
{
    assert(buffer != nullptr);
    memcpy(buffer + offset, data, size);
}

FileType
AmigaFile::type(const string &path)
{
    std::ifstream stream(path);
    if (!stream.is_open()) return FILETYPE_UKNOWN; // throw VAError(ERROR_FILE_NOT_FOUND);

    printf("File was found type()\n");
    
    if (Snapshot::isCompatiblePath(path) &&
        Snapshot::isCompatibleStream(stream)) return FILETYPE_SNAPSHOT;

    if (ADFFile::isCompatiblePath(path) &&
        ADFFile::isCompatibleStream(stream)) return FILETYPE_ADF;

    if (HDFFile::isCompatiblePath(path) &&
        HDFFile::isCompatibleStream(stream)) return FILETYPE_HDF;

    if (EXTFile::isCompatiblePath(path) &&
        EXTFile::isCompatibleStream(stream)) return FILETYPE_EXT;

    if (IMGFile::isCompatiblePath(path) &&
        IMGFile::isCompatibleStream(stream)) return FILETYPE_IMG;

    if (DMSFile::isCompatiblePath(path) &&
        DMSFile::isCompatibleStream(stream)) return FILETYPE_DMS;

    if (EXEFile::isCompatiblePath(path) &&
        EXEFile::isCompatibleStream(stream)) return FILETYPE_EXE;

    if (RomFile::isCompatiblePath(path) &&
        RomFile::isCompatibleStream(stream)) return FILETYPE_ROM;

    if (EXTFile::isCompatiblePath(path) &&
        EXTFile::isCompatibleStream(stream)) return FILETYPE_EXT;

    if (Folder::isFolder(path.c_str())) return FILETYPE_DIR;

    return FILETYPE_UKNOWN;
}

isize
AmigaFile::readFromStream(std::istream &stream)
{
    // Get stream size
    auto fsize = stream.tellg();
    stream.seekg(0, std::ios::end);
    fsize = stream.tellg() - fsize;
    stream.seekg(0, std::ios::beg);

    // Allocate memory
    assert(data == nullptr);
    data = new u8[fsize]();
    size = (isize)fsize;

    // Read from stream
    stream.read((char *)data, size);
        
    return size;
}

isize
AmigaFile::readFromFile(const char *path)
{
    assert(path);
        
    std::ifstream stream(path);

    if (!stream.is_open()) {
        throw VAError(ERROR_FILE_CANT_READ);
    }
    
    isize result = readFromStream(stream);
    assert(result == size);
    
    this->path = string(path);
    return result;
}

isize
AmigaFile::readFromBuffer(const u8 *buf, isize len)
{
    assert(buf);

    std::istringstream stream(string((const char *)buf, len));
    
    isize result = readFromStream(stream);
    assert(result == size);
    
    return result;
}

isize
AmigaFile::writeToStream(std::ostream &stream)
{
    stream.write((char *)data, size);
    return size;
}

isize
AmigaFile::writeToStream(std::ostream &stream, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return writeToStream(stream); }
    catch (VAError &exception) { *err = exception.data; }
    return 0;
}

isize
AmigaFile::writeToFile(const char *path)
{
    assert(path);
        
    std::ofstream stream(path);

    if (!stream.is_open()) {
        throw VAError(ERROR_FILE_CANT_WRITE);
    }
    
    isize result = writeToStream(stream);
    assert(result == size);
    
    return result;
}

isize
AmigaFile::writeToFile(const char *path, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return writeToFile(path); }
    catch (VAError &exception) { *err = exception.data; }
    return 0;
}

isize
AmigaFile::writeToBuffer(u8 *buf)
{
    assert(buf);

    std::ostringstream stream;
    isize len = writeToStream(stream);
    stream.write((char *)buf, len);
    
    return len;
}

isize
AmigaFile::writeToBuffer(u8 *buf, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return writeToBuffer(buf); }
    catch (VAError &exception) { *err = exception.data; }
    return 0;
}
