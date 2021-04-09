// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DmaDebuggerTypes.h"
#include "Bus.h"
#include "AmigaComponent.h"
#include "Colors.h"

class DmaDebugger : public AmigaComponent {

private:

    // Indicates if DMA debugging is turned on or off
    bool enabled = false;

    // Indicates if a certain DMA channel should be visualized
    bool visualize[BUS_COUNT]; 

    // DMA debugging colors
    RgbColor debugColor[BUS_COUNT][5];

    // Opacity of DMA pixels
    double opacity = 0.5;

    // Currently selected display mode
    DmaDisplayMode displayMode = DMA_DISPLAY_MODE_FG_LAYER;


    //
    // Initializing
    //

public:

    DmaDebugger(Amiga &ref);

    const char *getDescription() const override { return "DmaDebugger"; }

    void _reset(bool hard) override { }


    //
    // Configuring
    //

public:

    // Turns DMA debugging on or off
    bool isEnabled() const { return enabled; }
    void setEnabled(bool value);

    // Enables or disables the visual effects for a certain DMA source
    bool isVisualized(BusOwner owner) const;
    void setVisualized(BusOwner owner, bool value);
    void visualizeCopper(bool value);
    void visualizeBlitter(bool value);
    void visualizeDisk(bool value);
    void visualizeAudio(bool value);
    void visualizeSprite(bool value);
    void visualizeBitplane(bool value);
    void visualizeCpu(bool value);
    void visualizeRefresh(bool value);

    // Gets or sets the opacity of the superimposed visual effect
    double getOpacity() const { return opacity; }
    void setOpacity(double value);

    // Gets or sets the display mode
    DmaDisplayMode getDisplayMode() { return displayMode; }
    void setDisplayMode(DmaDisplayMode mode) { displayMode = mode; }

    // Gets or sets a debug color
    RgbColor getColor(BusOwner owner) const;
    void getColor(BusOwner owner, double *rgb);
    void setColor(BusOwner owner, RgbColor color);
    void setColor(BusOwner owner, double r, double g, double b);
    void setCopperColor(double r, double g, double b);
    void setBlitterColor(double r, double g, double b);
    void setDiskColor(double r, double g, double b);
    void setAudioColor(double r, double g, double b);
    void setSpriteColor(double r, double g, double b);
    void setBitplaneColor(double r, double g, double b);
    void setCpuColor(double r, double g, double b);
    void setRefreshColor(double r, double g, double b);

    
    //
    // Analyzing
    //
    
public:

    // Returns the result of the most recent call to inspect()
    DMADebuggerInfo getInfo();

    
    //
    // Serializing
    //

private:

    isize _size() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }
    

    //
    // Running the debugger
    //

public:
    
    // Superimposes the debug output onto the current rasterline
    void computeOverlay();

    // Cleans up some texture data at the end of each frame
    void vSyncHandler();
};
