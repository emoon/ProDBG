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
    setDescription("Muxer");
    
    subComponents = vector<HardwareComponent *> {

        &filterL,
        &filterR
    };
    
    setSampleRate(44100);
}
    
void
Muxer::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    stats.bufferUnderflows = 0;
    stats.bufferOverflows = 0;

    sampler[0].clear();
    sampler[1].clear();
    sampler[2].clear();
    sampler[3].clear();

    // Add dummy elements, because some methods assume the buffer is never empty
    sampler[0].write( TaggedSample { 0, 0 } );
    sampler[1].write( TaggedSample { 0, 0 } );
    sampler[2].write( TaggedSample { 0, 0 } );
    sampler[3].write( TaggedSample { 0, 0 } );
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
Muxer::getConfigItem(ConfigOption option)
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
            return (long)(exp2(config.volL) * 100.0);

        case OPT_AUDVOLR:
            return (long)(exp2(config.volR) * 100.0);

        case OPT_AUDVOL0:
            return (long)(exp2(config.vol[0] / 0.0000025) * 100.0);

        case OPT_AUDVOL1:
            return (long)(exp2(config.vol[1] / 0.0000025) * 100.0);
            
        case OPT_AUDVOL2:
            return (long)(exp2(config.vol[2] / 0.0000025) * 100.0);
            
        case OPT_AUDVOL3:
            return (long)(exp2(config.vol[3] / 0.0000025) * 100.0);

        case OPT_AUDPAN0:
            return (long)(config.pan[0] * 100.0);
            
        case OPT_AUDPAN1:
            return (long)(config.pan[1] * 100.0);
            
        case OPT_AUDPAN2:
            return (long)(config.pan[2] * 100.0);
            
        case OPT_AUDPAN3:
            return (long)(config.pan[3] * 100.0);

        default: assert(false);
    }
}

bool
Muxer::setConfigItem(ConfigOption option, long value)
{
    bool wasMuted = isMuted();
    
    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            
            if (!isSamplingMethod(value)) {
                warn("Invalid sampling method: %d\n", value);
                return false;
            }
            break;
            
        case OPT_FILTER_TYPE:
            
            if (!isFilterType(value)) {
                warn("Invalid filter type: %d\n", value);
                warn("       Valid values: 0 ... %d\n", FILT_COUNT - 1);
                return false;
            }
            break;
            
        case OPT_AUDVOLL:
        case OPT_AUDVOLR:
        case OPT_AUDVOL0:
        case OPT_AUDVOL1:
        case OPT_AUDVOL2:
        case OPT_AUDVOL3:
            
            if (value < 100 || value > 400) {
                warn("Invalid volumne: %d\n", value);
                warn("       Valid values: 100 ... 400\n");
                return false;
            }
            break;
            
        case OPT_AUDPAN0:
        case OPT_AUDPAN1:
        case OPT_AUDPAN2:
        case OPT_AUDPAN3:
            
            if (value < 0 || value > 100) {
                warn("Invalid pan: %d\n", value);
                warn("       Valid values: 0 ... 100\n");
                return false;
            }
            break;
            
        default:
            break;
    }

    switch (option) {
            
        case OPT_SAMPLING_METHOD:
            
            if (config.samplingMethod == value) {
                return false;
            }
            
            config.samplingMethod = (SamplingMethod)value;
            return true;
            
        case OPT_FILTER_TYPE:
            
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
            
            config.volL = log2((double)value / 100.0);
            if (wasMuted != isMuted())
                messageQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return true;
            
        case OPT_AUDVOLR:

            config.volR = log2((double)value / 100.0);
            if (wasMuted != isMuted())
                messageQueue.put(isMuted() ? MSG_MUTE_ON : MSG_MUTE_OFF);
            return true;
            
        case OPT_AUDVOL0:
            
            config.vol[0] = log2((double)value / 100.0) * 0.0000025;
            return true;
            
        case OPT_AUDVOL1:
            
            config.vol[1] = log2((double)value / 100.0) * 0.0000025;
            return true;

        case OPT_AUDVOL2:
            
            config.vol[2] = log2((double)value / 100.0) * 0.0000025;
            return true;

        case OPT_AUDVOL3:
            
            config.vol[3] = log2((double)value / 100.0) * 0.0000025;
            return true;

        case OPT_AUDPAN0:
            
            config.pan[0] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        case OPT_AUDPAN1:
            config.pan[1] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        case OPT_AUDPAN2:
            
            config.pan[2] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        case OPT_AUDPAN3:
            
            config.pan[3] = MAX(0.0, MIN(value / 100.0, 1.0));
            return true;

        default:
            return false;
    }
}

