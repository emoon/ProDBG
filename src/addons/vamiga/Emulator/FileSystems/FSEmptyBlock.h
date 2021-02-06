// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_EMPTY_BLOCK_H
#define _FS_EMPTY_BLOCK_H

#include "FSBlock.h"

struct FSEmptyBlock : FSBlock {
    
    FSEmptyBlock(FSVolume &ref, u32 nr);
    ~FSEmptyBlock();
     
    FSBlockType type() override { return FS_EMPTY_BLOCK; }

    void exportBlock(u8 *p, size_t bsize) override;
};

#endif
