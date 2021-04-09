// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

namespace moira {

// Base structure for a single breakpoint or watchpoint
struct Guard {

    // The observed address
    u32 addr;

    // Disabled guards never trigger
    bool enabled;

    // Counts the number of hits
    long hits;

    // Number of skipped hits before a match is signalled
    long skip;

public:

    // Returns true if the guard hits
    bool eval(u32 addr, Size S = Byte);

};

// Base class for a collection of guards
class Guards {

    friend class Debugger;

public:
    
    virtual ~Guards() { };
    
protected:

    // Reference to the connected CPU
    class Moira &moira;

    // Capacity of the guards array
    long capacity = 1;

    // Array holding all guards
    Guard *guards = new Guard[1];

    // Number of currently stored guards
    long count = 0;

    // Indicates if guard checking is necessary
    virtual void setNeedsCheck(bool value) = 0;


    //
    // Constructing
    //

public:

    Guards(Moira& ref) : moira(ref) { }

    //
    // Inspecting the guard list
    //

    long elements() { return count; }
    Guard *guardWithNr(long nr);
    Guard *guardAtAddr(u32 addr);

    u32 guardAddr(long nr) { return nr < count ? guards[nr].addr : 0; }

    bool isSetAt(u32 addr);
    bool isSetAndEnabledAt(u32 addr);
    bool isSetAndDisabledAt(u32 addr);
    bool isSetAndConditionalAt(u32 addr);

    //
    // Adding or removing guards
    //

    void addAt(u32 addr, long skip = 0);
    void removeAt(u32 addr);

    void remove(long nr);
    void removeAll() { count = 0; setNeedsCheck(false); }

    void replace(long nr, u32 addr);

    //
    // Enabling or disabling guards
    //

    bool isEnabled(long nr);
    bool isDisabled(long nr) { return !isEnabled(nr); }

    void setEnable(long nr, bool val);
    void enable(long nr) { setEnable(nr, true); }
    void disable(long nr) { setEnable(nr, false); }

    void setEnableAt(u32 addr, bool val);
    void enableAt(u32 addr) { setEnableAt(addr, true); }
    void disableAt(u32 addr) { setEnableAt(addr, false); }

    //
    // Checking a guard
    //

private:

    bool eval(u32 addr, Size S = Byte);
};

class Breakpoints : public Guards {

public:

    Breakpoints(Moira& ref) : Guards(ref) { }
    void setNeedsCheck(bool value) override;
};

class Watchpoints : public Guards {

public:

    Watchpoints(Moira& ref) : Guards(ref) { }
    void setNeedsCheck(bool value) override;
};

class Debugger {

public:

    // Reference to the connected CPU
    class Moira &moira;

    // Breakpoint storage
    Breakpoints breakpoints = Breakpoints(moira);

    // Watchpoint storage
    Watchpoints watchpoints = Watchpoints(moira);

private:

    /* Soft breakpoint for implementing single-stepping.
     * In contrast to a standard (hard) breakpoint, a soft breakpoint is
     * deleted when reached. The CPU halts if softStop matches the CPU's
     * program counter (used to implement "step over") or if softStop equals
     * UINT64_MAX (used to implement "step into"). To disable soft stopping,
     * simply set softStop to an unreachable memory location such as
     * UINT64_MAX - 1.
     */
    u64 softStop = UINT64_MAX - 1;

    // Buffer storing logged instructions
    static const int logBufferCapacity = 256;
    Registers logBuffer[logBufferCapacity];

    // Logging counter
    long logCnt = 0;


    //
    // Constructing
    //

public:

    Debugger(Moira& ref) : moira(ref) { }

    void reset();

    //
    // Working with breakpoints and watchpoints
    //

    // Sets a soft breakpoint that will trigger immediately
    void stepInto();

    // Sets a soft breakpoint to the next instruction
    void stepOver();

    // Returns true if a breakpoint hits at the provides address
    bool breakpointMatches(u32 addr);

    // Returns true if a watchpoint hits at the provides address
    bool watchpointMatches(u32 addr, Size S);

    //
    // Working with the log buffer
    //

    // Turns instruction logging on or off
    void enableLogging();
    void disableLogging();

    // Returns the number of logged instructions
    int loggedInstructions();

    // Logs an instruction
    void logInstruction();
    
    /* Reads an item from the log buffer
     *
     *    xxxRel: n == 0 returns the most recently recorded entry
     *    xxxAbs: n == 0 returns the oldest entry
     */
    Registers &logEntryRel(int n);
    Registers &logEntryAbs(int n);
    // u32 loggedPC0Rel(int n);
    // u32 loggedPC0Abs(int n);
    
    // Clears the log buffer
    void clearLog() { logCnt = 0; }
    
    
};

}
