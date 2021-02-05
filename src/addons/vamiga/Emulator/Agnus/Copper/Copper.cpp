// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Copper::Copper(Amiga& ref) : AmigaComponent(ref)
{
}

void
Copper::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

void
Copper::_inspect()
{    
    synchronized {
        
        info.copList = copList;
        info.active  = agnus.isPending<SLOT_COP>();
        info.cdang   = cdang;
        info.coppc   = coppc & agnus.ptrMask;
        info.cop1lc  = cop1lc & agnus.ptrMask;
        info.cop2lc  = cop2lc & agnus.ptrMask;
        info.cop1ins = cop1ins;
        info.cop2ins = cop2ins;
        info.length1 = (cop1end - cop1lc) / 4;
        info.length2 = (cop2end - cop2lc) / 4;
    }
}

void
Copper::_dump() const
{
    bool active = agnus.isPending<SLOT_COP>();
    msg("    cdang: %d\n", cdang);
    msg("   active: %s\n", active ? "yes" : "no");
    if (active) msg("    state: %ld\n", (long)agnus.slot[SLOT_COP].id);
    msg("    coppc: %X\n", coppc);
    msg("  copins1: %X\n", cop1ins);
    msg("  copins2: %X\n", cop2ins);
    msg("   cop1lc: %X\n", cop1lc);
    msg("   cop2lc: %X\n", cop2lc);
    msg("  cop1end: %X\n", cop1end);
    msg("  cop2end: %X\n", cop2end);
}

void
Copper::advancePC()
{
    coppc += 2;
}

void
Copper::switchToCopperList(isize nr)
{
    assert(nr == 1 || nr == 2);

    // debug("switchToCopperList(%d) coppc: %x -> %x\n", nr, coppc, (nr == 1) ? cop1lc : cop2lc);
    coppc = (nr == 1) ? cop1lc : cop2lc;
    copList = nr;
    agnus.scheduleRel<SLOT_COP>(0, COP_REQ_DMA);
}

bool
Copper::findMatch(Beam &result) const
{
    i16 vMatch, hMatch;

    // Get the current beam position
    Beam b = agnus.pos;

    // Set up the comparison positions
    i16 vComp = getVP();
    i16 hComp = getHP();

    // Set up the comparison masks
    i16 vMask = getVM() | 0x80;
    i16 hMask = getHM() & 0xFE;

    // Check if the current line is already below the vertical trigger position
    if ((b.v & vMask) > (vComp & vMask)) {

        // Success. The current position already matches
        result = b;
        return true;
    }

    // Check if the current line matches the vertical trigger position
    if ((b.v & vMask) == (vComp & vMask)) {

        // Check if we find a horizontal match in this line
        if (findHorizontalMatch(b.h, hComp, hMask, hMatch)) {

            // Success. We've found a match in the current line
            result.v = b.v;
            result.h = hMatch;
            return true;
        }
    }

    // Find the first vertical match below the current line
    if (!findVerticalMatch(b.v + 1, vComp, vMask, vMatch)) return false;

    // Find the first horizontal match in that line
    if (!findHorizontalMatch(0, hComp, hMask, hMatch)) return false;

    // Success. We've found a match below the current line
    result.v = vMatch;
    result.h = hMatch;
    return true;
}

bool
Copper::findVerticalMatch(i16 vStrt, i16 vComp, i16 vMask, i16 &result) const
{
    i16 vStop = agnus.frame.numLines();

    // Iterate through all vertical positions
    for (int v = vStrt; v < vStop; v++) {

        // Check if the comparator triggers at this position
        if ((v & vMask) >= (vComp & vMask)) {
            result = v;
            return true;
        }
    }
    return false;
}

bool
Copper::findHorizontalMatch(i16 hStrt, i16 hComp, i16 hMask, i16 &result) const
{
    i16 hStop = HPOS_CNT;

    // Iterate through all horizontal positions
    for (int h = hStrt; h < hStop; h++) {

        // Check if the comparator triggers at this position
        if ((h & hMask) >= (hComp & hMask)) {
            result = h;
            return true;
        }
    }
    return false;
}

