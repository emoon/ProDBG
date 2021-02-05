// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
AudioStream::copyMono(float *buffer, isize n,
                      i32 &volume, i32 targetVolume, i32 volumeDelta)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);

    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (isize i = 0; i < n; i++) {
            
            SamplePair pair = read();
            *buffer++ = (pair.left + pair.right) * scale;
        }

    } else {
        
        for (isize i = 0; i < n; i++) {
                            
            if (volume < targetVolume) {
                volume += MIN(volumeDelta, targetVolume - volume);
            } else {
                volume -= MIN(volumeDelta, volume - targetVolume);
            }

            float scale = volume / 10000.0f;

            SamplePair pair = read();
            *buffer++ = (pair.left + pair.right) * scale;
        }
    }
}

void
AudioStream::copy(float *left, float *right, isize n,
                  i32 &volume, i32 targetVolume, i32 volumeDelta)
{
    // The caller has to ensure that no buffer underflows occurs
    if (count() < n) {
        printf("count() = \(count()) n = \(n)");
        assert(false);
    }
    assert(count() >= n);

    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (isize i = 0; i < n; i++) {
            
            SamplePair pair = read();
            *left++ = pair.left * scale;
            *right++ = pair.right * scale;
        }

    } else {
        
        for (isize i = 0; i < n; i++) {
                            
            if (volume < targetVolume) {
                volume += MIN(volumeDelta, targetVolume - volume);
            } else {
                volume -= MIN(volumeDelta, volume - targetVolume);
            }

            float scale = volume / 10000.0f;

            SamplePair pair = read();
            *left++ = pair.left * scale;
            *right++ = pair.right * scale;
        }
    }
}

void
AudioStream::copyInterleaved(float *buffer, isize n,
                             i32 &volume, i32 targetVolume, i32 volumeDelta)
{
    // The caller has to ensure that no buffer underflows occurs
    assert(count() >= n);

    if (volume == targetVolume) {
        
        float scale = volume / 10000.0f;
        
        for (isize i = 0; i < n; i++) {
            
            SamplePair pair = read();
            *buffer++ = pair.left * scale;
            *buffer++ = pair.right * scale;
        }

    } else {
        
        for (isize i = 0; i < n; i++) {
                            
            if (volume < targetVolume) {
                volume += MIN(volumeDelta, targetVolume - volume);
            } else {
                volume -= MIN(volumeDelta, volume - targetVolume);
            }

            float scale = volume / 10000.0f;

            SamplePair pair = read();
            *buffer++ = pair.left * scale;
            *buffer++ = pair.right * scale;
        }
    }
}

float
AudioStream::draw(u32 *buffer, isize width, isize height,
                  bool left, float highestAmplitude, u32 color)
{
    isize dw = cap() / width;
    float newHighestAmplitude = 0.001;
    
    // Clear buffer
    for (isize i = 0; i < width * height; i++) {
        buffer[i] = color & 0xFFFFFF;
    }
    
    // Draw waveform
    for (isize w = 0; w < width; w++) {
        
        // Read samples from ringbuffer
        SamplePair pair = current(w * dw);
        float sample = left ? abs(pair.left) : abs(pair.right);
        
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
