// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AUDIO_FILTER_H
#define _AUDIO_FILTER_H

#include "HardwareComponent.h"

class AudioFilter : public AmigaComponent {
    
    // The currently set filter type
    FilterType type = FILT_BUTTERWORTH;
    
    // Coefficients of the butterworth filter
    double a1, a2, b0, b1, b2;
    
    // The butterworth filter pipeline
    double x1, x2, y1, y2;
    
    
    //
    // Initializing
    //
    
public:
    
    AudioFilter(Amiga& ref);

    void _reset(bool hard) override { RESET_SNAPSHOT_ITEMS(hard) }
    
    
    //
    // Configuring
    //
    
public:
    
    // Filter type
    FilterType getFilterType() { return type; }
    void setFilterType(FilterType type);
    
    // Sample rate
    void setSampleRate(double sampleRate);
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & type;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Using
    //

public:
    
    // Initializes the filter pipeline with zero elements
    void clear();

    // Inserts a sample into the filter pipeline
    float apply(float sample);
};
    
#endif
