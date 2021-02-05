// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaObject.h"
#include "FileTypes.h"

/* All media files are organized in the class hierarchy displayed below. Two
 * abstract classes are involed: AmigaFile and DiskFile. AmigaFile provides
 * basic functionality for reading and writing files, streams, and buffers.
 * DiskFile provides an abstract interface for accessing media files that will
 * be mounted as a virtual floppy disk.
 *
 *  ------------
 * | AmigaFile  |
 *  ------------
 *       |
 *       |---------------------------------------------------
 *       |           |           |            |              |
 *       |      ----------   ---------   ---------   -----------------
 *       |     | Snapshot | | HDFFile | | RomFile | | ExtendedRomFile |
 *       |      ----------   ---------   ---------   -----------------
 *       |
 *  ------------
 * |  DiskFile  |
 *  ------------
 *       |
 *       |------------------------------------------------------------
 *       |           |           |           |            |           |
 *   ---------   ---------   ---------   ---------    ---------   ---------
 *  | ADFFile | | EXTFile | | IMGFile | | DMSFile | | EXEFile | | Folder  |
 *   ---------   ---------   ---------   ---------    ---------   ---------
 */

class AmigaFile : public AmigaObject {
    
public:
    
    // Physical location of this file
    string path = "";
    
    // The raw data of this file
    u8 *data = nullptr;
    
    // The size of this file in bytes
    isize size = 0;
    
    
    //
    // Creating
    //
    
public:
    
    template <class T> static T *make(const string &path, std::istream &stream) throws
    {
        if (!T::isCompatibleStream(stream)) throw VAError(ERROR_FILE_TYPE_MISMATCH);
        
        T *obj = new T();
        obj->path = path;
        
        try { obj->readFromStream(stream); } catch (VAError &err) {
            delete obj;
            throw err;
        }
        return obj;
    }

    template <class T> static T *make(const string &path, std::istream &stream, ErrorCode *err)
    {
        *err = ERROR_OK;
        try { return make <T> (stream); }
        catch (VAError &exception) { *err = exception.errorCode; }
        return nullptr;
    }
    
    template <class T> static T *make(const u8 *buf, isize len) throws
    {
        std::stringstream stream;
        stream.write((const char *)buf, len);
        return make <T> ("", stream);
    }
    
    template <class T> static T *make(const u8 *buf, isize len, ErrorCode *err)
    {
        *err = ERROR_OK;
        try { return make <T> (buf, len); }
        catch (VAError &exception) { *err = exception.errorCode; }
        return nullptr;
    }
    
    template <class T> static T *make(const char *path) throws
    {
        std::ifstream stream(path);
        if (!stream.is_open()) throw VAError(ERROR_FILE_NOT_FOUND);

        T *file = make <T> (string(path), stream);
        return file;
    }

    template <class T> static T *make(const char *path, ErrorCode *err)
    {
        *err = ERROR_OK;
        try { return make <T> (path); }
        catch (VAError &exception) { *err = exception.errorCode; }
        return nullptr;
    }

    template <class T> static T *make(FILE *file) throws
    {
        std::stringstream stream;
        int c; while ((c = fgetc(file)) != EOF) { stream.put(c); }
        return make <T> ("", stream);
    }
    
    template <class T> static T *make(FILE *file, ErrorCode *err)
    {
        *err = ERROR_OK;
        try { return make <T> (file); }
        catch (VAError &exception) { *err = exception.errorCode; }
        return nullptr;
    }
    
    
    //
    // Initializing
    //
    
public:

    AmigaFile() { };
    AmigaFile(isize capacity);
    virtual ~AmigaFile();
        
    
    //
    // Accessing file attributes
    //
    
    // Returns the type of this file
    virtual FileType type() const { return FILETYPE_UKNOWN; }
            
    // Returns a fingerprint (hash value) for this file
    virtual u64 fnv() const { return fnv_1a_64(data, size); }
        
    
    //
    // Flashing data
    //
            
    // Copies the file contents into a buffer starting at the provided offset
    virtual void flash(u8 *buf, isize offset = 0);
    
    
    //
    // Serializing
    //
    
protected:
    
    virtual isize readFromStream(std::istream &stream) throws;
    isize readFromFile(const char *path) throws;
    isize readFromBuffer(const u8 *buf, isize len) throws;

public:
    
    virtual isize writeToStream(std::ostream &stream) throws;
    isize writeToStream(std::ostream &stream, ErrorCode *err);

    isize writeToFile(const char *path) throws;
    isize writeToFile(const char *path, ErrorCode *err);
    
    isize writeToBuffer(u8 *buf) throws;
    isize writeToBuffer(u8 *buf, ErrorCode *err);    
};
