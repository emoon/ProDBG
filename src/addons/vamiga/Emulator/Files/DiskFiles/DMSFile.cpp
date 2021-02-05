// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "DMSFile.h"

extern "C" {
unsigned short extractDMS(FILE *fi, FILE *fo);
}

bool
DMSFile::isCompatibleName(const string &name)
{
    return name == "dms" || name == "DMS";
}

bool
DMSFile::isCompatibleStream(std::istream &stream)
{
    u8 signature[] = { 'D', 'M', 'S', '!' };
                                                                                            
    return matchingStreamHeader(stream, signature, sizeof(signature));
}

isize
DMSFile::readFromStream(std::istream &stream)
{
    FILE *fpi, *fpo;
    char *pi, *po;
    size_t si, so;
    
    isize result = AmigaFile::readFromStream(stream);
        
    // We use a third-party tool called xdms to convert the DMS file into an
    // ADF file. Originally, xdms is a command line utility that is designed
    // to work with the file system. To ease the integration of this tool, we
    // utilize memory streams for getting data in and out.

    // Setup input stream
    fpi = open_memstream(&pi, &si);
    for (isize i = 0; i < size; i++) putc(data[i], fpi);
    fclose(fpi);
    
    // Setup output file
    fpi = fmemopen(pi, si, "r");
    fpo = open_memstream(&po, &so);
    extractDMS(fpi, fpo);
    fclose(fpi);
    fclose(fpo);
    
    // Create ADF
    fpo = fmemopen(po, so, "r");
    adf = AmigaFile::make <ADFFile> (fpo);
    fclose(fpo);
    
    if (!adf) throw VAError(ERROR_UNKNOWN);
    return result;
}
