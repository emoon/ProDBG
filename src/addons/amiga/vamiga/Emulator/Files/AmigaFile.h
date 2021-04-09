// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFileTypes.h"
#include "AmigaObject.h"
#include "Checksum.h"
#include "IO.h"
#include "Reflection.h"
#include <sstream>
#include <fstream>

struct FileTypeEnum : util::Reflection<FileTypeEnum, FileType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < FILETYPE_COUNT;
    }
    
    static const char *prefix() { return "FILETYPE"; }
    static const char *key(FileType value)
    {
        switch (value) {
                
            case FILETYPE_UKNOWN:       return "UKNOWN";
            case FILETYPE_SNAPSHOT:     return "SNAPSHOT";
            case FILETYPE_ADF:          return "ADF";
            case FILETYPE_HDF:          return "HDF";
            case FILETYPE_EXT:          return "EXT";
            case FILETYPE_IMG:          return "IMG";
            case FILETYPE_DMS:          return "DMS";
            case FILETYPE_EXE:          return "EXE";
            case FILETYPE_DIR:          return "DIR";
            case FILETYPE_ROM:          return "ROM";
            case FILETYPE_EXTENDED_ROM: return "EXTENDED_ROM";
            case FILETYPE_COUNT:        return "???";
        }
        return "???";
    }
};

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
 *       |-----------------------------------------------------------
 *       |           |           |           |            |          |
 *   ---------   ---------   ---------   ---------    ---------  ---------
 *  | ADFFile | | EXTFile | | IMGFile | | DMSFile | | EXEFile | | Folder  |
 *   ---------   ---------   ---------   ---------    ---------  ---------
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
        if (!T::isCompatiblePath(path)) throw VAError(ERROR_FILE_TYPE_MISMATCH);
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
        try { return make <T> (path, stream); }
        catch (VAError &exception) { *err = exception.data; }
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
        catch (VAError &exception) { *err = exception.data; }
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
        catch (VAError &exception) { *err = exception.data; }
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
        catch (VAError &exception) { *err = exception.data; }
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
    
    // Determines the type of an arbitrary file on file
    static FileType type(const string &path);
    
    // Returns the type of this file
    virtual FileType type() const { return FILETYPE_UKNOWN; }
            
    // Returns a fingerprint (hash value) for this file
    virtual u64 fnv() const { return util::fnv_1a_64(data, size); }
        
    
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
