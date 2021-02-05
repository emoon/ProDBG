// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "EventHandlerPublicTypes.h"
#include "Reflection.h"

#define isPrimarySlot(s) ((s) <= SLOT_SEC)
#define isSecondarySlot(s) ((s) > SLOT_SEC && (s) < SLOT_COUNT)

struct EventSlotEnum : Reflection<EventSlotEnum, EventSlot> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < SLOT_COUNT;
    }
    
    static const char *prefix() { return "SLOT"; }
    static const char *key(EventSlot value)
    {
        switch (value) {
                
            case SLOT_REG:   return "REG";
            case SLOT_RAS:   return "RAS";
            case SLOT_CIAA:  return "CIAA";
            case SLOT_CIAB:  return "CIAB";
            case SLOT_BPL:   return "BPL";
            case SLOT_DAS:   return "DAS";
            case SLOT_COP:   return "COP";
            case SLOT_BLT:   return "BLT";
            case SLOT_SEC:   return "SEC";

            case SLOT_CH0:   return "CH0";
            case SLOT_CH1:   return "CH1";
            case SLOT_CH2:   return "CH2";
            case SLOT_CH3:   return "CH3";
            case SLOT_DSK:   return "DSK";
            case SLOT_DCH:   return "DCH";
            case SLOT_VBL:   return "VBL";
            case SLOT_IRQ:   return "IRQ";
            case SLOT_IPL:   return "IPL";
            case SLOT_KBD:   return "KBD";
            case SLOT_TXD:   return "TXD";
            case SLOT_RXD:   return "RXD";
            case SLOT_POT:   return "POT";
            case SLOT_INS:   return "INS";
            case SLOT_COUNT: return "???";
        }
        return "???";
    }
    /*
    static const char *description(EventSlot value)
    {
        switch (value) {
                
            case SLOT_REG:   return "Registers";
            case SLOT_RAS:   return "Rasterline";
            case SLOT_CIAA:  return "CIA A";
            case SLOT_CIAB:  return "CIA B";
            case SLOT_BPL:   return "Bitplane DMA";
            case SLOT_DAS:   return "Other DMA";
            case SLOT_COP:   return "Copper";
            case SLOT_BLT:   return "Blitter";
            case SLOT_SEC:   return "Secondary";

            case SLOT_CH0:   return "Audio channel 0";
            case SLOT_CH1:   return "Audio channel 1";
            case SLOT_CH2:   return "Audio channel 2";
            case SLOT_CH3:   return "Audio channel 3";
            case SLOT_DSK:   return "Disk Controller";
            case SLOT_DCH:   return "Disk Change";
            case SLOT_VBL:   return "Vertical blank";
            case SLOT_IRQ:   return "Interrupts";
            case SLOT_IPL:   return "IPL";
            case SLOT_KBD:   return "Keyboard";
            case SLOT_TXD:   return "UART out";
            case SLOT_RXD:   return "UART in";
            case SLOT_POT:   return "Potentiometer";
            case SLOT_INS:   return "Inspector";
            case SLOT_COUNT: return "???";
        }
        return "???";
    }
    */
};
