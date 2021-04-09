// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AudioStream.h"
#include <algorithm>

void
Volume::shift()
{
    if (current < target) {
        current += std::min(delta, target - current);
    } else {
        current -= std::min(delta, current - target);
    }
}

template <class T> void
AudioStream<T>::alignWritePtr()
{
    this->align(8192);
}

template <class T> void
AudioStream<T>::copy(void *buffer, isize n, Volume &vol)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(this->count() >= n);

    // Quick path: Volume is stable at 0 or 1
    if (!vol.fading()) {

        if (vol.current == 0) {

            for (isize i = 0; i < n; i++) {
                T zero;
                for (isize i = 0; i < n; i++) {
                    zero.copy(buffer, i);
                }
            }
            return;
        }
        if (vol.current == 1.0) {

            for (isize i = 0; i < n; i++) {
                T sample = this->read();
                sample.copy(buffer, i);
            }
            return;
        }
    }
    
    // Generic path: Modulate the volume
    for (isize i = 0; i < n; i++) {
        vol.shift();
        T sample = this->read();
        sample.modulate(vol.current);
        sample.copy(buffer, i);
    }
}

template <class T> void
AudioStream<T>::copy(void *buffer1, void *buffer2, isize n, Volume &vol)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(this->count() >= n);

    // Quick path: Volume is stable at 0 or 1
    if (!vol.fading()) {

        if (vol.current == 0) {

            T zero;
            for (isize i = 0; i < n; i++) {
                zero.copy(buffer1, buffer2, i);
            }
            return;
        }
        if (vol.current == 1.0) {

            for (isize i = 0; i < n; i++) {
                T sample = this->read();
                sample.copy(buffer1, buffer2, i);
            }
            return;
        }
    }
    
    // Generic path: Modulate the volume
    for (isize i = 0; i < n; i++) {
        vol.shift();
        T sample = this->read();
        sample.modulate(vol.current);
        sample.copy(buffer1, buffer2, i);
    }
}

template <class T> float
AudioStream<T>::draw(u32 *buffer, isize width, isize height,
                     bool left, float highestAmplitude, u32 color)
{
    isize dw = this->cap() / width;
    float newHighestAmplitude = 0.001;
    
    // Clear buffer
    for (isize i = 0; i < width * height; i++) {
        buffer[i] = color & 0xFFFFFF;
    }
    
    // Draw waveform
    for (isize w = 0; w < width; w++) {
        
        // Read samples from ringbuffer
        T pair = this->current(w * dw);
        float sample = pair.magnitude(left);
        
        if (sample == 0) {
            
            // Draw some noise to make it look sexy
            unsigned *ptr = buffer + width * height / 2 + w;
            *ptr = color;
            if (rand() % 2) *(ptr + width) = color;
            if (rand() % 2) *(ptr - width) = color;
            
        } else {
            
            // Remember the highest amplitude
            if (sample > newHighestAmplitude) newHighestAmplitude = sample;
            
            // Scale the sample
            isize scaled = isize(sample * height / highestAmplitude);
            if (scaled > height) scaled = height;
            
            // Draw vertical line
            u32 *ptr = buffer + width * ((height - scaled) / 2) + w;
            for (isize j = 0; j < scaled; j++, ptr += width) *ptr = color;
        }
    }
    return newHighestAmplitude;
}

//
// Instantiate template functions
//

template void AudioStream<SampleType>::alignWritePtr();
template void AudioStream<SampleType>::copy(void *, isize, Volume &);
template void AudioStream<SampleType>::copy(void *, void *, isize, Volume &);
template float AudioStream<SampleType>::draw(u32 *, isize, isize, bool, float, u32);
