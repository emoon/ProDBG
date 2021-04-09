// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "BusTypes.h"
#include "EventTypes.h"
#include "EventHandlerTypes.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(AGNUS_REVISION)
{
    AGNUS_OCS,              // Revision 8367
    AGNUS_ECS_1MB,          // Revision 8372
    AGNUS_ECS_2MB,          // Revision 8375
    
    AGNUS_COUNT
};
typedef AGNUS_REVISION AgnusRevision;

#ifdef __cplusplus
struct AgnusRevisionEnum : util::Reflection<AgnusRevisionEnum, AgnusRevision> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < AGNUS_COUNT;
    }

    static const char *prefix() { return "AGNUS"; }
    static const char *key(AgnusRevision value)
    {
        switch (value) {
                
            case AGNUS_OCS:     return "OCS";
            case AGNUS_ECS_1MB: return "ECS_1MB";
            case AGNUS_ECS_2MB: return "ECS_2MB";
            case AGNUS_COUNT:   return "???";
        }
        return "???";
    }
};
#endif

enum_long(DDF_STATE)
{
    DDF_OFF,
    DDF_READY,
    DDF_ON
};
typedef DDF_STATE DDFState;

#ifdef __cplusplus
struct DDFStateEnum : util::Reflection<DDFStateEnum, DDFState> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <= DDF_ON;
    }

    static const char *prefix() { return "DDF"; }
    static const char *key(AgnusRevision value)
    {
        switch (value) {
                
            case DDF_OFF:   return "OFF";
            case DDF_READY: return "READY";
            case DDF_ON:    return "ON";
        }
        return "???";
    }
};
#endif

enum_long(SPR_DMA_STATE)
{
    SPR_DMA_IDLE,
    SPR_DMA_ACTIVE
};
typedef SPR_DMA_STATE SprDMAState;

#ifdef __cplusplus
struct SprDmaStateEnum : util::Reflection<SprDmaStateEnum, SprDMAState> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <= SPR_DMA_ACTIVE;
    }

    static const char *prefix() { return "SPR_DMA"; }
    static const char *key(SprDMAState value)
    {
        switch (value) {
                
            case SPR_DMA_IDLE:   return "IDLE";
            case SPR_DMA_ACTIVE: return "ACTIVE";
        }
        return "???";
    }
};
#endif

// Inspection interval in seconds (interval between INS_xxx events)
static const double inspectionInterval = 0.1;

//
// Structures
//

typedef struct
{
    AgnusRevision revision;
    bool slowRamMirror;
}
AgnusConfig;

typedef struct
{
    i16 vpos;
    i16 hpos;

    u16 dmacon;
    u16 bplcon0;
    u8  bpu;
    u16 ddfstrt;
    u16 ddfstop;
    u16 diwstrt;
    u16 diwstop;

    u16 bpl1mod;
    u16 bpl2mod;
    u16 bltamod;
    u16 bltbmod;
    u16 bltcmod;
    u16 bltdmod;
    u16 bltcon0;
    
    u32 coppc;
    u32 dskpt;
    u32 bplpt[6];
    u32 audpt[4];
    u32 audlc[4];
    u32 bltpt[4];
    u32 sprpt[8];

    bool bls;
}
AgnusInfo;

typedef struct
{
    long usage[BUS_COUNT];
    
    double copperActivity;
    double blitterActivity;
    double diskActivity;
    double audioActivity;
    double spriteActivity;
    double bitplaneActivity;
}
AgnusStats;

typedef struct
{
    EventSlot slot;
    EventID eventId;
    const char *eventName;

    // Trigger cycle of the event
    Cycle trigger;
    Cycle triggerRel;

    // Trigger relative to the current frame
    // -1 = earlier frame, 0 = current frame, 1 = later frame
    long frameRel;

    // The trigger cycle translated to a beam position.
    long vpos;
    long hpos;
}
EventSlotInfo;

typedef struct
{
    Cycle cpuClock;
    Cycle cpuCycles;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    long frame;
    long vpos;
    long hpos;

    EventSlotInfo slotInfo[SLOT_COUNT];
}
EventInfo;
