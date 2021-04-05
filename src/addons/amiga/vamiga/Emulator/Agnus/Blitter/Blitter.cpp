// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Blitter::Blitter(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("Blitter");
    
    // Initialize fill pattern lookup tables
    
    // Inclusive fill
    for (unsigned carryIn = 0; carryIn < 2; carryIn++) {
        
        for (unsigned byte = 0; byte < 256; byte++) {
            
            u8 carry = carryIn;
            u8 inclPattern = byte;
            u8 exclPattern = byte;
            
            for (int bit = 0; bit < 8; bit++) {
                
                inclPattern |= carry << bit; // inclusive fill
                exclPattern ^= carry << bit; // exclusive fill
                
                if (byte & (1 << bit)) carry = !carry;
            }
            fillPattern[0][carryIn][byte] = inclPattern;
            fillPattern[1][carryIn][byte] = exclPattern;
            nextCarryIn[carryIn][byte] = carry;
        }
    }
}

void
Blitter::_initialize()
{    
    initFastBlitter();
    initSlowBlitter();
}

void
Blitter::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {
        copycount = 0;
        linecount = 0;
    }
}

long
Blitter::getConfigItem(ConfigOption option)
{
    switch (option) {
            
        case OPT_BLITTER_ACCURACY: return config.accuracy;
        default: assert(false);
    }
}

bool
Blitter::setConfigItem(ConfigOption option, long value)
{
    switch (option) {
            
        case OPT_BLITTER_ACCURACY:
            
            #ifdef FORCE_BLT_LEVEL
            value = FORCE_BLT_LEVEL;
            warn("Overriding Blitter accuracy level: %d\n", value);
            #endif
            
            if (value < 0 || value > 2) {
                warn("Invalid Blitter accuracy level: %d\n", value);
                return false;
            }
            if (config.accuracy == value) {
                return false;
            }
            
            amiga.suspend();
            config.accuracy = value;
            amiga.resume();

            return true;
            
        default:
            return false;
    }
}

void
Blitter::_dumpConfig()
{
    msg("      accuracy : %d\n", config.accuracy);
}

void
Blitter::_inspect()
{
    synchronized {
        
        info.bltcon0 = bltcon0;
        info.bltcon1 = bltcon1;
        info.ash = bltconASH();
        info.bsh = bltconBSH();
        info.minterm = bltconLF();
        info.bltapt  = bltapt;
        info.bltbpt  = bltbpt;
        info.bltcpt  = bltcpt;
        info.bltdpt  = bltdpt;
        info.bltafwm = bltafwm;
        info.bltalwm = bltalwm;
        info.bltamod = bltamod;
        info.bltbmod = bltbmod;
        info.bltcmod = bltcmod;
        info.bltdmod = bltdmod;
        info.aold = aold;
        info.bold = bold;
        info.anew = anew;
        info.bnew = bnew;
        info.ahold = ahold;
        info.bhold = bhold;
        info.chold = chold;
        info.dhold = dhold;
        info.bbusy = bbusy;
        info.bzero = bzero;
        info.firstWord = isFirstWord();
        info.lastWord = isLastWord();
        info.fci = bltconFCI();
        info.fco = fillCarry;
        info.fillEnable = bltconFE();
        info.storeToDest = bltconUSED() && !lockD;
    }
}

