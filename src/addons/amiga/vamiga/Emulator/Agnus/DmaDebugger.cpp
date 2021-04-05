// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DmaDebugger::DmaDebugger(Amiga &ref) : AmigaComponent(ref)
{
    setDescription("DmaDebugger");

    // By default all DMA channels are visualized, except the CPU channel
    for (unsigned i = 0; i < BUS_COUNT; i++) {
        visualize[i] = (i != BUS_NONE) && (i != BUS_CPU);
    }

    // Assign default colors
    setCpuColor      (1.0, 1.0, 1.0);
    setRefreshColor  (1.0, 0.0, 0.0);
    setDiskColor     (0.0, 1.0, 0.0);
    setAudioColor    (1.0, 0.0, 1.0);
    setBitplaneColor (0.0, 1.0, 1.0);
    setSpriteColor   (0.0, 0.5, 1.0);
    setCopperColor   (1.0, 1.0, 0.0);
    setBlitterColor  (1.0, 0.8, 0.0);
}

DMADebuggerInfo
DmaDebugger::getInfo()
{
    DMADebuggerInfo result;
    
    synchronized {
        
        result.enabled = enabled;
        
        result.visualizeCopper = visualize[BUS_COPPER];
        result.visualizeBlitter = visualize[BUS_BLITTER];
        result.visualizeDisk = visualize[BUS_DISK];
        result.visualizeAudio = visualize[BUS_AUDIO];
        result.visualizeSprites = visualize[BUS_SPRITE0];
        result.visualizeBitplanes = visualize[BUS_BPL1];
        result.visualizeCpu = visualize[BUS_CPU];
        result.visualizeRefresh = visualize[BUS_REFRESH];
        
        result.displayMode = displayMode;
        result.opacity = opacity;
        
        getColor(BUS_COPPER, result.copperColor);
        getColor(BUS_BLITTER, result.blitterColor);
        getColor(BUS_DISK, result.diskColor);
        getColor(BUS_AUDIO, result.audioColor);
        getColor(BUS_SPRITE0, result.spriteColor);
        getColor(BUS_BPL1, result.bitplaneColor);
        getColor(BUS_CPU, result.cpuColor);
        getColor(BUS_REFRESH, result.refreshColor);
    }

    return result;
}

void
DmaDebugger::setEnabled(bool value)
{
    if (!enabled && value) {
        enabled = true;
        messageQueue.put(MSG_DMA_DEBUG_ON);
    }
    if (enabled && !value) {
        enabled = false;
        messageQueue.put(MSG_DMA_DEBUG_OFF);
    }
}

bool
DmaDebugger::isVisualized(BusOwner owner)
{
    assert(isBusOwner(owner));
    return visualize[owner];
}

void
DmaDebugger::setVisualized(BusOwner owner, bool value)
{
    assert(isBusOwner(owner));
    visualize[owner] = value;
}

void
DmaDebugger::visualizeCopper(bool value)
{
    setVisualized(BUS_COPPER, value);
}

void
DmaDebugger::visualizeBlitter(bool value)
{
    setVisualized(BUS_BLITTER, value);
}

void
DmaDebugger::visualizeDisk(bool value)
{
    setVisualized(BUS_DISK, value);
}

void
DmaDebugger::visualizeAudio(bool value)
{
    setVisualized(BUS_AUDIO, value);
}

void
DmaDebugger::visualizeSprite(bool value)
{
    setVisualized(BUS_SPRITE0, value);
    setVisualized(BUS_SPRITE1, value);
    setVisualized(BUS_SPRITE2, value);
    setVisualized(BUS_SPRITE3, value);
    setVisualized(BUS_SPRITE4, value);
    setVisualized(BUS_SPRITE5, value);
    setVisualized(BUS_SPRITE6, value);
    setVisualized(BUS_SPRITE7, value);
}

void
DmaDebugger::visualizeBitplane(bool value)
{
    setVisualized(BUS_BPL1, value);
    setVisualized(BUS_BPL2, value);
    setVisualized(BUS_BPL3, value);
    setVisualized(BUS_BPL4, value);
    setVisualized(BUS_BPL5, value);
    setVisualized(BUS_BPL6, value);
}

void
DmaDebugger::visualizeCpu(bool value)
{
    setVisualized(BUS_CPU, value);
}

void
DmaDebugger::visualizeRefresh(bool value)
{
    setVisualized(BUS_REFRESH, value);
}

RgbColor
DmaDebugger::getColor(BusOwner owner)
{
    assert(isBusOwner(owner));
    return debugColor[owner][4];
}

void
DmaDebugger::getColor(BusOwner owner, double *rgb)
{
    RgbColor color = getColor(owner);
    rgb[0] = color.r;
    rgb[1] = color.g;
    rgb[2] = color.b;
}

void
DmaDebugger::setColor(BusOwner owner, RgbColor color)
{
    assert(isBusOwner(owner));

    // Store the original color at an unused location
    debugColor[owner][4] = color;

    // Compute the color variants that are used for drawing
    debugColor[owner][0] = color.shade(0.3);
    debugColor[owner][1] = color.shade(0.1);
    debugColor[owner][2] = color.tint(0.1);
    debugColor[owner][3] = color.tint(0.3);
}

