// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MuxerTypes.h"

#include "AmigaComponent.h"
#include "AudioStream.h"
#include "AudioFilter.h"
#include "Chrono.h"
#include "Sampler.h"

/* Architecture of the audio pipeline
 *
 *           Mux class
 *           -----------------------------------------------------
 *  State   |   ---------                                         |
 * machine -|->| Sampler |-> vol ->|                              |
 *    0     |   ---------          |                              |
 *          |                      |                              |
 *  State   |   ---------          |                              |
 * machine -|->| Sampler |-> vol ->|                              |
 *    1     |   ---------          |     pan     --------------   |
 *          |                      |--> l vol ->| Audio Stream |--|-> GUI
 *  State   |   ---------          |    r vol    --------------   |
 * machine -|->| Sampler |-> vol ->|                              |
 *    2     |   ---------          |                              |
 *          |                      |                              |
 *  State   |   ---------          |                              |
 * machine -|->| Sampler |-> vol ->|                              |
 *    3     |   ---------                                         |
 *           -----------------------------------------------------
 */

class Muxer : public AmigaComponent {

    // Current configuration
    MuxerConfig config;
    
    // Underflow and overflow counters
    MuxerStats stats;

    // Sample rate in Hz
    double sampleRate = 0;
    
    // Master clock cycles per audio sample
    double cyclesPerSample = 0;

    // Fraction of a sample that hadn't been generated in synthesize
    double fraction;

    // Time stamp of the last write pointer alignment
    util::Time lastAlignment;

    // Volume control
    Volume volume;
            
    // Volume scaling factors
    float vol[4];
    float volL;
    float volR;

    // Panning factors
    float pan[4];
    
    
    //
    // Sub components
    //
    
public:

    // Inputs (one Sampler for each of the four channels)
    Sampler *sampler[4];

    // Output
    AudioStream<SampleType> stream;
    
    // Audio filters
    AudioFilter filterL = AudioFilter(amiga);
    AudioFilter filterR = AudioFilter(amiga);
        
    
    //
    // Initializing
    //
    
public:
    
    Muxer(Amiga& ref);
    ~Muxer();

    const char *getDescription() const override { return "Muxer"; }
    void _reset(bool hard) override;
    
    // Resets the output buffer and the two audio filters
    void clear();
    
    
    //
    // Configuring
    //
    
public:
    
    const MuxerConfig &getConfig() const { return config; }

    long getConfigItem(Option option) const;
    long getConfigItem(Option option, long id) const;
    bool setConfigItem(Option option, long value) override;
    bool setConfigItem(Option option, long id, long value) override;

    bool isMuted() const { return config.volL == 0 && config.volR == 0; }

    double getSampleRate() const { return sampleRate; }
    void setSampleRate(double hz);


    //
    // Analyzing
    //
    
public:
    
    // Returns information about the gathered statistical information
    MuxerStats getStats() const { return stats; }
    
private:
    
    void _dump(Dump::Category category, std::ostream& os) const override;

        
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker
        
        << config.samplingMethod
        << config.filterType
        << config.filterAlwaysOn
        << config.pan
        << config.vol
        << config.volL
        << config.volR
        << pan
        << vol
        << volL
        << volR;
    }
    
    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;
    
    
    //
    // Controlling the volume
    //
    
public:
        
    /* Starts to ramp up the volume. This function configures variables volume
     * and targetVolume to simulate a smooth audio fade in.
     */
    void rampUp();
    void rampUpFromZero();
    
    /* Starts to ramp down the volume. This function configures variables
     * volume and targetVolume to simulate a quick audio fade out.
     */
    void rampDown();
    
    
    //
    // Generating audio streams
    //
    
public:
    
    void synthesize(Cycle clock, Cycle target, long count);
    void synthesize(Cycle clock, Cycle target);

private:

    template <SamplingMethod method>
    void synthesize(Cycle clock, long count, double cyclesPerSample);
    
    // Handles a buffer underflow or overflow condition
    void handleBufferUnderflow();
    void handleBufferOverflow();
    
public:
    
    // Signals to ignore the next underflow or overflow condition
    void ignoreNextUnderOrOverflow();


    //
    // Reading audio samples
    //
    
public:
    
    // Copies a certain amout of audio samples into a buffer
    void copy(void *buffer, isize n);
    void copy(void *buffer1, void *buffer2, isize n);
    
    /* Returns a pointer to a buffer holding a certain amount of audio samples
     * without copying data. This function has been implemented for speedup.
     * Instead of copying ring buffer data into the target buffer, it returns
     * a pointer into the ringbuffer itself. The caller has to make sure that
     * the ring buffer buffer read pointer is not closer than n elements to the
     * buffer end.
     */
    SampleType *nocopy(isize n);
};