void
Blitter::_dump()
{
    msg("  Accuracy: %d\n", config.accuracy);
    msg("\n");
    msg("   bltcon0: %X\n", bltcon0);
    msg("\n");
    msg("            Shift A: %d\n", bltconASH());
    msg("              Use A: %s\n", bltconUSEA() ? "yes" : "no");
    msg("              Use B: %s\n", bltconUSEB() ? "yes" : "no");
    msg("              Use C: %s\n", bltconUSEC() ? "yes" : "no");
    msg("              Use D: %s\n", bltconUSED() ? "yes" : "no");
    msg("\n");
    msg("   bltcon1: %X\n", bltcon1);
    msg("\n");
    msg("            Shift B: %d\n", bltconBSH());
    msg("                EFE: %s\n", bltconEFE() ? "yes" : "no");
    msg("                IFE: %s\n", bltconIFE() ? "yes" : "no");
    msg("                FCI: %s\n", bltconFCI() ? "yes" : "no");
    msg("               DESC: %s\n", bltconDESC() ? "yes" : "no");
    msg("               LINE: %s\n", bltconLINE() ? "yes" : "no");
    msg("\n");
    msg("  bltsizeH: %d\n", bltsizeV);
    msg("  bltsizeW: %d\n", bltsizeH);
    msg("\n");
    msg("    bltapt: %X\n", bltapt);
    msg("    bltbpt: %X\n", bltbpt);
    msg("    bltcpt: %X\n", bltcpt);
    msg("    bltdpt: %X\n", bltdpt);
    msg("   bltafwm: %X\n", bltafwm);
    msg("   bltalwm: %X\n", bltalwm);
    msg("   bltamod: %X\n", bltamod);
    msg("   bltbmod: %X\n", bltbmod);
    msg("   bltcmod: %X\n", bltcmod);
    msg("   bltdmod: %X\n", bltdmod);
    msg("      anew: %X\n", anew);
    msg("      bnew: %X\n", bnew);
    msg("     ahold: %X\n", ahold);
    msg("     bhold: %X\n", bhold);
    msg("     chold: %X\n", chold);
    msg("     dhold: %X\n", dhold);
    msg("    ashift: %X bshift: %X\n", ashift, bshift);
    msg("     bbusy: %s bzero: %s\n", bbusy ? "yes" : "no", bzero ? "yes" : "no");
}

u16
Blitter::doMintermLogic(u16 a, u16 b, u16 c, u8 minterm)
{
    u16 result = 0;

    if (minterm & 0b10000000) result |=  a &  b &  c;
    if (minterm & 0b01000000) result |=  a &  b & ~c;
    if (minterm & 0b00100000) result |=  a & ~b &  c;
    if (minterm & 0b00010000) result |=  a & ~b & ~c;
    if (minterm & 0b00001000) result |= ~a &  b &  c;
    if (minterm & 0b00000100) result |= ~a &  b & ~c;
    if (minterm & 0b00000010) result |= ~a & ~b &  c;
    if (minterm & 0b00000001) result |= ~a & ~b & ~c;
    
    return result;
}

