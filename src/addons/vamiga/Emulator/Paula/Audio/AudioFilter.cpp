// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

AudioFilter::AudioFilter(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("AudioFilter");

    a1 = a2 = b0 = b1 = b2 = 0.0;
}

void
AudioFilter::setFilterType(FilterType type)
{
    assert(isFilterType(type));
    this->type = type;
}

void
AudioFilter::setSampleRate(double sampleRate)
{
    trace(AUD_DEBUG, "Setting sample rate to %f Hz\n", sampleRate);
    
    // Compute butterworth filter coefficients based on
    // https://stackoverflow.com/questions/
    //  20924868/calculate-coefficients-of-2nd-order-butterworth-low-pass-filter
    
    // Cutoff frequency in Hz
    const double f_cutoff = 4500;

    // Frequency ratio
    const double ff = f_cutoff / sampleRate;
    
    // Compute coefficients
    const double ita = 1.0/ tan(M_PI*ff);
    const double q = sqrt(2.0);
    
    b0 = 1.0 / (1.0 + q * ita + ita * ita);
    b1 = 2 * b0;
    b2 = b0;
    a1 = 2.0 * (ita * ita - 1.0) * b0;
    a2 = -(1.0 - q * ita + ita * ita) * b0;
}

void
AudioFilter::clear()
{
    x1 = x2 = y1 = y2 = 0.0;
}

float
AudioFilter::apply(float sample)
{
    if (type == FILT_NONE) return sample;
    
    // Apply butterworth filter
    assert(type == FILT_BUTTERWORTH);
    
    // Run pipeline
    double x0 = (double)sample;
    double y0 = (b0 * x0) + (b1 * x1) + (b2 * x2) + (a1 * y1) + (a2 * y2);
    
    // Shift pipeline
    x2 = x1; x1 = x0;
    y2 = y1; y1 = y0;
    
    return (float)y0;
}
