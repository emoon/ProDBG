// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSBlock.h"
#include "BootBlockImage.h"

struct FSBootBlock : FSBlock {
        
    FSBootBlock(FSPartition &p, Block nr);
    ~FSBootBlock();
    
    const char *getDescription() const override { return "FSBootBlock"; }

    
    //
    // Methods from Block class
    //

    FSBlockType type() const override { return FS_BOOT_BLOCK; }
    FSVolumeType dos() const override;
    FSItemType itemType(isize byte) const override;
    ErrorCode check(isize pos, u8 *expected, bool strict) const override;
    isize checksumLocation() const override;
    u32 checksum() const override;
    void dump() const override;
    
    
    //
    // Block specific methods
    //

    void writeBootBlock(BootBlockId id, isize page);
};