bool
Copper::findMatchNew(Beam &match) const
{
    // Start searching at the current beam position
    u32 beam = (agnus.pos.v << 8) | agnus.pos.h;

    // Get the comparison position and the comparison mask
    u32 comp = getVPHP();
    u32 mask = getVMHM();

    // Iterate through all lines starting from the current position
    u32 numLines = agnus.frame.numLines();
    while ((beam >> 8) < numLines) {

        // Check if the vertical components are equal
        if ((beam & mask & ~0xFF) == (comp & mask & ~0xFF)) {

            // debug("Matching vertically: beam = %X comp = %X mask = %X\n", beam, comp, mask);

            // Try to match the horizontal coordinate as well
            if (findHorizontalMatchNew(beam, comp, mask)) {

                // Success
                match.v = beam >> 8;
                match.h = beam & 0xFF;
                return true;
            }
        }

        // Check if the vertical beam position is greater
        else if ((beam & mask & ~0xFF) > (comp & mask & ~0xFF)) {

            // Success
            match.v = beam >> 8;
            match.h = beam & 0xFF;
            return true;
        }

        // Jump to the beginning of the next line
        beam = (beam & ~0xFF) + 0x100;
    }

    return false;
}

bool
Copper::findHorizontalMatchNew(u32 &match, u32 comp, u32 mask) const
{
    // Iterate through all horizontal positions
    for (u32 beam = match; (beam & 0xFF) < HPOS_CNT; beam++) {

        // Check if the comparator triggers at this position
        if ((beam & mask) >= (comp & mask)) {

            // Success
            match = beam;
            return true;
        }
    }

    return false;
}

void
Copper::move(u32 addr, u16 value)
{
    trace(COP_DEBUG,
          "COPPC: %X move(%s, $%X) (%d)\n", coppc, regName(addr), value, value);

    assert(IS_EVEN(addr));
    assert(addr < 0x1FF);

    // Catch registers with special timing needs

    if (addr >= 0x180 && addr <= 0x1BE) {

        trace(OCSREG_DEBUG, "pokeCustom16(%X [%s], %X)\n", addr, regName(addr), value);

        // Color registers
        pixelEngine.colChanges.insert(4 * agnus.pos.h, RegChange { addr, value} );
        return;
    }

    // Write the value
    agnus.doCopperDMA(addr, value);
}

#if 0
bool
Copper::comparator(u32 beam, u32 waitpos, u32 mask)
{
    // Get comparison bits for the vertical beam position
    u8 vBeam = (beam >> 8) & 0xFF;
    u8 vWaitpos = (waitpos >> 8) & 0xFF;
    u8 vMask = (mask >> 8) | 0x80;
    
    trace(COP_DEBUG && verbose,
          " * vBeam = %X vWaitpos = %X vMask = %X\n", vBeam, vWaitpos, vMask);

    // Compare vertical positions
    if ((vBeam & vMask) < (vWaitpos & vMask)) {
        // debug("beam %d waitpos %d mask 0x%x FALSE\n", beam, waitpos, mask);
        return false;
    }
    if ((vBeam & vMask) > (vWaitpos & vMask)) {
        // debug("beam %d waitpos %d mask 0x%x TRUE\n", beam, waitpos, mask);
        return true;
    }

    // Get comparison bits for horizontal position
    u8 hBeam = beam & 0xFE;
    u8 hWaitpos = waitpos & 0xFE;
    u8 hMask = mask & 0xFE;

    trace(COP_DEBUG && verbose,
          " * hBeam = %X hWaitpos = %X hMask = %X\n", hBeam, hWaitpos, hMask);
    /*
    debug("Comparing horizontal position waitpos = %d vWait = %d hWait = %d \n", waitpos, vWaitpos, hWaitpos);
    debug("hBeam = %d ($x) hMask = %X\n", hBeam, hBeam, hMask);
    debug("Result = %d\n", (hBeam & hMask) >= (hWaitpos & hMask));
    */

    // Compare horizontal positions
    return (hBeam & hMask) >= (hWaitpos & hMask);
}
#endif