u16
Blitter::doMintermLogicQuick(u16 a, u16 b, u16 c, u8 minterm)
{
    switch (minterm) {
        case 0: return 0;
        case 1: return (~c & ~b & ~a);
        case 2: return (c & ~b & ~a);
        case 3: return (~b & ~a);
        case 4: return (~c & b & ~a);
        case 5: return (~c & ~a);
        case 6: return (c & ~b & ~a) | (~c & b & ~a);
        case 7: return (~b & ~a) | (~c & ~a);
        case 8: return (c & b & ~a);
        case 9: return (~c & ~b & ~a) | (c & b & ~a);
        case 10: return (c & ~a);
        case 11: return (~b & ~a) | (c & ~a);
        case 12: return (b & ~a);
        case 13: return (~c & ~a) | (b & ~a);
        case 14: return (c & ~a) | (b & ~a);
        case 15: return (~a);
        case 16: return (~c & ~b & a);
        case 17: return (~c & ~b);
        case 18: return (c & ~b & ~a) | (~c & ~b & a);
        case 19: return (~b & ~a) | (~c & ~b);
        case 20: return (~c & b & ~a) | (~c & ~b & a);
        case 21: return (~c & ~a) | (~c & ~b);
        case 22: return (c & ~b & ~a) | (~c & b & ~a) | (~c & ~b & a);
        case 23: return (~b & ~a) | (~c & ~a) | (~c & ~b);
        case 24: return (c & b & ~a) | (~c & ~b & a);
        case 25: return (~c & ~b) | (c & b & ~a);
        case 26: return (c & ~a) | (~c & ~b & a);
        case 27: return (~b & ~a) | (c & ~a) | (~c & ~b);
        case 28: return (b & ~a) | (~c & ~b & a);
        case 29: return (~c & ~a) | (b & ~a) | (~c & ~b);
        case 30: return (c & ~a) | (b & ~a) | (~c & ~b & a);
        case 31: return (~a) | (~c & ~b);
        case 32: return (c & ~b & a);
        case 33: return (~c & ~b & ~a) | (c & ~b & a);
        case 34: return (c & ~b);
        case 35: return (~b & ~a) | (c & ~b);
        case 36: return (~c & b & ~a) | (c & ~b & a);
        case 37: return (~c & ~a) | (c & ~b & a);
        case 38: return (c & ~b) | (~c & b & ~a);
        case 39: return (~b & ~a) | (~c & ~a) | (c & ~b);
        case 40: return (c & b & ~a) | (c & ~b & a);
        case 41: return (~c & ~b & ~a) | (c & b & ~a) | (c & ~b & a);
        case 42: return (c & ~a) | (c & ~b);
        case 43: return (~b & ~a) | (c & ~a) | (c & ~b);
        case 44: return (b & ~a) | (c & ~b & a);
        case 45: return (~c & ~a) | (b & ~a) | (c & ~b & a);
        case 46: return (c & ~a) | (b & ~a) | (c & ~b);
        case 47: return (~a) | (c & ~b);
        case 48: return (~b & a);
        case 49: return (~c & ~b) | (~b & a);
        case 50: return (c & ~b) | (~b & a);
        case 51: return (~b);
        case 52: return (~c & b & ~a) | (~b & a);
        case 53: return (~c & ~a) | (~b & a);
        case 54: return (c & ~b) | (~c & b & ~a) | (~b & a);
        case 55: return (~b) | (~c & ~a);
        case 56: return (c & b & ~a) | (~b & a);
        case 57: return (~c & ~b) | (c & b & ~a) | (~b & a);
        case 58: return (c & ~a) | (~b & a);
        case 59: return (~b) | (c & ~a);
        case 60: return (b & ~a) | (~b & a);
        case 61: return (~c & ~a) | (b & ~a) | (~b & a);
        case 62: return (c & ~a) | (b & ~a) | (~b & a);
        case 63: return (~a) | (~b);
        case 64: return (~c & b & a);
        case 65: return (~c & ~b & ~a) | (~c & b & a);
        case 66: return (c & ~b & ~a) | (~c & b & a);
        case 67: return (~b & ~a) | (~c & b & a);
        case 68: return (~c & b);
        case 69: return (~c & ~a) | (~c & b);
        case 70: return (c & ~b & ~a) | (~c & b);
        case 71: return (~b & ~a) | (~c & ~a) | (~c & b);
        case 72: return (c & b & ~a) | (~c & b & a);
        case 73: return (~c & ~b & ~a) | (c & b & ~a) | (~c & b & a);
        case 74: return (c & ~a) | (~c & b & a);
        case 75: return (~b & ~a) | (c & ~a) | (~c & b & a);
        case 76: return (b & ~a) | (~c & b);
        case 77: return (~c & ~a) | (b & ~a) | (~c & b);
        case 78: return (c & ~a) | (b & ~a) | (~c & b);
        case 79: return (~a) | (~c & b);
        case 80: return (~c & a);
        case 81: return (~c & ~b) | (~c & a);
        case 82: return (c & ~b & ~a) | (~c & a);
        case 83: return (~b & ~a) | (~c & a);
        case 84: return (~c & b) | (~c & a);
        case 85: return (~c);
        case 86: return (c & ~b & ~a) | (~c & b) | (~c & a);
        case 87: return (~b & ~a) | (~c);
        case 88: return (c & b & ~a) | (~c & a);
        case 89: return (~c & ~b) | (c & b & ~a) | (~c & a);
        case 90: return (c & ~a) | (~c & a);
        case 91: return (~b & ~a) | (c & ~a) | (~c & a);
        case 92: return (b & ~a) | (~c & a);
        case 93: return (~c) | (b & ~a);
        case 94: return (c & ~a) | (b & ~a) | (~c & a);
        case 95: return (~a) | (~c);
        case 96: return (c & ~b & a) | (~c & b & a);
        case 97: return (~c & ~b & ~a) | (c & ~b & a) | (~c & b & a);
        case 98: return (c & ~b) | (~c & b & a);
        case 99: return (~b & ~a) | (c & ~b) | (~c & b & a);
        case 100: return (~c & b) | (c & ~b & a);
        case 101: return (~c & ~a) | (c & ~b & a) | (~c & b);
        case 102: return (c & ~b) | (~c & b);
        case 103: return (~b & ~a) | (~c & ~a) | (c & ~b) | (~c & b);
        case 104: return (c & b & ~a) | (c & ~b & a) | (~c & b & a);
        case 105: return (~c & ~b & ~a) | (c & b & ~a) | (c & ~b & a) | (~c & b & a);
        case 106: return (c & ~a) | (c & ~b) | (~c & b & a);
        case 107: return (~b & ~a) | (c & ~a) | (c & ~b) | (~c & b & a);
        case 108: return (b & ~a) | (c & ~b & a) | (~c & b);
        case 109: return (~c & ~a) | (b & ~a) | (c & ~b & a) | (~c & b);
        case 110: return (c & ~a) | (b & ~a) | (c & ~b) | (~c & b);
        case 111: return (~a) | (c & ~b) | (~c & b);
        case 112: return (~b & a) | (~c & a);
        case 113: return (~c & ~b) | (~b & a) | (~c & a);
        case 114: return (c & ~b) | (~b & a) | (~c & a);
        case 115: return (~b) | (~c & a);
        case 116: return (~c & b) | (~b & a);
        case 117: return (~c) | (~b & a);
        case 118: return (c & ~b) | (~c & b) | (~b & a);
        case 119: return (~b) | (~c);
        case 120: return (c & b & ~a) | (~b & a) | (~c & a);
        case 121: return (~c & ~b) | (c & b & ~a) | (~b & a) | (~c & a);
        case 122: return (c & ~a) | (~b & a) | (~c & a);
        case 123: return (~b) | (c & ~a) | (~c & a);
        case 124: return (b & ~a) | (~b & a) | (~c & a);
        case 125: return (~c) | (b & ~a) | (~b & a);
        case 126: return (c & ~a) | (b & ~a) | (~b & a) | (~c & a);
        case 127: return (~a) | (~b) | (~c);
        case 128: return (c & b & a);
        case 129: return (~c & ~b & ~a) | (c & b & a);
        case 130: return (c & ~b & ~a) | (c & b & a);
        case 131: return (~b & ~a) | (c & b & a);
        case 132: return (~c & b & ~a) | (c & b & a);
        case 133: return (~c & ~a) | (c & b & a);
        case 134: return (c & ~b & ~a) | (~c & b & ~a) | (c & b & a);
        case 135: return (~b & ~a) | (~c & ~a) | (c & b & a);
        case 136: return (c & b);
        case 137: return (~c & ~b & ~a) | (c & b);
        case 138: return (c & ~a) | (c & b);
        case 139: return (~b & ~a) | (c & ~a) | (c & b);
        case 140: return (b & ~a) | (c & b);
        case 141: return (~c & ~a) | (b & ~a) | (c & b);
        case 142: return (c & ~a) | (b & ~a) | (c & b);
        case 143: return (~a) | (c & b);
        case 144: return (~c & ~b & a) | (c & b & a);
        case 145: return (~c & ~b) | (c & b & a);
        case 146: return (c & ~b & ~a) | (~c & ~b & a) | (c & b & a);
        case 147: return (~b & ~a) | (~c & ~b) | (c & b & a);
        case 148: return (~c & b & ~a) | (~c & ~b & a) | (c & b & a);
        case 149: return (~c & ~a) | (~c & ~b) | (c & b & a);
        case 150: return (c & ~b & ~a) | (~c & b & ~a) | (~c & ~b & a) | (c & b & a);
        case 151: return (~b & ~a) | (~c & ~a) | (~c & ~b) | (c & b & a);
        case 152: return (c & b) | (~c & ~b & a);
        case 153: return (~c & ~b) | (c & b);
        case 154: return (c & ~a) | (~c & ~b & a) | (c & b);
        case 155: return (~b & ~a) | (c & ~a) | (~c & ~b) | (c & b);
        case 156: return (b & ~a) | (~c & ~b & a) | (c & b);
        case 157: return (~c & ~a) | (b & ~a) | (~c & ~b) | (c & b);
        case 158: return (c & ~a) | (b & ~a) | (~c & ~b & a) | (c & b);
        case 159: return (~a) | (~c & ~b) | (c & b);
        case 160: return (c & a);
        case 161: return (~c & ~b & ~a) | (c & a);
        case 162: return (c & ~b) | (c & a);
        case 163: return (~b & ~a) | (c & a);
        case 164: return (~c & b & ~a) | (c & a);
        case 165: return (~c & ~a) | (c & a);
        case 166: return (c & ~b) | (~c & b & ~a) | (c & a);
        case 167: return (~b & ~a) | (~c & ~a) | (c & a);
        case 168: return (c & b) | (c & a);
        case 169: return (~c & ~b & ~a) | (c & b) | (c & a);
        case 170: return (c);
        case 171: return (~b & ~a) | (c);
        case 172: return (b & ~a) | (c & a);
        case 173: return (~c & ~a) | (b & ~a) | (c & a);
        case 174: return (c) | (b & ~a);
        case 175: return (~a) | (c);
        case 176: return (~b & a) | (c & a);
        case 177: return (~c & ~b) | (~b & a) | (c & a);
        case 178: return (c & ~b) | (~b & a) | (c & a);
        case 179: return (~b) | (c & a);
        case 180: return (~c & b & ~a) | (~b & a) | (c & a);
        case 181: return (~c & ~a) | (~b & a) | (c & a);
        case 182: return (c & ~b) | (~c & b & ~a) | (~b & a) | (c & a);
        case 183: return (~b) | (~c & ~a) | (c & a);
        case 184: return (c & b) | (~b & a);
        case 185: return (~c & ~b) | (c & b) | (~b & a);
        case 186: return (c) | (~b & a);
        case 187: return (~b) | (c);
        case 188: return (b & ~a) | (~b & a) | (c & a);
        case 189: return (~c & ~a) | (b & ~a) | (~b & a) | (c & a);
        case 190: return (c) | (b & ~a) | (~b & a);
        case 191: return (~a) | (~b) | (c);
        case 192: return (b & a);
        case 193: return (~c & ~b & ~a) | (b & a);
        case 194: return (c & ~b & ~a) | (b & a);
        case 195: return (~b & ~a) | (b & a);
        case 196: return (~c & b) | (b & a);
        case 197: return (~c & ~a) | (b & a);
        case 198: return (c & ~b & ~a) | (~c & b) | (b & a);
        case 199: return (~b & ~a) | (~c & ~a) | (b & a);
        case 200: return (c & b) | (b & a);
        case 201: return (~c & ~b & ~a) | (c & b) | (b & a);
        case 202: return (c & ~a) | (b & a);
        case 203: return (~b & ~a) | (c & ~a) | (b & a);
        case 204: return (b);
        case 205: return (~c & ~a) | (b);
        case 206: return (c & ~a) | (b);
        case 207: return (~a) | (b);
        case 208: return (~c & a) | (b & a);
        case 209: return (~c & ~b) | (b & a);
        case 210: return (c & ~b & ~a) | (~c & a) | (b & a);
        case 211: return (~b & ~a) | (~c & a) | (b & a);
        case 212: return (~c & b) | (~c & a) | (b & a);
        case 213: return (~c) | (b & a);
        case 214: return (c & ~b & ~a) | (~c & b) | (~c & a) | (b & a);
        case 215: return (~b & ~a) | (~c) | (b & a);
        case 216: return (c & b) | (~c & a);
        case 217: return (~c & ~b) | (c & b) | (b & a);
        case 218: return (c & ~a) | (~c & a) | (b & a);
        case 219: return (~b & ~a) | (c & ~a) | (~c & a) | (b & a);
        case 220: return (b) | (~c & a);
        case 221: return (~c) | (b);
        case 222: return (c & ~a) | (b) | (~c & a);
        case 223: return (~a) | (~c) | (b);
        case 224: return (c & a) | (b & a);
        case 225: return (~c & ~b & ~a) | (c & a) | (b & a);
        case 226: return (c & ~b) | (b & a);
        case 227: return (~b & ~a) | (c & a) | (b & a);
        case 228: return (~c & b) | (c & a);
        case 229: return (~c & ~a) | (c & a) | (b & a);
        case 230: return (c & ~b) | (~c & b) | (b & a);
        case 231: return (~b & ~a) | (~c & ~a) | (c & a) | (b & a);
        case 232: return (c & b) | (c & a) | (b & a);
        case 233: return (~c & ~b & ~a) | (c & b) | (c & a) | (b & a);
        case 234: return (c) | (b & a);
        case 235: return (~b & ~a) | (c) | (b & a);
        case 236: return (b) | (c & a);
        case 237: return (~c & ~a) | (b) | (c & a);
        case 238: return (c) | (b);
        case 239: return (~a) | (c) | (b);
        case 240: return (a);
        case 241: return (~c & ~b) | (a);
        case 242: return (c & ~b) | (a);
        case 243: return (~b) | (a);
        case 244: return (~c & b) | (a);
        case 245: return (~c) | (a);
        case 246: return (c & ~b) | (~c & b) | (a);
        case 247: return (~b) | (~c) | (a);
        case 248: return (c & b) | (a);
        case 249: return (~c & ~b) | (c & b) | (a);
        case 250: return (c) | (a);
        case 251: return (~b) | (c) | (a);
        case 252: return (b) | (a);
        case 253: return (~c) | (b) | (a);
        case 254: return (c) | (b) | (a);
        default:  return 0xFFFF;
    }
}

