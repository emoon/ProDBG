// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "AmigaFile.h"

AmigaFile::AmigaFile()
{
}

AmigaFile::~AmigaFile()
{
    dealloc();
    
    if (path)
        free(path);
}

bool
AmigaFile::alloc(size_t capacity)
{
    dealloc();
    
    if ((data = new u8[capacity]) == NULL)
        return false;
    
    size = eof = capacity;
    fp = 0;
    
    return true;
}

void
AmigaFile::dealloc()
{
    if (data == NULL) {
        assert(size == 0);
        return;
    }
    
    delete[] data;
    data = NULL;
    
    size = 0;
    fp = -1;
    eof = -1;
}

void
AmigaFile::setPath(const char *str)
{
    assert(str != NULL);
    
    // Set path
    if (path) free(path);
    path = strdup(str);
}

void
AmigaFile::seek(long offset)
{
    eof = size;
    fp = (offset < eof) ? offset : -1;
}

int
AmigaFile::read()
{
    int result;
    
    assert(eof <= (long)size);
    
    if (fp < 0)
        return -1;
    
    // Get byte
    result = data[fp++];
    
    // Check for end of file
    if (fp == eof)
        fp = -1;
    
    return result;
}

void
AmigaFile::flash(u8 *buffer, size_t offset)
{
    int byte;
    assert(buffer != NULL);
    
    seek(0);
    
    while ((byte = read()) != EOF) {
        buffer[offset++] = (u8)byte;
    }
}

bool
AmigaFile::readFromBuffer(const u8 *buffer, size_t length)
{
    assert (buffer != NULL);
    
    // Check file type
    if (!bufferHasSameType(buffer, length)) {
        return false;
    }
    
    // Allocate memory
    if (!alloc(length)) {
        return false;
    }
    
    // Read from buffer
    memcpy(data, buffer, length);
 
    return true;
}

bool
AmigaFile::readFromFile(const char *filename)
{
    assert (filename != NULL);
    
    bool success = false;
    u8 *buffer = NULL;
    FILE *file = NULL;
    struct stat fileProperties;
    
    // Check file type
    if (!fileHasSameType(filename)) {
        goto exit;
    }
    
    // Get file properties
    if (stat(filename, &fileProperties) != 0) {
        goto exit;
    }
    
    // Open file
    if (!(file = fopen(filename, "r"))) {
        goto exit;
    }
    
    // Allocate memory
    if (!(buffer = new u8[fileProperties.st_size])) {
        goto exit;
    }
    
    // Read from file
    int c;
    for (unsigned i = 0; i < fileProperties.st_size; i++) {
        c = fgetc(file);
        if (c == EOF)
            break;
        buffer[i] = (u8)c;
    }
    
    // Read from buffer
    dealloc();
    if (!readFromBuffer(buffer, (unsigned)fileProperties.st_size)) {
        goto exit;
    }
    
    setPath(filename);
    success = true;
        
exit:
    
    if (file)
        fclose(file);
    if (buffer)
        delete[] buffer;
    
    return success;
}

bool
AmigaFile::readFromFile(FILE *file)
{
    assert (file != NULL);
    
    u8 *buffer = NULL;

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t size = (size_t)ftell(file);
    rewind(file);
    
    // Allocate memory
    if (!(buffer = new u8[size])) {
        return false;
    }

    // Read from file
    int c;
    for (unsigned i = 0; i < size; i++) {
        if ((c = fgetc(file)) == EOF) break;
        buffer[i] = (u8)c;
    }

    // Check type
    if (!bufferHasSameType(buffer, size)) {
        delete[] buffer;
        return false;
    }
    
    // Read from buffer
    dealloc();
    if (!readFromBuffer(buffer, size)) {
        delete[] buffer;
        return false;
    }
    
    delete[] buffer;
    return true;
}

size_t
AmigaFile::writeToBuffer(u8 *buffer)
{
    assert(data != NULL);
    
    if (buffer) {
        memcpy(buffer, data, size);
    }
    return size;
}

bool
AmigaFile::writeToFile(const char *filename)
{
    assert (filename != NULL);

    bool success = false;
    u8 *data = NULL;
    FILE *file;
    size_t filesize;
    
    // Determine the size of the file in bytes
    if (!(filesize = writeToBuffer(NULL))) return false;
    
    // Open file
    if (!(file = fopen(filename, "w"))) goto exit;
    
    // Allocate a buffer
    if (!(data = new u8[filesize])) goto exit;
    
    // Write contents to the created buffer
    if (!writeToBuffer(data)) goto exit;
    
    // Write the buffer to a file
    for (unsigned i = 0; i < filesize; i++) fputc(data[i], file);
    success = true;
    
exit:
    
    if (file)
        fclose(file);
    if (data)
        delete[] data;
    
    return success;
}