bool
Copper::comparator(Beam beam, u16 waitpos, u16 mask) const
{
    // Get comparison bits for the vertical beam position
    u8 vBeam = beam.v & 0xFF;
    u8 vWaitpos = HI_BYTE(waitpos);
    u8 vMask = HI_BYTE(mask) | 0x80;

    // msg(" * vBeam = %X vWaitpos = %X vMask = %X\n", vBeam, vWaitpos, vMask);

    // Compare vertical positions
    if ((vBeam & vMask) < (vWaitpos & vMask)) {
        // debug("beam %d waitpos %d mask 0x%x FALSE\n", beam, waitpos, mask);
        return false;
    }
    if ((vBeam & vMask) > (vWaitpos & vMask)) {
        // debug("beam %d waitpos %d mask 0x%x TRUE\n", beam, waitpos, mask);
        return true;
    }

    // Get comparison bits for horizontal position
    u8 hBeam = beam.h & 0xFE;
    u8 hWaitpos = LO_BYTE(waitpos) & 0xFE;
    u8 hMask = LO_BYTE(mask) & 0xFE;

    // msg(" * hBeam = %X hWaitpos = %X hMask = %X\n", hBeam, hWaitpos, hMask);
    /*
     debug("Comparing horizontal position waitpos = %d vWait = %d hWait = %d \n", waitpos, vWaitpos, hWaitpos);
     debug("hBeam = %d ($x) hMask = %X\n", hBeam, hBeam, hMask);
     debug("Result = %d\n", (hBeam & hMask) >= (hWaitpos & hMask));
     */

    // Compare horizontal positions
    return (hBeam & hMask) >= (hWaitpos & hMask);
}

bool
Copper::comparator(Beam beam) const
{
    return comparator(beam, getVPHP(), getVMHM());
}

bool
Copper::comparator() const
{
    return comparator(agnus.pos);
}

void
Copper::scheduleWaitWakeup(bool bfd)
{
    Beam trigger;

    // Find the trigger position for this WAIT command
    if (findMatchNew(trigger)) {

        // In how many cycles do we get there?
        int delay = trigger - agnus.pos;

        // msg("(%d,%d) matches in %d cycles\n", trigger.v, trigger.h, delay);

        if (delay == 0) {

            // Copper does not stop
            agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(2), COP_FETCH);

        } else if (delay == 2) {

            // Copper does not stop
            agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(2), COP_FETCH);

        } else {

            // Wake up 2 cycles earlier with a WAKEUP event
            delay -= 2;
            if (bfd) {
                agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(delay), COP_WAKEUP);
            } else {
                agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(delay), COP_WAKEUP_BLIT);
            }
        }

    } else {

        // msg("(%d,%d) does not match in this frame\n", trigger.v, trigger.h);
        agnus.scheduleAbs<SLOT_COP>(NEVER, COP_REQ_DMA);
    }
}

bool
Copper::isMoveCmd() const
{
    return !(cop1ins & 1);
}

bool Copper::isMoveCmd(u32 addr) const
{
    assert(IS_EVEN(addr));

    u16 hiword = mem.spypeek16 <ACCESSOR_AGNUS> (addr);

    return IS_EVEN(hiword);
}

bool Copper::isWaitCmd() const
{
     return (cop1ins & 1) && !(cop2ins & 1);
}

bool Copper::isWaitCmd(u32 addr) const
{
    assert(IS_EVEN(addr));

    u16 hiword = mem.spypeek16 <ACCESSOR_AGNUS> (addr);
    u16 loword = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);

    return IS_ODD(hiword) && IS_EVEN(loword);
}

u16
Copper::getRA() const
{
    return cop1ins & 0x1FE;
}

u16
Copper::getRA(u32 addr) const
{
    u16 hiword = mem.spypeek16 <ACCESSOR_AGNUS> (addr);
    return hiword & 0x1FE;
}

u16
Copper::getDW() const
{
    return cop1ins;
}

u16
Copper::getDW(u32 addr) const
{
    u16 loword = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);
    return loword;
}

bool
Copper::getBFD() const
{
    return (cop2ins & 0x8000) != 0;
}

