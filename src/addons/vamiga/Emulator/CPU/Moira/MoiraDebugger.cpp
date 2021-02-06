// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Moira.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

namespace moira {

//
// Guard
//

bool
Guard::eval(u32 addr, Size S)
{
    if (this->addr >= addr && this->addr < addr + S && this->enabled) {
        if (++hits > skip) {
            return true;
        }
    }
    return false;
}


//
// Guards
//

Guard *
Guards::guardWithNr(long nr)
{
    return nr < count ? &guards[nr] : NULL;
}

Guard *
Guards::guardAtAddr(u32 addr)
{
    for (int i = 0; i < count; i++) {
        if (guards[i].addr == addr) return &guards[i];
    }

    return NULL;
}

bool
Guards::isSetAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL;
}

bool
Guards::isSetAndEnabledAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL && guard->enabled;
}

bool
Guards::isSetAndDisabledAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL && !guard->enabled;
}

bool
Guards::isSetAndConditionalAt(u32 addr)
{
    Guard *guard = guardAtAddr(addr);

    return guard != NULL && guard->skip != 0;
}

void
Guards::addAt(u32 addr, long skip)
{
    if (isSetAt(addr)) return;

    if (count >= capacity) {

        Guard *newguards = new Guard[2 * capacity];
        for (long i = 0; i < capacity; i++) newguards[i] = guards[i];
        delete [] guards;
        guards = newguards;
        capacity *= 2;
    }

    guards[count].addr = addr;
    guards[count].enabled = true;
    guards[count].hits = 0;
    guards[count].skip = skip;
    count++;
    setNeedsCheck(true);
}

void
Guards::remove(long nr)
{
    if (nr < count) removeAt(guards[nr].addr);
}

void
Guards::removeAt(u32 addr)
{
    for (int i = 0; i < count; i++) {

        if (guards[i].addr == addr) {

            for (int j = i; j + 1 < count; j++) guards[j] = guards[j + 1];
            count--;
            break;
        }
    }
    setNeedsCheck(count != 0);
}

void
Guards::replace(long nr, u32 addr)
{
    if (nr >= count || isSetAt(addr)) return;
    
    guards[nr].addr = addr;
    guards[nr].hits = 0;
}

bool
Guards::isEnabled(long nr)
{
    return nr < count ? guards[nr].enabled : false;
}

void
Guards::setEnable(long nr, bool val)
{
    if (nr < count) guards[nr].enabled = val;
}

void
Guards::setEnableAt(u32 addr, bool value)
{
    Guard *guard = guardAtAddr(addr);
    if (guard) guard->enabled = value;
}

bool
Guards::eval(u32 addr, Size S)
{
    for (int i = 0; i < count; i++)
        if (guards[i].eval(addr, S)) return true;

    return false;
}

void
Breakpoints::setNeedsCheck(bool value)
{
    if (value) {
        moira.flags |= Moira::CPU_CHECK_BP;
    } else {
        moira.flags &= ~Moira::CPU_CHECK_BP;
    }
}

void
Watchpoints::setNeedsCheck(bool value)
{
    if (value) {
        moira.flags |= Moira::CPU_CHECK_WP;
    } else {
        moira.flags &= ~Moira::CPU_CHECK_WP;
    }
}

void
Debugger::reset()
{
    breakpoints.setNeedsCheck(breakpoints.elements() != 0);
    watchpoints.setNeedsCheck(watchpoints.elements() != 0);
}

void
Debugger::stepInto()
{
    softStop = UINT64_MAX;
    breakpoints.setNeedsCheck(true);
}

void
Debugger::stepOver()
{
    char tmp[64];
    softStop = moira.getPC() + moira.disassemble(moira.getPC(), tmp);
    breakpoints.setNeedsCheck(true);
}

bool
Debugger::breakpointMatches(u32 addr)
{
    // Check if a soft breakpoint has been reached
    if (addr == softStop || softStop == UINT64_MAX) {

        // Soft breakpoints are deleted when reached
        softStop = UINT64_MAX - 1;
        breakpoints.setNeedsCheck(breakpoints.elements() != 0);

        return true;
    }

    return breakpoints.eval(addr);
}

bool
Debugger::watchpointMatches(u32 addr, Size S)
{
    return watchpoints.eval(addr, S);
}

void
Debugger::enableLogging()
{
    moira.flags |= Moira::CPU_LOG_INSTRUCTION;
}

void
Debugger::disableLogging()
{
    moira.flags &= ~Moira::CPU_LOG_INSTRUCTION;
}

int
Debugger::loggedInstructions()
{
    return logCnt < logBufferCapacity ? (int)logCnt : logBufferCapacity;
}

void
Debugger::logInstruction()
{
    logBuffer[logCnt % logBufferCapacity] = moira.reg;
    logCnt++;
}

Registers &
Debugger::logEntryRel(int n)
{
    assert(n < loggedInstructions());
    return logBuffer[(logCnt - 1 - n) % logBufferCapacity];
}

Registers &
Debugger::logEntryAbs(int n)
{
    assert(n < loggedInstructions());
    return logEntryRel(loggedInstructions() - n - 1);
}

/*
u32
Debugger::loggedPC0Rel(int n)
{
    assert(n < loggedInstructions());
    return logBuffer[(logCnt - 1 - n) % logBufferCapacity].pc0;
}

u32
Debugger::loggedPC0Abs(int n)
{
    assert(n < loggedInstructions());
    return loggedPC0Rel(loggedInstructions() - n - 1);
}
*/

}
