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
#include "Concurrency.h"
#include "RingBuffer.h"

/* About the AudioStream
 *
 * The audio stream is the last element in the audio pipeline. It is a temporary
 * stores for the final audio samples, waiting to be handed over to the audio
 * unit of the host machine.
 *
 * The audio stream is designes as a ring buffer, because samples are written
 * and read asynchroneously. Since reading and writing is carried out in
 * different threads, accesses to the audio stream need to a preceded by a call
 * lock() and followed by a call to unlock().
 *
 * The audio stream is designed to hold elements of a generic type to make
 * vAmiga compilable on different target platforms. E.g., the Mac version holds
 * elements of type FloatStereo, because the audio backend in macOS expects
 * sound samples in form of float values. In the SFML version, the audio stream
 * is instantiated with elements of type U16Stereo, because the frameworks
 * expects audio samples to be provided as an (interleaved) stream of short
 * integers.
 */

//
// Sample types
//

// Integer mono stream
struct U16Mono
{
    i16 lr;
    
    U16Mono() { lr = 0; }
    U16Mono(float l, float r) { this->lr = (i16)(l + r); }
    
    float magnitude(bool left) { return abs(lr); }
    
    void modulate(float vol) { lr = (i16)(lr * vol); }
    
    void copy(void *buffer, isize offset) {
        ((i16 *)buffer)[offset] = lr;
    }
    
    void copy(void *left, void *right, isize offset) {
        ((i16 *)left)[offset] = lr;
        ((i16 *)right)[offset] = lr;
    }
};

// Integer stereo stream
struct U16Stereo
{
    i16 l;
    i16 r;
    
    U16Stereo() { l = 0; r = 0; }
    U16Stereo(float l, float r) { this->l = (i16)l; this->r = (i16)r; }
    
    float magnitude(bool left) { return left ? abs(l) : abs(r); }
    
    void modulate(float vol) { l = (i16)(l * vol); r = (i16)(r * vol); }
    
    void copy(void *buffer, isize offset) {
        ((U16Stereo *)buffer)[offset] = *this;
    }
    
    void copy(void *left, void *right, isize offset) {
        ((i16 *)left)[offset] = l;
        ((i16 *)right)[offset] = r;
    }
};

// Floating-point stereo stream
struct FloatStereo
{
    float l;
    float r;
    
    FloatStereo() { l = 0; r = 0; }
    FloatStereo(float l, float r) { this->l = l * 0.000005; this->r = r * 0.000005; }
    
    float magnitude(bool left) { return left ? abs(l) : abs(r); }
    
    void modulate(float vol) { l *= vol; r *= vol; }
    
    void copy(void *buffer, isize offset) {
        ((FloatStereo *)buffer)[offset] = *this;
    }
    
    void copy(void *left, void *right, isize offset) {
        ((float *)left)[offset] = l;
        ((float *)right)[offset] = r;
    }
};


//
// Volume
//

struct Volume {

    // Maximum volume
    // constexpr const static float maxVolume = 1.0;

    // Current volume (will eventually reach the target volume)
    float current = 1.0;

    // Target volume
    float target = 1.0;

    // Delta steps (added to volume until the target volume is reached)
    float delta = 0;

    bool fading() { return current != target; }
    bool silent() { return current == 0.0; }
    
    // Shifts the current volume towards the target volume
    void shift();
};


//
// AudioStream
//

template <class T> class AudioStream : public util::RingBuffer <T, 16384> {

    // Mutex for synchronizing read / write accesses
    util::Mutex mutex;

public:
    
    // Locks or unlocks the synchronization mutex
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    // Initializes the ring buffer with zeroes
    void wipeOut() { this->clear(T(0,0)); }
    
    // Adds a sample to the ring buffer
    void add(float l, float r) { this->write(T(l,r)); }
        
    // Puts the write pointer somewhat ahead of the read pointer
    void alignWritePtr();
    
    
    //
    // Copying data
    //
    
    /* Copies n audio samples into a memory buffer. These functions mark the
     * final step in the audio pipeline. They are used to copy the generated
     * sound samples into the buffers of the native sound device. In additon
     * to copying, the volume is modulated if the music is supposed to fade
     * in or fade out.
     */
    void copy(void *buffer, isize n, Volume &vol);
    void copy(void *buffer1, void *buffer2, isize n, Volume &vol);
    
    
    //
    // Visualizing the waveform
    //
    
    /* Plots a graphical representation of the waveform. Returns the highest
     * amplitute that was found in the ringbuffer. To implement auto-scaling,
     * pass the returned value as parameter highestAmplitude in the next call
     * to this function.
     */
    float draw(u32 *buffer, isize width, isize height,
               bool left, float highestAmplitude, u32 color);
};