void
Blitter::doFill(u16 &data, bool &carry)
{
    assert(carry == 0 || carry == 1);

    trace(BLT_DEBUG, "data = %X carry = %X\n", data, carry);
    
    u8 dataHi = HI_BYTE(data);
    u8 dataLo = LO_BYTE(data);
    u8 exclusive = !!bltconEFE();
    
    // Remember: A fill operation is carried out from right to left
    u8 resultLo = fillPattern[exclusive][carry][dataLo];
    carry = nextCarryIn[carry][dataLo];
    u8 resultHi = fillPattern[exclusive][carry][dataHi];
    carry = nextCarryIn[carry][dataHi];
    
    data = HI_LO(resultHi, resultLo);
}

void
Blitter::prepareBlit()
{
    remaining = bltsizeH * bltsizeV;
    cntA = cntB = cntC = cntD = bltsizeH;

    running = true;
    bzero = true;
    bbusy = true;
    birq = false;

    bltpc = 0;
    iteration = 0;
}

void
Blitter::beginBlit()
{
    int level = config.accuracy;

    if (BLT_GUARD) memset(memguard, 0, sizeof(memguard));
    
    if (bltconLINE()) {

        if (BLT_CHECKSUM) {
            linecount++;
            check1 = check2 = fnv_1a_init32();
            debug("Line %d (%d,%d) (%d%d%d%d)[%x] (%d %d %d %d) %x %x %x %x\n",
                       linecount, bltsizeH, bltsizeV,
                       bltconUSEA(), bltconUSEB(), bltconUSEC(), bltconUSED(),
                       bltcon0,
                       bltamod, bltbmod, bltcmod, bltdmod,
                       bltapt & agnus.ptrMask,
                       bltbpt & agnus.ptrMask,
                       bltcpt & agnus.ptrMask,
                       bltdpt & agnus.ptrMask);
        }

        beginLineBlit(level);

    } else {

        if (BLT_CHECKSUM) { // && (bltsizeH != 1 || bltsizeV != 4)
            copycount++;
            check1 = check2 = fnv_1a_init32();
            
            debug("Blit %d (%d,%d) (%d%d%d%d)[%x] (%d %d %d %d) %x %x %x %x %s%s\n",
                       copycount,
                       bltsizeH, bltsizeV,
                       bltconUSEA(), bltconUSEB(), bltconUSEC(), bltconUSED(),
                       bltcon0,
                       bltamod, bltbmod, bltcmod, bltdmod,
                       bltapt & agnus.ptrMask,
                       bltbpt & agnus.ptrMask,
                       bltcpt & agnus.ptrMask,
                       bltdpt & agnus.ptrMask,
                       bltconDESC() ? "D" : "", bltconFE() ? "F" : "");
        }

        beginCopyBlit(level);
    }
}

