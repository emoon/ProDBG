// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

PixelEngine::PixelEngine(Amiga& ref) : AmigaComponent(ref)
{
    // Allocate frame buffers
    emuTexture[0].data = new u32[PIXELS]; emuTexture[0].longFrame = true;
    emuTexture[1].data = new u32[PIXELS]; emuTexture[1].longFrame = true;
    
    // Create random background noise pattern
    const isize noiseSize = 2 * 512 * 512;
    noise = new u32[noiseSize];
    for (isize i = 0; i < noiseSize; i++) {
        noise[i] = rand() % 2 ? 0xFF000000 : 0xFFFFFFFF;
    }

    // Setup ECS BRDRBLNK color
    indexedRgba[64] = GpuColor(0x00, 0x00, 0x00).rawValue;
    
    // Setup some debug colors
    indexedRgba[65] = GpuColor(0xD0, 0x00, 0x00).rawValue;
    indexedRgba[66] = GpuColor(0xA0, 0x00, 0x00).rawValue;
    indexedRgba[67] = GpuColor(0x90, 0x00, 0x00).rawValue;
    indexedRgba[68] = GpuColor(0x00, 0xFF, 0xFF).rawValue;
    indexedRgba[69] = GpuColor(0x00, 0xD0, 0xD0).rawValue;
    indexedRgba[70] = GpuColor(0x00, 0xA0, 0xA0).rawValue;
    indexedRgba[71] = GpuColor(0x00, 0x90, 0x90).rawValue;
    indexedRgba[72] = GpuColor(0xFF, 0x00, 0x00).rawValue;
}

PixelEngine::~PixelEngine()
{
    delete[] emuTexture[0].data;
    delete[] emuTexture[1].data;
    delete[] noise;
}

isize
PixelEngine::didLoadFromBuffer(const u8 *buffer)
{
    updateRGBA();
    return 0;
}

void
PixelEngine::_powerOn()
{
    // Initialize frame buffers with a checkerboard pattern (for debugging)
    for (isize line = 0; line < VPIXELS; line++) {
        for (isize i = 0; i < HPIXELS; i++) {

            isize pos = line * HPIXELS + i;
            u32 col = (line / 4) % 2 == (i / 8) % 2 ? 0xFF222222 : 0xFF444444;
            emuTexture[0].data[pos] = col;
            emuTexture[1].data[pos] = col;
        }
    }
}

void
PixelEngine::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    frameBuffer = & emuTexture[0];
    updateRGBA();
}

void
PixelEngine::setPalette(Palette p)
{
    palette = p;
    updateRGBA();
}

void
PixelEngine::setBrightness(double value)
{
    brightness = value;
    updateRGBA();
}

void
PixelEngine::setSaturation(double value)
{
    saturation = value;
    updateRGBA();
}

void
PixelEngine::setContrast(double value)
{
    contrast = value;
    updateRGBA();

}

void
PixelEngine::setColor(isize reg, u16 value)
{
    assert(reg < 32);

    colreg[reg] = value & 0xFFF;

    u8 r = (value & 0xF00) >> 8;
    u8 g = (value & 0x0F0) >> 4;
    u8 b = (value & 0x00F);

    indexedRgba[reg] = rgba[value & 0xFFF];
    indexedRgba[reg + 32] = rgba[((r / 2) << 8) | ((g / 2) << 4) | (b / 2)];
}

void
PixelEngine::updateRGBA()
{
    // Iterate through all 4096 colors
    for (u16 col = 0x000; col <= 0xFFF; col++) {

        // Convert the Amiga color into an RGBA value
        u8 r = (col >> 4) & 0xF0;
        u8 g = (col >> 0) & 0xF0;
        u8 b = (col << 4) & 0xF0;

        // Convert the Amiga value to an RGBA value
        adjustRGB(r, g, b);

        // Write the result into the register lookup table
        rgba[col] = HI_HI_LO_LO(0xFF, b, g, r);
    }

    // Update all RGBA values that are cached in indexedRgba[]
    for (isize i = 0; i < 32; i++) setColor(i, colreg[i]);
}

