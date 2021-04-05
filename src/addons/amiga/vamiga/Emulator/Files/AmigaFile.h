// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_FILE_H
#define _AMIGA_FILE_H

#include "AmigaObject.h"

/* Base class of all file readable types. It provides the basic functionality
 * for reading and writing files.
 */
class AmigaFile : public AmigaObject {
    
protected:
    
    // Physical location of this file on disk
    char *path = NULL;
    
    // The raw data of this file
    u8 *data = NULL;
    
    // The size of this file in bytes
    size_t size = 0;
    
    /* File pointer
     * An offset into the data array with -1 indicating EOF
     */
    long fp = -1;
    
    /* End of file position
     * This value equals the last valid offset plus 1
     */
    long eof = -1;
    
    
    //
    // Initializing
    //
    
public:
    
    AmigaFile();
    virtual ~AmigaFile();
    
    // Allocates memory for storing the object data
    virtual bool alloc(size_t capacity);
    
    // Frees the allocated memory
    virtual void dealloc();
    
    
    //
    // Accessing file attributes
    //
    
    // Returns the type of this file
    virtual AmigaFileType fileType() { return FILETYPE_UKNOWN; }
    
    // Returns a string representation of the file type, e.g., "ADF"
    virtual const char *typeAsString() { return ""; }
    
    // Returns the physical name of this file
    const char *getPath() { return path ? path : ""; }
    
    // Sets the physical name of this file
    void setPath(const char *path);
    
    // Returns a fingerprint (hash value) for this file
    virtual u64 fnv() { return fnv_1a_64(data, size); }
    
    
    //
    // Reading data from the file
    //
    
    //  Returns the number of bytes in this file
    virtual size_t getSize() { return size; }
    
    // Moves the file pointer to the specified offset
    virtual void seek(long offset);
    
    //  Reads a byte (returns -1 (EOF) if the end of file has been reached)
    virtual int read();
        
    // Copies the whole file data into a buffer
    virtual void flash(u8 *buffer, size_t offset = 0);
    
    
    //
    // Serializing
    //
    
    // Returns the required buffer size for this file
    size_t sizeOnDisk() { return writeToBuffer(NULL); }

    /* Returns true iff this specified buffer is compatible with this object.
     * This function is used in readFromBuffer().
     */
    virtual bool bufferHasSameType(const u8 *buffer, size_t length) { return false; }


    /* Returns true iff this specified file is compatible with this object.
     * This function is used in readFromFile().
     */
    virtual bool fileHasSameType(const char *path) { return false; }
    
    /* Deserializes this object from a memory buffer. This function uses
     * bufferHasSameType() to verify that the buffer contains a compatible
     * binary representation.
     */
    virtual bool readFromBuffer(const u8 *buffer, size_t length);
    
    /* Deserializes this object from a file. This function uses
     * fileHasSameType() to verify that the file contains a compatible binary
     * representation. This function requires no custom implementation. It
     * first reads in the file contents in memory and invokes readFromBuffer
     * afterwards.
     */
    virtual bool readFromFile(const char *filename);

    /* Deserializes this object from a file that is already open.
     */
    virtual bool readFromFile(FILE *file);

    /* Writes the file contents into a memory buffer. If a NULL pointer is
     * passed in, a test run is performed. Test runs can be performed to
     * determine the size of the file on disk.
     */
    virtual size_t writeToBuffer(u8 *buffer);
    
    /* Writes the file contents to a file. This function requires no custom
     * implementation. It invokes writeToBuffer first and writes the data to
     * disk afterwards.
     */
    virtual bool writeToFile(const char *filename);
};

#endif
