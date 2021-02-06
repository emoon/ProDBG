//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_ROOT_BLOCK_H
#define _FS_ROOT_BLOCK_H

#include "FSBlock.h"

struct FSRootBlock : FSBlock {
          
    FSRootBlock(FSVolume &ref, u32 nr);
    FSRootBlock(FSVolume &ref, u32 nr, const char *name);
    ~FSRootBlock();

    // Methods from Block class
    FSBlockType type() override { return FS_ROOT_BLOCK; }
    void dump() override;
    bool check(bool verbose) override;
    void updateChecksum() override;

    FSName getName() override                    { return FSName(addr(-20)); }
    void setName(FSName name) override           { name.write(addr(-20));    }

    FSTime getCreationDate() override            { return FSTime(addr(-7));  }
    void setCreationDate(FSTime t) override      { t.write(addr(-7));        }

    FSTime getModificationDate() override        { return FSTime(addr(-23)); }
    void setModificationDate(FSTime t) override  { t.write(addr(-23));       }

    u32 hashTableSize() override { return 72; }
};

#endif