void
PixelEngine::adjustRGB(u8 &r, u8 &g, u8 &b)
{
    // Normalize adjustment parameters
    double brightness = this->brightness - 50.0;
    double contrast = this->contrast / 100.0;
    double saturation = this->saturation / 50.0;

    // Convert RGB to YUV
    double y =  0.299 * r + 0.587 * g + 0.114 * b;
    double u = -0.147 * r - 0.289 * g + 0.436 * b;
    double v =  0.615 * r - 0.515 * g - 0.100 * b;

    // Adjust saturation
    u *= saturation;
    v *= saturation;

    // Apply contrast
    y *= contrast;
    u *= contrast;
    v *= contrast;

    // Apply brightness
    y += brightness;

    // Translate to monochrome if applicable
    switch(palette) {

        case PALETTE_BLACK_WHITE:
            u = 0.0;
            v = 0.0;
            break;

        case PALETTE_PAPER_WHITE:
            u = -128.0 + 120.0;
            v = -128.0 + 133.0;
            break;

        case PALETTE_GREEN:
            u = -128.0 + 29.0;
            v = -128.0 + 64.0;
            break;

        case PALETTE_AMBER:
            u = -128.0 + 24.0;
            v = -128.0 + 178.0;
            break;

        case PALETTE_SEPIA:
            u = -128.0 + 97.0;
            v = -128.0 + 154.0;
            break;

        default:
            assert(palette == PALETTE_COLOR);
    }

    // Convert YUV to RGB
    double newR = y             + 1.140 * v;
    double newG = y - 0.396 * u - 0.581 * v;
    double newB = y + 2.029 * u;
    newR = MAX(MIN(newR, 255), 0);
    newG = MAX(MIN(newG, 255), 0);
    newB = MAX(MIN(newB, 255), 0);

    // Apply Gamma correction for PAL models
    /*
     r = gammaCorrect(r, 2.8, 2.2);
     g = gammaCorrect(g, 2.8, 2.2);
     b = gammaCorrect(b, 2.8, 2.2);
     */

    r = u8(newR);
    g = u8(newG);
    b = u8(newB);
}

ScreenBuffer
PixelEngine::getStableBuffer()
{
    ScreenBuffer result;
    
    synchronized {
        result = (frameBuffer == &emuTexture[0]) ? emuTexture[1] : emuTexture[0];
    }
    
    assert(result.data);
    return result;
}

u32 *
PixelEngine::getNoise() const
{
    int offset = rand() % (512 * 512);
    return noise + offset;
}

u32 *
PixelEngine::pixelAddr(isize pixel) const
{
    isize offset = pixel + agnus.pos.v * HPIXELS;

    assert(pixel < HPIXELS);
    assert(offset < PIXELS);

    return frameBuffer->data + offset;
}

void
PixelEngine::beginOfFrame()
{
    // Switch the working buffer
    synchronized {
        frameBuffer = (frameBuffer == &emuTexture[0]) ? &emuTexture[1] : &emuTexture[0];
        frameBuffer->longFrame = agnus.frame.lof;
    }
    
    dmaDebugger.vSyncHandler();
}

void
PixelEngine::endOfVBlankLine()
{
    // Apply all color register changes that happened in this line
    for (isize i = colChanges.begin(); i != colChanges.end(); i = colChanges.next(i)) {
        applyRegisterChange(colChanges.elements[i]);
    }
}

void
PixelEngine::applyRegisterChange(const RegChange &change)
{
    switch (change.addr) {

        case 0:
            break;

        case BPLCON0:
            hamMode = Denise::ham(change.value);
            break;
            
        default: // It must be a color register then
            assert(change.addr >= 0x180 && change.addr <= 0x1BE);
            setColor((change.addr - 0x180) >> 1, change.value);
            break;
    }
}

