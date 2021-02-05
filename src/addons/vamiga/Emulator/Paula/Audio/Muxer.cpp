// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Muxer::Muxer(Amiga& ref) : AmigaComponent(ref)
{    
    subComponents = vector<HardwareComponent *> {

        &filterL,
        &filterR
    };
    
    sampler[0] = new Sampler();
    sampler[1] = new Sampler();
    sampler[2] = new Sampler();
    sampler[3] = new Sampler();

    setSampleRate(44100);
}
 
Muxer::~Muxer()
{
    delete sampler[0];
    delete sampler[1];
    delete sampler[2];
    delete sampler[3];
}

void
Muxer::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    stats.bufferUnderflows = 0;
    stats.bufferOverflows = 0;

    for (isize i = 0; i < 4; i++) sampler[i]->reset();
    stream.clear();
}

void
Muxer::clear()
{
    trace(AUDBUF_DEBUG, "clear()\n");
    
    // Wipe out the ringbuffer
    stream.clear(SamplePair {0, 0});
    stream.alignWritePtr();
    
    // Wipe out the filter buffers
    filterL.clear();
    filterR.clear();
}

long
Muxer::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            return config.samplingMethod;
            
        case OPT_FILTER_TYPE:
            assert(filterL.getFilterType() == config.filterType);
            assert(filterR.getFilterType() == config.filterType);
            return config.filterType;
                        
        case OPT_FILTER_ALWAYS_ON:
            return config.filterAlwaysOn;

        case OPT_AUDVOLL:
            return config.volL;

        case OPT_AUDVOLR:
            return config.volR;

        default:
            assert(false);
            return 0;
    }
}

long
Muxer::getConfigItem(Option option, long id) const
{
    switch (option) {
            
        case OPT_AUDVOL:
            return config.vol[id];

        case OPT_AUDPAN:
            return config.pan[id];
            
        default:
            assert(false);
            return 0;
    }
}

bool
Muxer::setConfigItem(Option option, long value)
{
    bool wasMuted = isMuted();
    
    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            
            if (!SamplingMethodEnum::verify(value)) return false;

            if (config.samplingMethod == value) {
                return false;
            }
            
            config.samplingMethod = (SamplingMethod)value;
            return true;
            
        case OPT_FILTER_TYPE:
            
            if (!FilterTypeEnum::verify(value)) return false;

            if (config.filterType == value) {
                return false;
            }

            config.filterType = (FilterType)value;
            filterL.setFilterType((FilterType)value);
            filterR.setFilterType((FilterType)value);
            return true;
                        
        case OPT_FILTER_ALWAYS_ON:
            
            if (config.filterAlwaysOn == value) {
                return false;
            }
            
            config.filterAlwaysOn = value;
            return true;

        case OPT_AUDVOLL:
            
            if (value < 0) value = 0;
            if (value > 100) value = 100;

            config.volL = value;
            volL = pow((double)value / 50, 1.4);
                        
            if (wasMuted != isMuted())
                messageQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return true;
            
        case OPT_AUDVOLR:

            if (value < 0) value = 0;
            if (value > 100) value = 100;

            config.volR = value;
            volR = pow((double)value / 50, 1.4);

            if (wasMuted != isMuted())
                messageQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return true;
            
        default:
            return false;
    }
}

bool
Muxer::setConfigItem(Option option, long id, long value)
{    
    switch (option) {
                        
        case OPT_AUDVOL:
    
            assert(id >= 0 && id <= 3);
            if (value < 0) value = 0;
            if (value > 100) value = 100;
            
            config.vol[id] = value;
            vol[id] = pow((double)value / 100, 1.4) * 0.0000025;
            
            return true;
            
        case OPT_AUDPAN:
                        
            assert(id >= 0 && id <= 3);
            if (value < 0 || value > 200) {
                warn(" Invalid pan: %ld\n", value);
                warn("Valid values: 0 ... 200\n");
                return false;
            }

            config.pan[id] = value;
            
            if (value <= 50) pan[id] = (50 + value) / 100.0;
            else if (value <= 150) pan[id] = (150 - value) / 100.0;
            else if (value <= 200) pan[id] = (value - 150) / 100.0;
            return true;

        default:
            return false;
    }
}

void
Muxer::_dumpConfig() const
{
    msg("samplingMethod : %s\n", SamplingMethodEnum::key(config.samplingMethod));
    msg("    filtertype : %s\n", FilterTypeEnum::key(config.filterType));
    msg("filterAlwaysOn : %d\n", config.filterAlwaysOn);
    msg("    vol0, pan0 : %lld, %lld\n", config.vol[0], config.pan[0]);
    msg("    vol1, pan1 : %lld, %lld\n", config.vol[1], config.pan[1]);
    msg("    vol2, pan2 : %lld, %lld\n", config.vol[2], config.pan[2]);
    msg("    vol3, pan3 : %lld, %lld\n", config.vol[3], config.pan[3]);
    msg("    volL, volR : %lld, %lld\n", config.volL, config.volR);
}

void
Muxer::setSampleRate(double hz)
{
    trace(AUD_DEBUG, "setSampleRate(%f)\n", hz);

    sampleRate = hz;
    cyclesPerSample = MHz(masterClockFrequency) / hz;

    filterL.setSampleRate(hz);
    filterR.setSampleRate(hz);
}

isize
Muxer::didLoadFromBuffer(const u8 *buffer)
{
    for (isize i = 0; i < 4; i++) sampler[i]->reset();
    return 0;
}

void
Muxer::rampUp()
{
    // Only proceed if the emulator is not running in warp mode
    if (warpMode) return;
    
    volume.target = Volume::maxVolume;
    volume.delta = 3;
    ignoreNextUnderOrOverflow();
}

