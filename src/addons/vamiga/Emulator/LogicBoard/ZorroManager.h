// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ZORRO_MANAGER_H
#define _ZORRO_MANAGER_H

#include "AmigaComponent.h"

/* Additional information:
 *
 *   Fast Ram emulation (Zorro II) is based on:
 *   github.com/PR77/A500_ACCEL_RAM_IDE-Rev-1/blob/master/Logic/RAM/A500_RAM.v
 */

// Manager for plugged in Zorro II devices
class ZorroManager : public AmigaComponent {

    // Value returned when peeking into the auto-config space
    u8 autoConfData;
    
    // Current configuration state (0 = unconfigured)
    u8 fastRamConf;
    
    // Base address of the Fast Ram (provided by Kickstart)
    u32 fastRamBaseAddr;
    
    
    //
    // Initializing
    //
    
public:
    
    ZorroManager(Amiga& ref);

private:
    
    void _reset(bool hard) override { RESET_SNAPSHOT_ITEMS(hard) }

    
    //
    // Serializing
    //

public:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & autoConfData
        & fastRamConf
        & fastRamBaseAddr;
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

        
    //
    // Emulating Fast Ram
    //
    
public:
    
    u8 peekFastRamDevice(u32 addr);
    u8 spypeekFastRamDevice(u32 addr);
    void pokeFastRamDevice(u32 addr, u8 value);
};

#endif
