// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

//
// Release settings
//

// Snapshot version number
#define V_MAJOR 0
#define V_MINOR 9
#define V_SUBMINOR 17

// Uncomment these settings in a release build
// #define RELEASEBUILD
// #define NDEBUG

//
// Configuration overrides
//

// Uncomment to override a configuration setting

// #define FORCE_AGNUS_REVISION AGNUS_OCS
// #define FORCE_BLT_LEVEL      0
// #define FORCE_CHIP_RAM       512
// #define FORCE_SLOW_RAM       512
// #define FORCE_FAST_RAM       0
// #define FORCE_RTC            RTC_NONE
// #define FORCE_DRIVE_SPEED    -1
// #define FORCE_RTC_REGISTER   0

// Uncomment to set a breakpoint on startup
// #define INITIAL_BREAKPOINT 0xFC1354

// Uncomment to fallback to a simpler Agnus execution function
// #define AGNUS_EXEC_DEBUG

// Uncomment to lauch the emulator with a disk in df0
// #define DF0_DISK "/Users/hoff/Desktop/Testing/Planet_Rocklobster_Oxyron.adf"
// #define DF0_DISK "/Users/hoff/Desktop/Testing/Ruffntumble.adf"
// #define DF1_DISK "/Users/hoff/Dropbox/Amiga/DOSDisks/Disk1.img"
// #define DF0_DISK "/Users/hoff/Desktop/Testing/JetsonsCracked.adf"
// #define DF0_DISK "/Users/hoff/Desktop/Testing/A2000WB1.2D.adf"
// #define DF0_DISK "/Users/hoff/Downloads/Jetsons1.adf"

// Uncomment to colorize certain rasterlines
// #define LINE_DEBUG (agnus.pos.v == 260 || agnus.pos.v == 300)


//
// Compatibility settings
//



//
// Debug settings
//

// General
static const int CNF_DEBUG       = 0; // Configuration options
static const int XFILES          = 0; // Report paranormal activity
static const int MIMIC_UAE       = 0; // Enable to compare debug logs with UAE
static const int NO_SSE          = 0; // Don't use SSE extensions

// Runloop
static const int RUN_DEBUG       = 0; // Run loop, component states
static const int QUEUE_DEBUG     = 0; // Message queue
static const int SNP_DEBUG       = 0; // Serialization (snapshots)

// CPU
static const int CPU_DEBUG       = 0; // CPU

// Memory access
static const int OCSREG_DEBUG    = 0; // General OCS register debugging
static const int ECSREG_DEBUG    = 0; // Special ECS register debugging
static const int INVREG_DEBUG    = 0; // Invalid register accesses
static const int MEM_DEBUG       = 0; // Memory 
static const int FAS_DEBUG       = 0; // Fast RAM

// Agnus
static const int DMA_DEBUG       = 0; // DMA registers
static const int DDF_DEBUG       = 0; // Display data fetch
static const int NO_PTR_DROPS    = 0; // Never drop a pointer register write

// Copper
static const int COP_CHECKSUM    = 0; // Compute Copper checksums
static const int COPREG_DEBUG    = 0; // Copper registers
static const int COP_DEBUG       = 0; // Copper execution

// Blitter
static const int BLT_CHECKSUM    = 0; // Compute Blitter checksums
static const int BLTREG_DEBUG    = 0; // Blitter registers
static const int BLT_GUARD       = 0; // Guard registers while Blitter runs
static const int BLT_DEBUG       = 0; // Blitter execution
static const int BLTTIM_DEBUG    = 0; // Blitter Timing
static const int SLOW_BLT_DEBUG  = 0; // Execute micro-instructions in one chunk

// Denise
static const int BPLREG_DEBUG    = 0; // Bitplane registers
static const int BPLDAT_DEBUG    = 0; // BPLxDAT registers
static const int SPRREG_DEBUG    = 0; // Sprite registers
static const int COLREG_DEBUG    = 0; // Color registers
static const int CLXREG_DEBUG    = 0; // Collision detection registers
static const int BPL_DEBUG       = 0; // Bitplane DMA
static const int DIW_DEBUG       = 0; // Display window
static const int SPR_DEBUG       = 0; // Sprites
static const int CLX_DEBUG       = 0; // Collision detection
static const int BORDER_DEBUG    = 0; // Draw the border in debug colors

// Paula
static const int INTREG_DEBUG    = 0; // Interrupt registers
static const int INT_DEBUG       = 0; // Interrupt logic

// CIAs
static const int CIA_ON_STEROIDS = 0; // Keep the CIAs awake all the time
static const int CIAREG_DEBUG    = 0; // CIA registers
static const int CIASER_DEBUG    = 0; // CIA serial register
static const int CIAA_DEBUG      = 0; // CIA A execution
static const int CIAB_DEBUG      = 0; // CIA B execution
static const int TOD_DEBUG       = 0; // TODs (CIA 24-bit counters)

// Drive
static const int ALIGN_HEAD      = 0; // Make head movement deterministic
static const int DSK_CHECKSUM    = 0; // Compute disk checksums
static const int DSKREG_DEBUG    = 0; // Disk controller registers
static const int DSK_DEBUG       = 0; // Disk controller execution
static const int MFM_DEBUG       = 0; // Disk encoder / decoder
static const int FS_DEBUG        = 0; // File System Classes (OFS / FFS)

// Audio
static const int AUDREG_DEBUG    = 0; // Audio registers
static const int AUD_DEBUG       = 0; // Audio execution
static const int AUDBUF_DEBUG    = 0; // Audio buffers
static const int DISABLE_AUDIRQ  = 0; // Disable audio interrupts

// Ports
static const int POSREG_DEBUG    = 0; // POSxxx registers
static const int JOYREG_DEBUG    = 0; // JOYxxx registers
static const int POTREG_DEBUG    = 0; // POTxxx registers
static const int PORT_DEBUG      = 0; // Control ports and connected devices
static const int SER_DEBUG       = 0; // Serial interface
static const int POT_DEBUG       = 0; // Potentiometer inputs
static const int HOLD_MOUSE_L    = 0; // Hold down the left mouse button
static const int HOLD_MOUSE_R    = 0; // Hold down the right mouse button

// Other components
static const int RTC_DEBUG       = 0; // Real-time clock
static const int KBD_DEBUG       = 0; // Keyboard
static const int REC_DEBUG       = 0; // Screen recorder
