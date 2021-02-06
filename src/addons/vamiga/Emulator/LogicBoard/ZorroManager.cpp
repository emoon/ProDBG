// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

ZorroManager::ZorroManager(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("ZorroManager");
}

u8
ZorroManager::peekFastRamDevice(u32 addr)
{    
    trace(FAS_DEBUG, "peekFastRamDevice(%X)\n", addr & 0xFFFF);
    // debug(FAS_DEBUG, "fastRamSize = %d\n", mem.fastRamSize());

    if (fastRamConf || mem.fastRamSize() == 0) return 0xF; // Already configured
    
    /* Register pair 00/02 (er_Type)
     *
     * Bits 7,6:  PIC type
     *              00 = Reserved
     *              01 = Reserved
     *              10 = Zorro III
     *              11 = Zorro II
     * Bit 5:     Link memory free pool
     * Bit 4:     Read auto-boot Rom
     * Bit 3:     Next board is linked to this one
     * Bit 2,1,0: Configutration size
     *              000 = 8 megabytes
     *              001 = 64 kilobytes
     *              010 = 128 kilobytes
     *              011 = 256 kilobytes
     *              100 = 512 kilobytes
     *              101 = 1 megabyte
     *              110 = 2 megabytes
     *              111 = 4 megabytes
     */
    u8 erTypeHi = 0b1110; // Zorro II, Free pool, Don't boot
    u8 erTypeLo;
    
    switch (mem.fastRamSize()) {
        case KB(64):  erTypeLo = 0b001; break;
        case KB(128): erTypeLo = 0b010; break;
        case KB(256): erTypeLo = 0b011; break;
        case KB(512): erTypeLo = 0b100; break;
        case MB(1):   erTypeLo = 0b101; break;
        case MB(2):   erTypeLo = 0b110; break;
        case MB(4):   erTypeLo = 0b111; break;
        case MB(8):   erTypeLo = 0b000; break;
        default: assert(false);
    }
    
    /* Register pair 08/0A (er_flags) Note: Bits must be returned negated.
     *
     * Bits 7:    Location indicator
     * Bits 6:    Shut up (0 = can be shut up by software)
     * Bits 5:    Size extension bit
     * Bits 4:    Reserved (1 for Zorro III)
     * Bits 3-0:  Board's sub size (0000 = matches physical size)
     */
    u8 erFlagsHi = 0b0111;
    u8 erFlagsLo = 0b1111; // Logical and size match
    
    switch (addr & 0xFFFF) {
            
        case 0x00: // er_Type (upper nibble)
            if (fastRamConf == 0) autoConfData = erTypeHi;
            break;
            
        case 0x02: // er_Type (lower nibble)
            if (fastRamConf == 0) autoConfData = erTypeLo;
            break;
            
        case 0x04: // er_Product (upper nibble)
            autoConfData = 0x9;
            break;
            
        case 0x06: // er_Product (lower nibble)
            if (fastRamConf == 0) autoConfData = 0x8;
            break;
            
        case 0x08: // er_Flags (upper nibble)
            autoConfData = erFlagsHi;
            break;
            
        case 0x0A: // er_Flags (lower nibble)
            autoConfData = erFlagsLo;
            break;
            
        case 0x0C: case 0x0E: // er_Reserved03 (must be 0)
            autoConfData = 0xF;
            break;
            
        case 0x10: // er_Manufacturer (upper nibble of high byte)
            autoConfData = 0xF;
            break;
            
        case 0x12: // er_Manufacturer (lower nibble of high byte)
            autoConfData = 0x8;
            break;
            
        case 0x14: // er_Manufacturer (upper nibble of low byte)
            autoConfData = 0x4;
            break;
            
        case 0x16: // er_Manufacturer (lower nibble of low byte)
            autoConfData = 0x6;
            break;
            
        case 0x18: // er_SerialNumber (upper nibble of byte 0 (msb))
            autoConfData = 0xA;
            break;
            
        case 0x1A: // er_SerialNumber (lower nibble of byte 0 (msb))
            autoConfData = 0xF;
            break;
            
        case 0x1C: // er_SerialNumber (upper nibble of byte 1)
            autoConfData = 0xB;
            break;
            
        case 0x1E: // er_SerialNumber (lower nibble of byte 1)
            autoConfData = 0xE;
            break;
            
        case 0x20: // er_SerialNumber (upper nibble of byte 2)
            autoConfData = 0xA;
            break;
            
        case 0x22: // er_SerialNumber (lower nibble of byte 2)
            autoConfData = 0xA;
            break;
            
        case 0x24: // er_SerialNumber (upper nibble of byte 3 (lsb))
            autoConfData = 0xB;
            break;
            
        case 0x26: // er_SerialNumber (lower nibble of byte 3 (lsb))
            autoConfData = 0x3;
            break;
            
        default:
            autoConfData = 0xF;
    }
    
    trace(FAS_DEBUG, "autoConfData = %x\n", autoConfData);
    return autoConfData;
}

u8
ZorroManager::spypeekFastRamDevice(u32 addr)
{
    return peekFastRamDevice(addr);
}

void
ZorroManager::pokeFastRamDevice(u32 addr, u8 value)
{
    if (mem.fastRamSize() == 0) return;
    
    // debug("pokeFastRamDevice(%X, %X)\n", addr, value);
    
    switch (addr & 0xFFFF) {
            
        case 0x44: // ec_BaseAddress (A31 - A28, 0xX---0000, Zorro III)
            return;
            
        case 0x46: // ec_BaseAddress (A27 - A24, 0x-X--0000, Zorro III)
            return;
            
        case 0x48: // ec_BaseAddress (A23 - A20, 0x--X-0000)
            fastRamBaseAddr |= (value & 0xF0) << 16;
            trace(FAS_DEBUG, "Zorro II card mapped to $%x\n", fastRamBaseAddr);
            
            /* "Note that writing to register 48 actually configures the board for
             *  both Zorro II and Zorro III boards in the Zorro II configuration
             *  block." [HRM 3rd]
             */
            fastRamConf = 1;
            return;
            
        case 0x4A: // ec_BaseAddress (A19 - A16, 0x---X0000)
            fastRamBaseAddr = (value & 0xF0) << 12;
            return;
            
        default:
            return;
    }
}