void
PixelEngine::colorize(isize line)
{
    // Jump to the first pixel in the specified line in the active frame buffer
    u32 *dst = frameBuffer->data + line * HPIXELS;
    Pixel pixel = 0;

    // Initialize the HAM mode hold register with the current background color
    u16 hold = colreg[0];

    // Add a dummy register change to ensure we draw until the line end
    colChanges.insert(HPIXELS, RegChange { SET_NONE, 0 } );

    // Iterate over all recorded register changes
    for (isize i = colChanges.begin(); i != colChanges.end(); i = colChanges.next(i)) {

        Pixel trigger = (Pixel)colChanges.keys[i];
        RegChange &change = colChanges.elements[i];

        // Colorize a chunk of pixels
        if (hamMode) {
            colorizeHAM(dst, pixel, trigger, hold);
        } else {
            colorize(dst, pixel, trigger);
        }
        pixel = trigger;

        // Perform the register change
        applyRegisterChange(change);
    }

    // Wipe out the HBLANK area
    for (Pixel pixel = 4 * HBLANK_MIN; pixel <= 4 * HBLANK_MAX; pixel++) {
        dst[pixel] = rgbaHBlank;
    }

    // Clear the history cache
    colChanges.clear();
}

void
PixelEngine::colorize(u32 *dst, Pixel from, Pixel to)
{
    u8 *mbuf = denise.mBuffer;

    for (Pixel i = from; i < to; i++) {
        dst[i] = indexedRgba[mbuf[i]];
    }
}

void
PixelEngine::colorizeHAM(u32 *dst, Pixel from, Pixel to, u16& ham)
{
    u8 *bbuf = denise.bBuffer;
    u8 *ibuf = denise.iBuffer;
    u8 *mbuf = denise.mBuffer;

    for (Pixel i = from; i < to; i++) {

        u8 index = ibuf[i];
        assert(isRgbaIndex(index));

        // switch ((index >> 4) & 0b11) {
        switch ((bbuf[i] >> 4) & 0b11) {

            case 0b00: // Get color from register

                ham = colreg[index];
                break;

            case 0b01: // Modify blue

                ham &= 0xFF0;
                ham |= (index & 0b1111);
                break;

            case 0b10: // Modify red

                ham &= 0x0FF;
                ham |= (index & 0b1111) << 8;
                break;

            case 0b11: // Modify green

                ham &= 0xF0F;
                ham |= (index & 0b1111) << 4;
                break;

            default:
                assert(false);
        }

        // Synthesize pixel
        if (denise.spritePixelIsVisible(i)) {
            dst[i] = rgba[colreg[mbuf[i]]];
        } else {
            dst[i] = rgba[ham];
        }
    }
}

void
PixelEngine::hide(isize line, u16 layers, u8 alpha)
{
    u32 *p = frameBuffer->data + line * HPIXELS;

    for (Pixel i = 0; i < HPIXELS; i++) {

        u16 z = denise.zBuffer[i];

        // Check for case 1: A sprite is visible
        if (Denise::isSpritePixel(z)) {

            if (Denise::isSpritePixel<0>(z) && !(layers & 0x01)) continue;
            if (Denise::isSpritePixel<1>(z) && !(layers & 0x02)) continue;
            if (Denise::isSpritePixel<2>(z) && !(layers & 0x04)) continue;
            if (Denise::isSpritePixel<3>(z) && !(layers & 0x08)) continue;
            if (Denise::isSpritePixel<4>(z) && !(layers & 0x10)) continue;
            if (Denise::isSpritePixel<5>(z) && !(layers & 0x20)) continue;
            if (Denise::isSpritePixel<6>(z) && !(layers & 0x40)) continue;
            if (Denise::isSpritePixel<7>(z) && !(layers & 0x80)) continue;
        
        } else {

            // Check for case 2: Playfield 1 is visible
            if ((Denise::upperPlayfield(z) == 1) && !(layers & 0x100)) continue;
        
            // Check for case 3: layfield 2 is visible
            if ((Denise::upperPlayfield(z) == 2) && !(layers & 0x200)) continue;
        }
        
        u8 r = p[i] & 0xFF;
        u8 g = (p[i] >> 8) & 0xFF;
        u8 b = (p[i] >> 16) & 0xFF;

        double scale = alpha / 255.0;
        u8 bg = (line / 4) % 2 == (i / 8) % 2 ? 0x22 : 0x44;
        u8 newr = (u8)(r * (1 - scale) + bg * scale);
        u8 newg = (u8)(g * (1 - scale) + bg * scale);
        u8 newb = (u8)(b * (1 - scale) + bg * scale);
        
        p[i] = 0xFF000000 | newb << 16 | newg << 8 | newr;
    }
}
