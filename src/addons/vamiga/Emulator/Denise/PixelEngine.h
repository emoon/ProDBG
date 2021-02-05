// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"

class PixelEngine : public AmigaComponent {

    friend class DmaDebugger;
    
public:

    // RGBA colors used to visualize the HBLANK and VBLANK area in the debugger
    static const i32 rgbaHBlank = 0xFF444444;
    static const i32 rgbaVBlank = 0xFF444444;

private:

    //
    // Screen buffers
    //

    /* The emulator uses double-buffering for storing the computed textures.
     * At any time, one of the two buffers is the "working buffer" and the other
     * one the "stable buffer". All drawing functions write to the working
     * buffer whereas the GPU reads from the stable buffer. Once a frame has
     * been completed, the working buffer and the stable buffer are switched.
     */
    ScreenBuffer emuTexture[2];

    // Pointer to the "working buffer"
    ScreenBuffer *frameBuffer = &emuTexture[0];

    // Buffer with background noise (random black and white pixels)
    u32 *noise;

    
    //
    // Color management
    //

    // The 32 Amiga color registers
    u16 colreg[32];

    // RGBA values for all possible 4096 Amiga colors
    u32 rgba[4096];

    /* The color register values translated to RGBA
     * Note that the number of elements exceeds the number of color registers:
     *  0 .. 31 : RGBA values of the 32 color registers
     * 32 .. 63 : RGBA values of the 32 color registers in halfbright mode
     *       64 : Pure black (used if the ECS BRDRBLNK bit is set)
     * 65 .. 72 : Additional colors used for debugging
     */
    static const int rgbaIndexCnt = 32 + 32 + 1 + 8;
    u32 indexedRgba[rgbaIndexCnt];

    // Color adjustment parameters
    Palette palette = PALETTE_COLOR;
    double brightness = 50.0;
    double contrast = 100.0;
    double saturation = 50.0;
    
    // Indicates whether HAM mode is switched
    bool hamMode;
    
    
    //
    // Register change history buffer
    //

public:

    // Color register history
    RegChangeRecorder<128> colChanges;


    //
    // Initializing
    //
    
public:
    
    PixelEngine(Amiga& ref);
    ~PixelEngine();

    const char *getDescription() const override { return "PixelEngine"; }

    
    //
    // Configuring
    //
    
public:
    
    Palette getPalette() { return palette; }
    void setPalette(Palette p);
    
    double getBrightness() { return brightness; }
    void setBrightness(double value);
    
    double getSaturation() { return saturation; }
    void setSaturation(double value);
    
    double getContrast() { return contrast; }
    void setContrast(double value);
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & colChanges
        & colreg
        & hamMode;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;

    
    //
    // Controlling
    //
    
private:

    void _powerOn() override;
    void _reset(bool hard) override;


    //
    // Accessing color registers
    //

public:

    // Performs a consistency check for debugging.
    static bool isRgbaIndex(isize nr) { return nr < rgbaIndexCnt; }
    
    // Changes one of the 32 Amiga color registers.
    void setColor(isize reg, u16 value);

    // Returns a color value in Amiga format or RGBA format
    u16 getColor(isize nr) const { return colreg[nr]; }
    u32 getRGBA(isize nr) const { return indexedRgba[nr]; }

    // Returns sprite color in Amiga format or RGBA format
    u16 getSpriteColor(isize s, isize nr) const { return getColor(16 + nr + 2 * (s & 6)); }
    u32 getSpriteRGBA(isize s, isize nr) const { return rgba[getSpriteColor(s,nr)]; }


    //
    // Using the color lookup table
    //

private:

    // Updates the entire RGBA lookup table
    void updateRGBA();

    // Adjusts the RGBA value according to the selected color parameters
    void adjustRGB(u8 &r, u8 &g, u8 &b);


    //
    // Working with frame buffers
    //

public:

    // Returns the stable frame buffer for long frames
    ScreenBuffer getStableBuffer();

    // Returns a pointer to randon noise
    u32 *getNoise() const;
    
    // Returns the frame buffer address of a certain pixel in the current line
    u32 *pixelAddr(isize pixel) const;

    // Called after each line in the VBLANK area
    void endOfVBlankLine();

    // Called after each frame to switch the frame buffers
    void beginOfFrame();


    //
    // Working with recorded register changes
    //

public:

    // Applies a register change
    void applyRegisterChange(const RegChange &change);


    //
    // Synthesizing pixels
    //

public:
    
    /* Colorizes a rasterline.
     * This function implements the last stage in the emulator's graphics
     * pipelile. It translates a line of color register indices into a line
     * of RGBA values in GPU format.
     */
    void colorize(isize line);
    
private:
    
    void colorize(u32 *dst, Pixel from, Pixel to);
    void colorizeHAM(u32 *dst, Pixel from, Pixel to, u16& ham);
    
    /* Hides some graphics layers. This function is an optional stage applied
     * after colorize(). It can be used to hide some layers for debugging.
     */
    
public:
    
    void hide(isize line, u16 layer, u8 alpha);
};