void
Muxer::_dumpConfig()
{
    msg("samplingMethod : %s\n", sSamplingMethod(config.samplingMethod));
    msg("    filtertype : %s\n", sFilterType(config.filterType));
    msg("filterAlwaysOn : %d\n", config.filterAlwaysOn);
    msg("    vol0, pan0 : %f, %f\n", config.vol[0], config.pan[0]);
    msg("    vol1, pan1 : %f, %f\n", config.vol[1], config.pan[1]);
    msg("    vol2, pan2 : %f, %f\n", config.vol[2], config.pan[2]);
    msg("    vol3, pan3 : %f, %f\n", config.vol[3], config.pan[3]);
    msg("    volL, volR : %f, %f\n", config.volL, config.volR);
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
    
    bool filter = ciaa.powerLED() || config.filterAlwaysOn;

    // Check for a buffer overflow
    if (stream.count() + count >= stream.cap()) handleBufferOverflow();

    double cycle = clock;
    for (long i = 0; i < count; i++) {

        double ch0 = sampler[0].interpolate<method>((Cycle)cycle) * config.vol[0];
        double ch1 = sampler[1].interpolate<method>((Cycle)cycle) * config.vol[1];
        double ch2 = sampler[2].interpolate<method>((Cycle)cycle) * config.vol[2];
        double ch3 = sampler[3].interpolate<method>((Cycle)cycle) * config.vol[3];

        /*
        if (this == &denise.screenRecorder.muxer)
        {
            dumpConfig();
            debug("ch0: %f ch1: %f ch2: %f ch3: %f\n", ch0, ch1, ch2, ch3);
        }
        */
        
        // Compute left channel output
        float l =
        ch0 * config.pan[0] + ch1 * config.pan[1] +
        ch2 * config.pan[2] + ch3 * config.pan[3];

        // Compute right channel output
        float r =
        ch0 * (1 - config.pan[0]) + ch1 * (1 - config.pan[1]) +
        ch2 * (1 - config.pan[2]) + ch3 * (1 - config.pan[3]);
        
        // Apply audio filter
        if (filter) { l = filterL.apply(l); r = filterR.apply(r); }
        
        // Write sample into ringbuffer
        stream.write( SamplePair { l, r } );
        
        cycle += cyclesPerSample;
    }
}

void
Muxer::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer
    // (2) The producer is halted or not startet yet
    
    trace(AUDBUF_DEBUG, "UNDERFLOW (r: %d w: %d)\n", stream.r, stream.w);
    
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
    
    trace(AUDBUF_DEBUG, "OVERFLOW (r: %d w: %d)\n", stream.r, stream.w);
    
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
    lastAlignment = oscillator.nanos();
}

void
Muxer::copyMono(float *buffer, size_t n)
{
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();
    
    // Read sound samples
    stream.copyMono(buffer, n, volume.current, volume.target, volume.delta);
}

void
Muxer::copyStereo(float *left, float *right, size_t n)
{
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();
    
    // Read sound samples
    stream.copy(left, right, n, volume.current, volume.target, volume.delta);
}

void
Muxer::copyInterleaved(float *buffer, size_t n)
{
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();
    
    // Read sound samples
    stream.copyInterleaved(buffer, n, volume.current, volume.target, volume.delta);
}