void
Muxer::rampUpFromZero()
{
    volume.current = 0;
    rampUp();
}
 
void
Muxer::rampDown()
{
    volume.target = 0;
    volume.delta = 50;
    ignoreNextUnderOrOverflow();
}

void
Muxer::synthesize(Cycle clock, Cycle target, long count)
{
    assert(target > clock);
    assert(count > 0);

    // Determine the number of elapsed cycles per audio sample
    double cyclesPerSample = (double)(target - clock) / (double)count;
                
    switch (config.samplingMethod) {
            
        case SMP_NONE:    synthesize<SMP_NONE>   (clock, count, cyclesPerSample); break;
        case SMP_NEAREST: synthesize<SMP_NEAREST>(clock, count, cyclesPerSample); break;
        case SMP_LINEAR:  synthesize<SMP_LINEAR> (clock, count, cyclesPerSample); break;
        default:          assert(false);
    }
}

void
Muxer::synthesize(Cycle clock, Cycle target)
{
    assert(target > clock);
    assert(cyclesPerSample > 0);
    
    // Determine how many samples we need to produce
    double exact = (double)(target - clock) / cyclesPerSample + fraction;
    long count = (long)exact;
    fraction = exact - (double)count;
             
    switch (config.samplingMethod) {
        case SMP_NONE:    synthesize<SMP_NONE>   (clock, count, cyclesPerSample); break;
        case SMP_NEAREST: synthesize<SMP_NEAREST>(clock, count, cyclesPerSample); break;
        case SMP_LINEAR:  synthesize<SMP_LINEAR> (clock, count, cyclesPerSample); break;
        default:          assert(false);

    }
}

template <SamplingMethod method> void
Muxer::synthesize(Cycle clock, long count, double cyclesPerSample)
{
    assert(count > 0);

    stream.lock();
    
    // Check for a buffer overflow
    if (stream.count() + count >= stream.cap()) handleBufferOverflow();

    double cycle = clock;
    bool filter = ciaa.powerLED() || config.filterAlwaysOn;

    for (long i = 0; i < count; i++) {

        double ch0 = sampler[0]->interpolate<method>((Cycle)cycle) * vol[0];
        double ch1 = sampler[1]->interpolate<method>((Cycle)cycle) * vol[1];
        double ch2 = sampler[2]->interpolate<method>((Cycle)cycle) * vol[2];
        double ch3 = sampler[3]->interpolate<method>((Cycle)cycle) * vol[3];
        
        // Compute left channel output
        float l =
        ch0 * (1 - pan[0]) + ch1 * (1 - pan[1]) +
        ch2 * (1 - pan[2]) + ch3 * (1 - pan[3]);

        // Compute right channel output
        float r =
        ch0 * pan[0] + ch1 * pan[1] +
        ch2 * pan[2] + ch3 * pan[3];

        // Apply audio filter
        if (filter) { l = filterL.apply(l); r = filterR.apply(r); }
        
        // Apply master volume
        l *= volL;
        r *= volR;
        
        // Write sample into ringbuffer
        stream.write( SamplePair { l, r } );
        
        cycle += cyclesPerSample;
    }
    
    stream.unlock();
}

void
Muxer::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer
    // (2) The producer is halted or not startet yet
    
    trace(AUDBUF_DEBUG, "UNDERFLOW (r: %lld w: %lld)\n", stream.r, stream.w);
    
    // Reset the write pointer
    stream.alignWritePtr();

    // Determine the elapsed seconds since the last pointer adjustment
    u64 now = Oscillator::nanos();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime > 10.0) {

        stats.bufferUnderflows++;
        
        // Increase the sample rate based on what we've measured
        int offPerSecond = (int)(stream.count() / elapsedTime);
        setSampleRate(getSampleRate() + offPerSecond);
    }
}

void
Muxer::handleBufferOverflow()
{
    // There are two common scenarios in which buffer overflows occur:
    //
    // (1) The consumer runs slightly slower than the producer
    // (2) The consumer is halted or not startet yet
    
    trace(AUDBUF_DEBUG, "OVERFLOW (r: %lld w: %lld)\n", stream.r, stream.w);
    
    // Reset the write pointer
    stream.alignWritePtr();

    // Determine the number of elapsed seconds since the last adjustment
    u64 now = Oscillator::nanos();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;
    trace(AUDBUF_DEBUG, "elapsedTime: %f\n", elapsedTime);
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime > 10.0) {
        
        stats.bufferOverflows++;
        
        // Decrease the sample rate based on what we've measured
        int offPerSecond = (int)(stream.count() / elapsedTime);
        double newSampleRate = getSampleRate() - offPerSecond;

        trace(AUDBUF_DEBUG, "Changing sample rate to %f\n", newSampleRate);
        setSampleRate(newSampleRate);
    }
}

void
Muxer::ignoreNextUnderOrOverflow()
{
    lastAlignment = Oscillator::nanos();
}

void
Muxer::copyMono(float *buffer, isize n)
{
    stream.lock();
    
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();
    
    // Copy sound samples
    stream.copyMono(buffer, n, volume.current, volume.target, volume.delta);
    
    stream.unlock();
}

void
Muxer::copyStereo(float *left, float *right, isize n)
{
    stream.lock();
    
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();
    
    // Copy sound samples
    stream.copy(left, right, n, volume.current, volume.target, volume.delta);
    
    stream.unlock();
}

void
Muxer::copyInterleaved(float *buffer, isize n)
{
    stream.lock();
    
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();
    
    // Copy sound samples
    stream.copyInterleaved(buffer, n, volume.current, volume.target, volume.delta);
    
    stream.unlock();
}
