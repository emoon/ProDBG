// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AUDIO_STREAM_H
#define _AUDIO_STREAM_H

#include "HardwareComponent.h"

typedef struct
{
    float left;
    float right;
    
    template <class T>
    void applyToItems(T& worker)
    {
        worker

        & left
        & right;
    }

}
SamplePair;

class AudioStream : public RingBuffer <SamplePair, 16384> {

public:
    
    //
    // Initializing
    //
    
    /* Aligns the write pointer. This function puts the write pointer somewhat
     * ahead of the read pointer. With a standard sample rate of 44100 Hz,
     * 735 samples is 1/60 sec.
     */
    static u32 samplesAhead() { return 8 * 735; }
    void alignWritePtr() { align(samplesAhead()); }
    
    
    //
    // Copying data
    //
    
    /* Copies n audio samples into a memory buffer. These functions mark the
     * final step in the audio pipeline. They are used to copy the generated
     * sound samples into the buffers of the native sound device. In additon
     * to copying, the volume is modulated and audio filters can be applied.
     */
    void copyMono(float *buffer, size_t n,
                  i32 &volume, i32 targetVolume, i32 volumeDelta);
    
    void copy(float *left, float *right, size_t n,
                    i32 &volume, i32 targetVolume, i32 volumeDelta);
    
    void copyInterleaved(float *buffer, size_t n,
                         i32 &volume, i32 targetVolume, i32 volumeDelta);
    
    
    //
    // Visualizing the waveform
    //
    
    /* Plots a graphical representation of the waveform. Returns the highest
     * amplitute that was found in the ringbuffer. To implement auto-scaling,
     * pass the returned value as parameter highestAmplitude in the next call
     * to this function.
     */
    float draw(unsigned *buffer, int width, int height,
               bool left, float highestAmplitude, unsigned color);
};

#endif