void
Blitter::beginLineBlit(int level)
{
    static bool verbose = true;

    if (BLT_CHECKSUM && verbose) {
        verbose = false;
        msg("Performing level %d line blits.\n", level);
    }

    switch (level) {
        case 0: beginFastLineBlit(); break;
        case 1: beginFakeLineBlit(); break;
        case 2: beginSlowLineBlit(); break;
        default: assert(false);
    }
}

void
Blitter::beginCopyBlit(int level)
{
    static bool verbose = true;

    if (BLT_CHECKSUM && verbose) {
        verbose = false;
        msg("Performing level %d copy blits.\n", level);
    }

    switch (level) {
        case 0: beginFastCopyBlit(); break;
        case 1: beginFakeCopyBlit(); break;
        case 2: beginSlowCopyBlit(); break;
        default: assert(false);
    }
}

void
Blitter::signalEnd()
{
    debug(BLTTIM_DEBUG, "(%d,%d) Blitter bbusy\n", agnus.pos.v, agnus.pos.h);

    // Clear the Blitter busy flag
    bbusy = false;
}

void
Blitter::endBlit()
{
    debug(BLTTIM_DEBUG, "(%d,%d) Blitter terminates\n", agnus.pos.v, agnus.pos.h);
    
    running = false;
    
    if (BLT_GUARD) memset(memguard, 0, sizeof(memguard));
    
    // Clear the Blitter slot
    agnus.cancel<BLT_SLOT>();

    // Dump checksums if requested
    if (BLT_CHECKSUM) {
        debug("check1: %x check2: %x ABCD: %x %x %x %x\n",
                   check1, check2,
                   bltapt & agnus.ptrMask,
                   bltbpt & agnus.ptrMask,
                   bltcpt & agnus.ptrMask,
                   bltdpt & agnus.ptrMask);
        /*
        debug("Memory: %x (%x)\n",
                   fnv_1a_32(mem.chip, mem.chipRamSize()),
                   fnv_1a_32(mem.slow, mem.slowRamSize()));
        */
    }
    
    // Let the Copper know about the termination
    copper.blitterDidTerminate();
}