bool
Copper::getBFD(u32 addr) const
{
    u16 instr = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);
    return (instr & 0x8000) != 0;
}

u16
Copper::getVPHP() const
{
    return cop1ins & 0xFFFE;
}

u16
Copper::getVPHP(u32 addr) const
{
    u16 instr = mem.spypeek16 <ACCESSOR_AGNUS> (addr);
    return instr & 0xFFFE;
}

u16
Copper::getVMHM() const
{
    return (cop2ins & 0x7FFE) | 0x8001;
}

u16
Copper::getVMHM(u32 addr) const
{
    u16 instr = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);
    return (instr & 0x7FFE) | 0x8001;
}

bool
Copper::isIllegalAddress(u32 addr) const
{
    if (cdang) {
        return agnus.isOCS() ? addr < 0x40 : false;
    } else {
        return addr < 0x80;
    }
}

bool
Copper::isIllegalInstr(u32 addr) const
{
    return isMoveCmd(addr) && isIllegalAddress(getRA(addr));
}

void
Copper::vsyncHandler()
{
    /* "At the start of each vertical blanking interval, COP1LC is automatically
     *  used to start the program counter. That is, no matter what the Copper is
     *  doing, when the end of vertical blanking occurs, the Copper is
     *  automatically forced to restart its operations at the address contained
     *  in COP1LC." [HRM]
     */
    agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(0), COP_VBLANK);
    
    if (COP_CHECKSUM) {
        
        if (checkcnt) {
            msg("[%lld] Checksum: %x (%lld) lc1 = %x lc2 = %x\n",
                agnus.frame.nr, checksum, checkcnt, cop1lc, cop2lc);
        }
        checkcnt = 0;
        checksum = fnv_1a_init32();
    }
}

void
Copper::blitterDidTerminate()
{
    if (agnus.hasEvent<SLOT_COP>(COP_WAIT_BLIT)) {

        // Wake up the Copper in the next even cycle
        if (IS_EVEN(agnus.pos.h)) {
            serviceEvent(COP_WAIT_BLIT);
        } else {
            agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(1), COP_WAIT_BLIT);
        }
    }
}

isize
Copper::instrCount(isize nr) const
{
    assert(nr == 1 || nr == 2);

    int strt = (nr == 1) ? cop1lc  : cop2lc;
    int stop = (nr == 1) ? cop1end : cop2end;

    return MAX(0, 1 + (stop - strt) / 4);
}

void
Copper::adjustInstrCount(isize nr, isize offset)
{
    assert(nr == 1 || nr == 2);

    if (nr == 1) {
        if (cop1end + offset >= cop1lc) cop1end += offset;
    } else {
        if (cop2end + offset >= cop2lc) cop2end += offset;
    }
    inspect();
}

char *
Copper::disassemble(u32 addr)
{
    char pos[16];
    char mask[16];
    
    if (isMoveCmd(addr)) {
        
        sprintf(disassembly, "MOVE $%04X, %s", getDW(addr), regName(getRA(addr)));
        return disassembly;
    }
    
    const char *mnemonic = isWaitCmd(addr) ? "WAIT" : "SKIP";
    const char *suffix = getBFD(addr) ? "" : "b";
    
    sprintf(pos, "($%02X,$%02X)", getVP(addr), getHP(addr));
    
    if (getVM(addr) == 0xFF && getHM(addr) == 0xFF) {
        mask[0] = 0;
    } else {
        sprintf(mask, ", ($%02X,$%02X)", getHM(addr), getVM(addr));
    }
    
    sprintf(disassembly, "%s%s %s%s", mnemonic, suffix, pos, mask);
    return disassembly;
}

char *
Copper::disassemble(isize list, isize offset)
{
    assert(list == 1 || list == 2);
    
    u32 addr = (u32)((list == 1 ? cop1lc : cop2lc) + 2 * offset);
    return disassemble(addr);
}

void
Copper::dumpCopperList(isize list, isize length)
{
    for (isize i = 0; i < length; i++) {
        msg("%s\n", disassemble(list, 2*i));
    }
}