void
DmaDebugger::setColor(BusOwner owner, double r, double g, double b)
{
    assert(isBusOwner(owner));
    setColor(owner, RgbColor(r, g, b));
}

void
DmaDebugger::setCopperColor(double r, double g, double b)
{
    setColor(BUS_COPPER, r, g, b);
}

void
DmaDebugger::setBlitterColor(double r, double g, double b)
{
    setColor(BUS_BLITTER, r, g, b);
}

void
DmaDebugger::setDiskColor(double r, double g, double b)
{
    setColor(BUS_DISK, r, g, b);
}

void
DmaDebugger::setAudioColor(double r, double g, double b)
{
    setColor(BUS_AUDIO, r, g, b);
}

void
DmaDebugger::setSpriteColor(double r, double g, double b)
{
    setColor(BUS_SPRITE0, r, g, b);
    setColor(BUS_SPRITE1, r, g, b);
    setColor(BUS_SPRITE2, r, g, b);
    setColor(BUS_SPRITE3, r, g, b);
    setColor(BUS_SPRITE4, r, g, b);
    setColor(BUS_SPRITE5, r, g, b);
    setColor(BUS_SPRITE6, r, g, b);
    setColor(BUS_SPRITE7, r, g, b);
}

void
DmaDebugger::setBitplaneColor(double r, double g, double b)
{
    setColor(BUS_BPL1, r, g, b);
    setColor(BUS_BPL2, r, g, b);
    setColor(BUS_BPL3, r, g, b);
    setColor(BUS_BPL4, r, g, b);
    setColor(BUS_BPL5, r, g, b);
    setColor(BUS_BPL6, r, g, b);
}

void
DmaDebugger::setCpuColor(double r, double g, double b)
{
    setColor(BUS_CPU, r, g, b);
}

void
DmaDebugger::setRefreshColor(double r, double g, double b)
{
    setColor(BUS_REFRESH, r, g, b);
}

double
DmaDebugger::getOpacity()
{
    return opacity;
}

void
DmaDebugger::setOpacity(double value)
{
    assert(value >= 0.0 && value <= 1.0);
    opacity = value;
}

void
DmaDebugger::computeOverlay()
{
    // Only proceed if DMA debugging has been turned on
    if (!enabled) return;

    BusOwner *owners = agnus.busOwner;
    u16 *values = agnus.busValue;
    u32 *ptr = denise.pixelEngine.pixelAddr(0);

    double bgWeight, fgWeight;

    switch (displayMode) {

        case MODULATE_FG_LAYER:

            bgWeight = 0.0;
            fgWeight = 1.0 - opacity;
            break;

        case MODULATE_BG_LAYER:

            bgWeight = 1.0 - opacity;
            fgWeight = 0.0;
            break;

        case MODULATE_ODD_EVEN_LAYERS:

            bgWeight = opacity;
            fgWeight = 1.0 - opacity;
            break;

        default: assert(false);

    }

    for (int i = 0; i < HPOS_CNT; i++, ptr += 4) {

        BusOwner owner = owners[i];

        // Handle the easy case first: No foreground pixels
        if (!visualize[owner]) {

            if (bgWeight != 0.0) {
                ptr[0] = GpuColor(ptr[0]).shade(bgWeight).rawValue;
                ptr[1] = GpuColor(ptr[1]).shade(bgWeight).rawValue;
                ptr[2] = GpuColor(ptr[2]).shade(bgWeight).rawValue;
                ptr[3] = GpuColor(ptr[3]).shade(bgWeight).rawValue;
            }
            continue;
        }

        // Get RGBA values of foreground pixels
        GpuColor col0 = debugColor[owner][(values[i] & 0xC000) >> 14];
        GpuColor col1 = debugColor[owner][(values[i] & 0x0C00) >> 10];
        GpuColor col2 = debugColor[owner][(values[i] & 0x00C0) >> 6];
        GpuColor col3 = debugColor[owner][(values[i] & 0x000C) >> 2];

        if (fgWeight != 0.0) {
            col0 = col0.mix(GpuColor(ptr[0]), fgWeight);
            col1 = col1.mix(GpuColor(ptr[1]), fgWeight);
            col2 = col2.mix(GpuColor(ptr[2]), fgWeight);
            col3 = col3.mix(GpuColor(ptr[3]), fgWeight);
        }

        ptr[0] = col0.rawValue;
        ptr[1] = col1.rawValue;
        ptr[2] = col2.rawValue;
        ptr[3] = col3.rawValue;
    }
}

void
DmaDebugger::vSyncHandler()
{
    // Only proceed if the debugger is enabled
    if (!enabled) return;

    // Clear old data in the next frame's VBLANK area
    u32 *ptr = denise.pixelEngine.frameBuffer->data;
    for (int row = 0; row < VBLANK_CNT; row++) {
        for (int col = 0; col <= LAST_PIXEL; col++) {
            ptr[row * HPIXELS + col] = PixelEngine::rgbaVBlank;
        }
    }
}

