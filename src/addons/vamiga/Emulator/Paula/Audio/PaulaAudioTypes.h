// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _PAULA_AUDIO_TYPES_H
#define _PAULA_AUDIO_TYPES_H

//
// Enumerations
//

VAMIGA_ENUM(long, SamplingMethod)
{
    SMP_NONE,
    SMP_NEAREST,
    SMP_LINEAR,
    SMP_COUNT
};

static inline bool isSamplingMethod(long value)
{
    return value >= 0 && value < SMP_COUNT;
}

static inline const char *sSamplingMethod(SamplingMethod value)
{
    switch (value) {
        case SMP_NONE:     return "SMP_NONE";
        case SMP_NEAREST:  return "SMP_NEAREST";
        case SMP_LINEAR:   return "SMP_LINEAR";
        default:           return "???";
    }
}

VAMIGA_ENUM(long, FilterType)
{
    FILT_NONE,
    FILT_BUTTERWORTH,
    FILT_COUNT
};

static inline bool isFilterType(long value)
{
    return value >= 0 && value < FILT_COUNT;
}

static inline const char *sFilterType(FilterType value)
{
    switch (value) {
        case FILT_NONE:         return "FILT_NONE";
        case FILT_BUTTERWORTH:  return "FILT_BUTTERWORTH";
        default:                return "???";
    }
}


//
// Structures
//

typedef struct
{
    i8 state;
    bool dma;
    u16 audlenLatch;
    u16 audlen;
    u16 audperLatch;
    i32 audper;
    u16 audvolLatch;
    u16 audvol;
    u16 auddat;
}
AudioChannelInfo;

typedef struct
{
    AudioChannelInfo channel[4];
}
AudioInfo;

typedef struct
{
    // Selects how the audio buffer is resampled to match the target frequency
    SamplingMethod samplingMethod;

    // The selected audio filter
    FilterType filterType;

    // If set to true, the Amiga can't deactivate the filter
    bool filterAlwaysOn;

    // Input channel volumes and pan settings
    double vol[4];
    double pan[4];
    
    // Output channel volumes
    double volL;
    double volR;
}
MuxerConfig;

typedef struct
{
    long bufferUnderflows;
    long bufferOverflows;
}
MuxerStats;

#endif
