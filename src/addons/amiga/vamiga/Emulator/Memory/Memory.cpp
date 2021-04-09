// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Memory.h"
#include "Amiga.h"
#include "Agnus.h"
#include "Checksum.h"
#include "CIA.h"
#include "CPU.h"
#include "Denise.h"
#include "ExtendedRomFile.h"
#include "MsgQueue.h"
#include "Paula.h"
#include "RomFile.h"
#include "RTC.h"
#include "ZorroManager.h"

Memory::Memory(Amiga& ref) : AmigaComponent(ref)
{
    memset(&config, 0, sizeof(config));

    config.slowRamDelay   = true;
    config.bankMap        = BANK_MAP_A500;
    config.ramInitPattern = RAM_INIT_ALL_ZEROES;
    config.unmappingType  = UNMAPPED_FLOATING;
    config.extStart       = 0xE0;
}

Memory::~Memory()
{
    dealloc();
}

void
Memory::dealloc()
{
    if (rom) { delete[] rom; rom = nullptr; }
    if (wom) { delete[] wom; wom = nullptr; }
    if (ext) { delete[] ext; ext = nullptr; }
    if (chip) { delete[] chip; chip = nullptr; }
    if (slow) { delete[] slow; slow = nullptr; }
    if (fast) { delete[] fast; fast = nullptr; }
}

void
Memory::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    // Set up the memory lookup table
    updateMemSrcTables();
    
    // In hard-reset mode, we also initialize RAM
    if (hard) fillRamWithInitPattern();
}

long
Memory::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_CHIP_RAM:          return config.chipSize / KB(1);
        case OPT_SLOW_RAM:          return config.slowSize / KB(1);
        case OPT_FAST_RAM:          return config.fastSize / KB(1);
        case OPT_EXT_START:         return config.extStart;
        case OPT_SLOW_RAM_DELAY:    return config.slowRamDelay;
        case OPT_BANKMAP:           return config.bankMap;
        case OPT_UNMAPPING_TYPE:    return config.unmappingType;
        case OPT_RAM_INIT_PATTERN:  return config.ramInitPattern;

        default:
            assert(false);
            return 0;
    }
}

bool
Memory::setConfigItem(Option option, long value)
{
    switch (option) {
            
        case OPT_CHIP_RAM:
            
            if (isPoweredOn()) throw ConfigLockedError();

            #ifdef FORCE_CHIP_RAM
            value = FORCE_CHIP_RAM;
            warn("Overriding Chip Ram size: %ld KB\n", value);
            #endif
            
            if (value != 256 && value != 512 && value != 1024 && value != 2048) {
                throw ConfigArgError("256, 512, 1024, 2048");
            }
            
            mem.allocChip((i32)KB(value));
            return true;
            
        case OPT_SLOW_RAM:
            
            if (isPoweredOn()) throw ConfigLockedError();
            
            #ifdef FORCE_SLOW_RAM
            value = FORCE_SLOW_RAM;
            warn("Overriding Slow Ram size: %ld KB\n", value);
            #endif
            
            if ((value % 256) != 0 || value > 512) {
                throw ConfigArgError("0, 256, 512");
            }
                        
            mem.allocSlow((i32)KB(value));
            return true;
            
        case OPT_FAST_RAM:
            
            if (isPoweredOn()) throw ConfigLockedError();
            
            #ifdef FORCE_FAST_RAM
            value = FORCE_FAST_RAM;
            warn("Overriding Fast Ram size: %ld KB\n", value);
            #endif
            
            if ((value % 64) != 0 || value > 8192) {
                throw ConfigArgError("0, 64, 128, ..., 8192");
            }
                        
            mem.allocFast((i32)KB(value));
            return true;
            
        case OPT_EXT_START:
            
            if (isPoweredOn()) throw ConfigLockedError();
            
            if (value != 0xE0 && value != 0xF0) {
                throw ConfigArgError("E0, F0");
            }
            
            config.extStart = (u32)value;
            updateMemSrcTables();
            return true;
            
        case OPT_SLOW_RAM_DELAY:
            
            if (config.slowRamDelay == value) {
                return false;
            }
            
            suspend();
            config.slowRamDelay = value;
            resume();
            return true;
            
        case OPT_BANKMAP:
            
            if (!BankMapEnum::isValid(value)) {
                throw ConfigArgError(BankMapEnum::keyList());
            }
            if (config.bankMap == value) {
                return false;
            }
            
            suspend();
            config.bankMap = (BankMap)value;
            updateMemSrcTables();
            resume();
            return true;

        case OPT_UNMAPPING_TYPE:
            
            if (!UnmappedMemoryEnum::isValid(value)) {
                throw ConfigArgError(UnmappedMemoryEnum::keyList());
            }
            if (config.unmappingType == value) {
                return false;
            }
            
            suspend();
            config.unmappingType = (UnmappedMemory)value;
            resume();
            return true;
            
        case OPT_RAM_INIT_PATTERN:
            
            if (!RamInitPatternEnum::isValid(value)) {
                throw ConfigArgError(RamInitPatternEnum::keyList());
            }
            if (config.ramInitPattern == value) {
                return false;
            }

            suspend();
            config.ramInitPattern = (RamInitPattern)value;
            resume();
            if (isPoweredOff()) fillRamWithInitPattern();
            return true;
            
        default:
            return false;
    }
}

isize
Memory::_size()
{
    util::SerCounter counter;

    applyToPersistentItems(counter);
    applyToHardResetItems(counter);
    applyToResetItems(counter);
    
    counter
    << config.romSize
    << config.womSize
    << config.extSize
    << config.chipSize
    << config.slowSize
    << config.fastSize;
    
    counter.count += config.romSize;
    counter.count += config.womSize;
    counter.count += config.extSize;
    counter.count += config.chipSize;
    counter.count += config.slowSize;
    counter.count += config.fastSize;

    return counter.count;
}

isize
Memory::didLoadFromBuffer(const u8 *buffer)
{
    util::SerReader reader(buffer);

    // Load memory size information
    reader
    << config.romSize
    << config.womSize
    << config.extSize
    << config.chipSize
    << config.slowSize
    << config.fastSize;

    // Make sure that corrupted values do not cause any damage
    if (config.romSize > KB(512)) { config.romSize = 0; assert(false); }
    if (config.womSize > KB(256)) { config.womSize = 0; assert(false); }
    if (config.extSize > KB(512)) { config.extSize = 0; assert(false); }
    if (config.chipSize > MB(2)) { config.chipSize = 0; assert(false); }
    if (config.slowSize > KB(512)) { config.slowSize = 0; assert(false); }
    if (config.fastSize > MB(8)) { config.fastSize = 0; assert(false); }

    // Free previously allocated memory
    dealloc();

    // Allocate new memory
    if (config.romSize) rom = new (std::nothrow) u8[config.romSize];
    if (config.womSize) wom = new (std::nothrow) u8[config.womSize];
    if (config.extSize) ext = new (std::nothrow) u8[config.extSize];
    if (config.chipSize) chip = new (std::nothrow) u8[config.chipSize];
    if (config.slowSize) slow = new (std::nothrow) u8[config.slowSize];
    if (config.fastSize) fast = new (std::nothrow) u8[config.fastSize];

    // Load memory contents from buffer
    reader.copy(rom, config.romSize);
    reader.copy(wom, config.womSize);
    reader.copy(ext, config.extSize);
    reader.copy(chip, config.chipSize);
    reader.copy(slow, config.slowSize);
    reader.copy(fast, config.fastSize);

    return (isize)(reader.ptr - buffer);
}

isize
Memory::didSaveToBuffer(u8 *buffer) const
{
    // Save memory size information
    util::SerWriter writer(buffer);
    writer
    << config.romSize
    << config.womSize
    << config.extSize
    << config.chipSize
    << config.slowSize
    << config.fastSize;
    
    // Save memory contents
    writer.copy(rom, config.romSize);
    writer.copy(wom, config.womSize);
    writer.copy(ext, config.extSize);
    writer.copy(chip, config.chipSize);
    writer.copy(slow, config.slowSize);
    writer.copy(fast, config.fastSize);
    
    return (isize)(writer.ptr - buffer);
}

void
Memory::_dump(Dump::Category category, std::ostream& os) const
{
    if (category & Dump::Config) {
        
        os << DUMP("Chip Ram") << std::dec << config.chipSize / 1024 << " KB" << std::endl;
        os << DUMP("Slow Ram") << std::dec << config.slowSize / 1024 << " KB" << std::endl;
        os << DUMP("Fast Ram") << std::dec << config.fastSize / 1024 << " KB" << std::endl;
        os << DUMP("Rom") << std::dec << config.romSize / 1024 << " KB" << std::endl;
        os << DUMP("Wom") << std::dec << config.womSize / 1024 << " KB" << std::endl;
        os << DUMP("Rom extension") << std::dec << config.extSize / 1024 << " KB";
        os << " at " << std::hex << config.extStart << "0000" << std::endl;
        os << std::endl;
        os << DUMP("Emulate Slow Ram delay");
        os << YESNO(config.slowRamDelay) << std::endl;
        os << DUMP("Bank mapping scheme");
        os << BankMapEnum::key(config.bankMap) << std::endl;
        os << DUMP("Ram init pattern");
        os << RamInitPatternEnum::key(config.ramInitPattern) << std::endl;
        os << DUMP("Unmapped memory");
        os << UnmappedMemoryEnum::key(config.unmappingType) << std::endl;
    }
    
    if (category & Dump::State) {

        os << DUMP("Data bus") << HEX16 << dataBus << std::endl;
        os << DUMP("Wom is locked") << YESNO(womIsLocked) << std::endl;
    }
    
    if (category & Dump::Checksums) {

        os << DUMP("Rom checksum");
        os << HEX32 << util::fnv_1a_32(rom, config.romSize) << std::endl;
        os << DUMP("Wom checksum");
        os << HEX32 << util::fnv_1a_32(wom, config.womSize) << std::endl;
        os << DUMP("Extended Rom checksum");
        os << HEX32 << util::fnv_1a_32(ext, config.extSize) << std::endl;
        os << DUMP("Chip Ram checksum");
        os << HEX32 << util::fnv_1a_32(chip, config.chipSize) << std::endl;
        os << DUMP("Slow Ram checksum");
        os << HEX32 << util::fnv_1a_32(slow, config.slowSize) << std::endl;
        os << DUMP("Fast Ram checksum");
        os << HEX32 << util::fnv_1a_32(fast, config.fastSize) << std::endl;
    }
    
    if (category & Dump::BankMap) {

        MemorySource oldsrc = cpuMemSrc[0]; isize oldi = 0;
        for (isize i = 0; i <= 0x100; i++) {
            MemorySource newsrc = i < 0x100 ? cpuMemSrc[i] : (MemorySource)-1;
            if (oldsrc != newsrc) {
                os << HEX8 << oldi << " - " << HEX8 << i - 1 << " : ";
                os << MemorySourceEnum::key(oldsrc) << std::endl;
                oldsrc = newsrc; oldi = i;
            }
        }
    }
}

void
Memory::_powerOn()
{
    // Erase WOM (if any)
    if (hasWom()) eraseWom();

    // Fill RAM with the proper startup pattern
    fillRamWithInitPattern();

    // Set up the memory lookup table
    updateMemSrcTables();
}

void
Memory::updateStats()
{
    const double w = 0.5;
    
    stats.chipReads.accumulated =
    w * stats.chipReads.accumulated + (1.0 - w) * stats.chipReads.raw;
    stats.chipWrites.accumulated =
    w * stats.chipWrites.accumulated + (1.0 - w) * stats.chipWrites.raw;
    stats.slowReads.accumulated =
    w * stats.slowReads.accumulated + (1.0 - w) * stats.slowReads.raw;
    stats.slowWrites.accumulated =
    w * stats.slowWrites.accumulated + (1.0 - w) * stats.slowWrites.raw;
    stats.fastReads.accumulated =
    w * stats.fastReads.accumulated + (1.0 - w) * stats.fastReads.raw;
    stats.fastWrites.accumulated =
    w * stats.fastWrites.accumulated + (1.0 - w) * stats.fastWrites.raw;
    stats.kickReads.accumulated =
    w * stats.kickReads.accumulated + (1.0 - w) * stats.kickReads.raw;
    stats.kickWrites.accumulated =
    w * stats.kickWrites.accumulated + (1.0 - w) * stats.kickWrites.raw;

    stats.chipReads.raw = 0;
    stats.chipWrites.raw = 0;
    stats.slowReads.raw = 0;
    stats.slowWrites.raw = 0;
    stats.fastReads.raw = 0;
    stats.fastWrites.raw = 0;
    stats.kickReads.raw = 0;
    stats.kickWrites.raw = 0;
}

bool
Memory::alloc(i32 bytes, u8 *&ptr, i32 &size, u32 &mask)
{
    // Check the invariants
    assert((ptr == nullptr) == (size == 0));
    assert((ptr == nullptr) == (mask == 0));
    assert((ptr == nullptr) || (mask == (u32)size - 1));

    // Only proceed if memory layout changes
    if (bytes == size) return true;
    
    // Delete previous allocation
    if (ptr) { delete[] ptr; ptr = nullptr; size = 0; mask = 0; }
    
    // Allocate memory
    if (bytes) {
        
        isize allocSize = bytes;
        
        if (!(ptr = new (std::nothrow) u8[allocSize])) {
            warn("Cannot allocate %d KB of memory\n", bytes);
            return false;
        }
        size = (u32)bytes;
        mask = size - 1;
        fillRamWithInitPattern();
        
        if ((uintptr_t)ptr & 1) {
            warn("Memory at %p (%d bytes) is not aligned\n", ptr, bytes);
            assert(false);
        }
    }
    updateMemSrcTables();
    return true;
}

void
Memory::fillRamWithInitPattern()
{
    assert(!isRunning());
    
    switch (config.ramInitPattern) {
            
        case RAM_INIT_RANDOMIZED:

            srand(0);
            if (chip) for (isize i = 0; i < config.chipSize; i++) chip[i] = rand();
            if (slow) for (isize i = 0; i < config.slowSize; i++) slow[i] = rand();
            if (fast) for (isize i = 0; i < config.fastSize; i++) fast[i] = rand();
            break;
            
        case RAM_INIT_ALL_ZEROES:

            if (chip) memset(chip, 0x00, config.chipSize);
            if (slow) memset(slow, 0x00, config.slowSize);
            if (fast) memset(fast, 0x00, config.fastSize);
            break;
            
        case RAM_INIT_ALL_ONES:
            
            if (chip) memset(chip, 0xFF, config.chipSize);
            if (slow) memset(slow, 0xFF, config.slowSize);
            if (fast) memset(fast, 0xFF, config.fastSize);
            break;
            
        default:
            assert(false);
    }
}

u32
Memory::romFingerprint()
{
    return util::crc32(rom, config.romSize);
}

u32
Memory::extFingerprint()
{
    return util::crc32(ext, config.extSize);
}

RomIdentifier
Memory::romIdentifier()
{
    return RomFile::identifier(romFingerprint());
}

RomIdentifier
Memory::extIdentifier()
{
    return RomFile::identifier(extFingerprint());
}

const char *
Memory::romTitle()
{
    return RomFile::title(romIdentifier());
}

const char *
Memory::romVersion()
{
    static char str[32];

    if (romIdentifier() == ROM_UNKNOWN) {
        sprintf(str, "CRC %x", romFingerprint());
        return str;
    }

    return RomFile::version(romIdentifier());
}

const char *
Memory::romReleased()
{
    return RomFile::released(romIdentifier());
}

const char *
Memory::extTitle()
{
    return RomFile::title(extIdentifier());
}

const char *
Memory::extVersion()
{
    static char str[32];

    if (extIdentifier() == ROM_UNKNOWN) {
        sprintf(str, "CRC %x", extFingerprint());
        return str;
    }

    return RomFile::version(extIdentifier());
}

const char *
Memory::extReleased()
{
    return RomFile::released(extIdentifier());
}

bool
Memory::hasArosRom()
{
    return RomFile::isArosRom(romIdentifier());
}

void
Memory::loadRom(RomFile *file)
{
    assert(file);

    // Decrypt Rom
    file->decrypt();

    // Allocate memory
    if (!allocRom((i32)file->size)) throw VAError(ERROR_OUT_OF_MEMORY);
    
    // Load Rom
    loadRom(file, rom, config.romSize);

    // Add a Wom if a Boot Rom is installed instead of a Kickstart Rom
    hasBootRom() ? (void)allocWom(KB(256)) : deleteWom();

    // Remove extended Rom (if any)
    deleteExt();
}

void
Memory::loadRomFromFile(const char *path)
{
    assert(path);
    
    RomFile *file = AmigaFile::make <RomFile> (path);
    loadRom(file);
}

void
Memory::loadRomFromFile(const char *path, ErrorCode *ec)
{
    *ec = ERROR_OK;
    try { loadRomFromFile(path); }
    catch (VAError &err) { *ec = err.data; }
}

void
Memory::loadRomFromBuffer(const u8 *buf, isize len)
{
    assert(buf);
    
    RomFile *file = AmigaFile::make <RomFile> (buf, len);
    loadRom(file);
}

void
Memory::loadRomFromBuffer(const u8 *buf, isize len, ErrorCode *ec)
{
    *ec = ERROR_OK;
    try { loadRomFromBuffer(buf, len); }
    catch (VAError &err) { *ec = err.data; }
}

void
Memory::loadExt(ExtendedRomFile *file)
{
    assert(file);

    // Allocate memory
    if (!allocExt((i32)file->size)) throw VAError(ERROR_OUT_OF_MEMORY);
    
    // Load Rom
    loadRom(file, ext, config.extSize);
}

void
Memory::loadExtFromFile(const char *path)
{
    assert(path);
    
    ExtendedRomFile *file = AmigaFile::make <ExtendedRomFile> (path);
    loadExt(file);
}

void
Memory::loadExtFromFile(const char *path, ErrorCode *ec)
{
    *ec = ERROR_OK;
    try { loadExtFromFile(path); }
    catch (VAError &exception) { *ec = exception.data; }
}

void
Memory::loadExtFromBuffer(const u8 *buf, isize len)
{
    assert(buf);
    
    ExtendedRomFile *file = AmigaFile::make <ExtendedRomFile> (buf, len);
    loadExt(file);
}

void
Memory::loadExtFromBuffer(const u8 *buf, isize len, ErrorCode *ec)
{
    *ec = ERROR_OK;
    try { loadExtFromBuffer(buf, len); }
    catch (VAError &exception) { *ec = exception.data; }
}
 
void
Memory::loadRom(AmigaFile *file, u8 *target, isize length)
{
    if (file) {

        assert(target != nullptr);
        memset(target, 0, length);

        if (file->size < length) {
            warn("ROM is smaller than buffer\n");
        }
        
        memcpy(target, file->data, std::min(file->size, length));
    }
}

void
Memory::saveRom(const char *path)
{
    if (rom == nullptr) return;
    
    RomFile *file = AmigaFile::make <RomFile> (rom, config.romSize);
    file->writeToFile(path);
}

void
Memory::saveRom(const char *path, ErrorCode *ec)
{
    *ec = ERROR_OK;
    try { saveRom(path); } catch (VAError &err) { *ec = err.data; }
}

void
Memory::saveWom(const char *path)
{
    if (wom == nullptr) return;
    
    RomFile *file = AmigaFile::make <RomFile> (wom, config.womSize);
    file->writeToFile(path);
}

void
Memory::saveWom(const char *path, ErrorCode *ec)
{
    *ec = ERROR_OK;
    try { saveWom(path); } catch (VAError &err) { *ec = err.data; }
}

void
Memory::saveExt(const char *path)
{
    if (ext == nullptr) return;

    RomFile *file = AmigaFile::make <RomFile> (ext, config.extSize);
    file->writeToFile(path);
}

void
Memory::saveExt(const char *path, ErrorCode *ec)
{
    *ec = ERROR_OK;
    try { saveExt(path); } catch (VAError &err) { *ec = err.data; }
}

template <> MemorySource
Memory::getMemSrc <ACCESSOR_CPU> (u32 addr)
{
    return cpuMemSrc[(addr >> 16) & 0xFF];
}

template <> MemorySource
Memory::getMemSrc <ACCESSOR_AGNUS> (u32 addr)
{
    return agnusMemSrc[(addr >> 16) & 0xFF];
}

void
Memory::updateMemSrcTables()
{
    updateCpuMemSrcTable();
    updateAgnusMemSrcTable();
}

void
Memory::updateCpuMemSrcTable()
{
    MemorySource mem_rom = rom ? MEM_ROM : MEM_NONE;
    MemorySource mem_wom = wom ? MEM_WOM : mem_rom;
    MemorySource mem_rom_mirror = rom ? MEM_ROM_MIRROR : MEM_NONE;

    isize chipRamPages = config.chipSize / 0x10000;
    isize slowRamPages = config.slowSize / 0x10000;
    isize fastRamPages = config.fastSize / 0x10000;
    
    assert(config.chipSize % 0x10000 == 0);
    assert(config.slowSize % 0x10000 == 0);
    assert(config.fastSize % 0x10000 == 0);

    bool ovl = ciaa.getPA() & 1;
    bool old = config.bankMap == BANK_MAP_A1000 || config.bankMap == BANK_MAP_A2000A;

    // Start from scratch
    for (isize i = 0x00; i <= 0xFF; i++) {
        cpuMemSrc[i] = MEM_NONE;
    }
    
    // Chip Ram
    if (chipRamPages) {
        for (isize i = 0x00; i < chipRamPages; i++) {
            cpuMemSrc[i] = MEM_CHIP;
        }
        for (isize i = chipRamPages; i <= 0x1F; i++) {
            cpuMemSrc[i] = MEM_CHIP_MIRROR;
        }
    }
        
    // Fast Ram
    for (isize i = 0x20; i < 0x20 + fastRamPages; i++) {
        cpuMemSrc[i] = MEM_FAST;
    }
    
    // CIAs
    for (isize i = 0xA0; i <= 0xBE; i++) {
        cpuMemSrc[i] = MEM_CIA_MIRROR;
    }
    cpuMemSrc[0xBF] = MEM_CIA;
    
    // Slow Ram
    for (isize i = 0xC0; i <= 0xD7; i++) {
        cpuMemSrc[i] = (i - 0xC0) < slowRamPages ? MEM_SLOW : MEM_CUSTOM_MIRROR;
    }
    
    // Real-time clock (older Amigas)
    for (isize i = 0xD8; i <= 0xDB; i++) {
        cpuMemSrc[i] = old ? MEM_RTC : MEM_CUSTOM;
    }

    // Real-time clock (newer Amigas)
    cpuMemSrc[0xDC] = old ? MEM_CUSTOM : MEM_RTC;
    
    
    // Reserved
    cpuMemSrc[0xDD] = MEM_NONE;

    // Custom chip set
    for (isize i = 0xDE; i <= 0xDF; i++) {
        cpuMemSrc[i] = MEM_CUSTOM;
    }
    
    // Kickstart mirror, unmapped, or Extended Rom
    if (config.bankMap != BANK_MAP_A1000) {
        for (isize i = 0xE0; i <= 0xE7; i++) {
            cpuMemSrc[i] = mem_rom_mirror;
        }
    }
    if (ext && config.extStart == 0xE0) {
        for (isize i = 0xE0; i <= 0xE7; i++) {
            cpuMemSrc[i] = MEM_EXT;
        }
    }

    // Auto-config (Zorro II)
    cpuMemSrc[0xE8] = MEM_AUTOCONF;
    for (isize i = 0xE9; i <= 0xEF; i++) {
        assert(cpuMemSrc[i] == MEM_NONE);
    }
    
    // Unmapped or Extended Rom
    if (ext && config.extStart == 0xF0) {
        for (isize i = 0xF0; i <= 0xF7; i++) {
            cpuMemSrc[i] = MEM_EXT;
        }
    }

    // Kickstart Wom or Kickstart Rom
    for (isize i = 0xF8; i <= 0xFF; i++) {
        cpuMemSrc[i] = mem_wom;
    }
    
    // Blend in Boot Rom if a writeable Wom is present
    if (hasWom() && !womIsLocked) {
        for (isize i = 0xF8; i <= 0xFB; i++)
            cpuMemSrc[i] = mem_rom;
    }

    // Blend in Rom in lower memory area if the overlay line (OVL) is high
    if (ovl) {
        for (isize i = 0; i < 8 && cpuMemSrc[0xF8 + i] != MEM_NONE; i++)
            cpuMemSrc[i] = cpuMemSrc[0xF8 + i];
    }

    messageQueue.put(MSG_MEM_LAYOUT);
}

void
Memory::updateAgnusMemSrcTable()
{
    isize banks = config.chipSize / 0x10000;
    
    // Start from scratch
    for (isize i = 0x00; i <= 0xFF; i++) {
        agnusMemSrc[i] = MEM_NONE;
    }
    
    // Chip Ram banks
    for (isize i = 0x0; i < banks; i++) {
        agnusMemSrc[i] = MEM_CHIP;
    }
    
    // Slow Ram mirror
    if (agnus.slowRamIsMirroredIn()) {
        for (isize i = 0x8; i <= 0xF; i++) {
            agnusMemSrc[i] = MEM_SLOW_MIRROR;
        }
    }
}

//
// Peek (CPU)
//

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_NONE> (u32 addr) const
{
    switch (config.unmappingType) {
            
        case UNMAPPED_FLOATING:   return dataBus;
        case UNMAPPED_ALL_ONES:   return 0xFFFF;
        case UNMAPPED_ALL_ZEROES: return 0x0000;

        default: assert(false); return 0;
    }
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_NONE> (u32 addr)
{
    return (u8)const_cast<Memory *>(this)->spypeek16 <ACCESSOR_CPU, MEM_NONE> (addr);
}

template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_NONE> (u32 addr)
{
    return const_cast<Memory *>(this)->spypeek16 <ACCESSOR_CPU, MEM_NONE> (addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_CHIP> (u32 addr)
{
    ASSERT_CHIP_ADDR(addr);
    agnus.executeUntilBusIsFree();
    
    stats.chipReads.raw++;
    dataBus = READ_CHIP_8(addr);
    return dataBus;
}
    
template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_CHIP> (u32 addr)
{
    ASSERT_CHIP_ADDR(addr);
    agnus.executeUntilBusIsFree();
    
    stats.chipReads.raw++;
    dataBus = READ_CHIP_16(addr);
    return dataBus;
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_CHIP> (u32 addr) const
{
    return READ_CHIP_16(addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_SLOW> (u32 addr)
{
    ASSERT_SLOW_ADDR(addr);
    agnus.executeUntilBusIsFree();
    
    stats.slowReads.raw++;
    dataBus = READ_SLOW_8(addr);
    return dataBus;
}
    
template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_SLOW> (u32 addr)
{
    ASSERT_SLOW_ADDR(addr);
    agnus.executeUntilBusIsFree();
    
    stats.slowReads.raw++;
    dataBus = READ_SLOW_16(addr);
    return dataBus;
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_SLOW> (u32 addr) const
{
    return READ_SLOW_16(addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_FAST> (u32 addr)
{
    ASSERT_FAST_ADDR(addr);
    
    stats.fastReads.raw++;
    return READ_FAST_8(addr);
}

template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_FAST> (u32 addr)
{
    ASSERT_FAST_ADDR(addr);
    
    stats.fastReads.raw++;
    return READ_FAST_16(addr);
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_FAST> (u32 addr) const
{
    return READ_FAST_16(addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_CIA> (u32 addr)
{
    ASSERT_CIA_ADDR(addr);
    
    agnus.executeUntilBusIsFreeForCIA();

    dataBus = peekCIA8(addr);
    return dataBus;
}

template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_CIA> (u32 addr)
{
    ASSERT_CIA_ADDR(addr);
    trace(XFILES, "XFILES (CIA): Reading a WORD from %x\n", addr);

    agnus.executeUntilBusIsFreeForCIA();
    
    dataBus = peekCIA16(addr);
    return dataBus;
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_CIA> (u32 addr) const
{
    return spypeekCIA16(addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_RTC> (u32 addr)
{
    ASSERT_RTC_ADDR(addr);
    
    // agnus.executeUntilBusIsFree();
    
    dataBus = peekRTC8(addr);
    return dataBus;
}

template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_RTC> (u32 addr)
{
    ASSERT_RTC_ADDR(addr);
    
    // agnus.executeUntilBusIsFree();

    dataBus = peekRTC16(addr);
    return dataBus;
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_RTC> (u32 addr) const
{
    ASSERT_RTC_ADDR(addr);
    return peekRTC16(addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_CUSTOM> (u32 addr)
{
    ASSERT_CUSTOM_ADDR(addr);
            
    agnus.executeUntilBusIsFree();

    if (IS_EVEN(addr)) {
        dataBus = HI_BYTE(peekCustom16(addr));
    } else {
        dataBus = LO_BYTE(peekCustom16(addr & 0x1FE));
    }
    return dataBus;
}

template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_CUSTOM> (u32 addr)
{
    ASSERT_CUSTOM_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = peekCustom16(addr);
    return dataBus;
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_CUSTOM> (u32 addr) const
{
    return spypeekCustom16(addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_AUTOCONF> (u32 addr)
{
    ASSERT_AUTO_ADDR(addr);
    
    // Experimental code to match UAE output (for debugging)
    if (MIMIC_UAE && fastRamSize() == 0) {
        dataBus = (addr & 0b10) ? 0xE8 : 0x02;
        return dataBus;
    }
    
    dataBus = zorro.peekFastRamDevice(addr) << 4;
    trace(FAS_DEBUG, "peek8<AUTOCONF>(%x) = %x\n", addr, dataBus);
    return dataBus;
}

template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_AUTOCONF> (u32 addr)
{
    ASSERT_AUTO_ADDR(addr);
    
    // agnus.executeUntilBusIsFree();
    
    u8 hi = zorro.peekFastRamDevice(addr) << 4;
    u8 lo = zorro.peekFastRamDevice(addr + 1) << 4;
    
    dataBus = HI_LO(hi,lo);
    trace(FAS_DEBUG, "peek16<AUTOCONF>(%x) = %x\n", addr, dataBus);
    return dataBus;
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_AUTOCONF> (u32 addr) const
{
    u8 hi = zorro.spypeekFastRamDevice(addr) << 4;
    u8 lo = zorro.spypeekFastRamDevice(addr + 1) << 4;
    
    return HI_LO(hi,lo);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_ROM> (u32 addr)
{
    ASSERT_ROM_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_ROM_8(addr);
}

template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_ROM> (u32 addr)
{
    ASSERT_ROM_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_ROM_16(addr);
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_ROM> (u32 addr) const
{
    return READ_ROM_16(addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_WOM> (u32 addr)
{
    ASSERT_WOM_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_WOM_8(addr);
}

template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_WOM> (u32 addr)
{
    ASSERT_WOM_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_WOM_16(addr);
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_WOM> (u32 addr) const
{
    return READ_WOM_16(addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU, MEM_EXT> (u32 addr)
{
    ASSERT_EXT_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_EXT_8(addr);
}

template<> u16
Memory::peek16 <ACCESSOR_CPU, MEM_EXT> (u32 addr)
{
    ASSERT_EXT_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_EXT_16(addr);
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU, MEM_EXT> (u32 addr) const
{
    return READ_EXT_16(addr);
}

template<> u8
Memory::peek8 <ACCESSOR_CPU> (u32 addr)
{
    u8 result;
        
    switch (cpuMemSrc[(addr & 0xFFFFFF) >> 16]) {
            
        case MEM_NONE:          result = peek8 <ACCESSOR_CPU, MEM_NONE>     (addr); break;
        case MEM_CHIP:          result = peek8 <ACCESSOR_CPU, MEM_CHIP>     (addr); break;
        case MEM_CHIP_MIRROR:   result = peek8 <ACCESSOR_CPU, MEM_CHIP>     (addr); break;
        case MEM_SLOW:          result = peek8 <ACCESSOR_CPU, MEM_SLOW>     (addr); break;
        case MEM_SLOW_MIRROR:   result = peek8 <ACCESSOR_CPU, MEM_SLOW>     (addr); break;
        case MEM_FAST:          result = peek8 <ACCESSOR_CPU, MEM_FAST>     (addr); break;
        case MEM_CIA:           result = peek8 <ACCESSOR_CPU, MEM_CIA>      (addr); break;
        case MEM_CIA_MIRROR:    result = peek8 <ACCESSOR_CPU, MEM_CIA>      (addr); break;
        case MEM_RTC:           result = peek8 <ACCESSOR_CPU, MEM_RTC>      (addr); break;
        case MEM_CUSTOM:        result = peek8 <ACCESSOR_CPU, MEM_CUSTOM>   (addr); break;
        case MEM_CUSTOM_MIRROR: result = peek8 <ACCESSOR_CPU, MEM_CUSTOM>   (addr); break;
        case MEM_AUTOCONF:      result = peek8 <ACCESSOR_CPU, MEM_AUTOCONF> (addr); break;
        case MEM_ROM:           result = peek8 <ACCESSOR_CPU, MEM_ROM>      (addr); break;
        case MEM_ROM_MIRROR:    result = peek8 <ACCESSOR_CPU, MEM_ROM>      (addr); break;
        case MEM_WOM:           result = peek8 <ACCESSOR_CPU, MEM_WOM>      (addr); break;
        case MEM_EXT:           result = peek8 <ACCESSOR_CPU, MEM_EXT>      (addr); break;
            
        default: assert(false); return 0;
    }
        
    return result;
}

template<> u16
Memory::peek16 <ACCESSOR_CPU> (u32 addr)
{
    u16 result;
    
    assert(IS_EVEN(addr));
    
    switch (cpuMemSrc[(addr & 0xFFFFFF) >> 16]) {
            
        case MEM_NONE:          result = peek16 <ACCESSOR_CPU, MEM_NONE>     (addr); break;
        case MEM_CHIP:          result = peek16 <ACCESSOR_CPU, MEM_CHIP>     (addr); break;
        case MEM_CHIP_MIRROR:   result = peek16 <ACCESSOR_CPU, MEM_CHIP>     (addr); break;
        case MEM_SLOW:          result = peek16 <ACCESSOR_CPU, MEM_SLOW>     (addr); break;
        case MEM_SLOW_MIRROR:   result = peek16 <ACCESSOR_CPU, MEM_SLOW>     (addr); break;
        case MEM_FAST:          result = peek16 <ACCESSOR_CPU, MEM_FAST>     (addr); break;
        case MEM_CIA:           result = peek16 <ACCESSOR_CPU, MEM_CIA>      (addr); break;
        case MEM_CIA_MIRROR:    result = peek16 <ACCESSOR_CPU, MEM_CIA>      (addr); break;
        case MEM_RTC:           result = peek16 <ACCESSOR_CPU, MEM_RTC>      (addr); break;
        case MEM_CUSTOM:        result = peek16 <ACCESSOR_CPU, MEM_CUSTOM>   (addr); break;
        case MEM_CUSTOM_MIRROR: result = peek16 <ACCESSOR_CPU, MEM_CUSTOM>   (addr); break;
        case MEM_AUTOCONF:      result = peek16 <ACCESSOR_CPU, MEM_AUTOCONF> (addr); break;
        case MEM_ROM:           result = peek16 <ACCESSOR_CPU, MEM_ROM>      (addr); break;
        case MEM_ROM_MIRROR:    result = peek16 <ACCESSOR_CPU, MEM_ROM>      (addr); break;
        case MEM_WOM:           result = peek16 <ACCESSOR_CPU, MEM_WOM>      (addr); break;
        case MEM_EXT:           result = peek16 <ACCESSOR_CPU, MEM_EXT>      (addr); break;
            
        default: assert(false); return 0;
    }
        
    return result;
}

template<> u16
Memory::spypeek16 <ACCESSOR_CPU> (u32 addr) const
{
    assert(IS_EVEN(addr));

    auto src = cpuMemSrc[(addr & 0xFFFFFF) >> 16];
        
    switch (src) {
            
        case MEM_NONE:          return spypeek16 <ACCESSOR_CPU, MEM_NONE>     (addr);
        case MEM_CHIP:          return spypeek16 <ACCESSOR_CPU, MEM_CHIP>     (addr);
        case MEM_CHIP_MIRROR:   return spypeek16 <ACCESSOR_CPU, MEM_CHIP>     (addr);
        case MEM_SLOW:          return spypeek16 <ACCESSOR_CPU, MEM_SLOW>     (addr);
        case MEM_SLOW_MIRROR:   return spypeek16 <ACCESSOR_CPU, MEM_SLOW>     (addr);
        case MEM_FAST:          return spypeek16 <ACCESSOR_CPU, MEM_FAST>     (addr);
        case MEM_CIA:           return spypeek16 <ACCESSOR_CPU, MEM_CIA>      (addr);
        case MEM_CIA_MIRROR:    return spypeek16 <ACCESSOR_CPU, MEM_CIA>      (addr);
        case MEM_RTC:           return spypeek16 <ACCESSOR_CPU, MEM_RTC>      (addr);
        case MEM_CUSTOM:        return spypeek16 <ACCESSOR_CPU, MEM_CUSTOM>   (addr);
        case MEM_CUSTOM_MIRROR: return spypeek16 <ACCESSOR_CPU, MEM_CUSTOM>   (addr);
        case MEM_AUTOCONF:      return spypeek16 <ACCESSOR_CPU, MEM_AUTOCONF> (addr);
        case MEM_ROM:           return spypeek16 <ACCESSOR_CPU, MEM_ROM>      (addr);
        case MEM_ROM_MIRROR:    return spypeek16 <ACCESSOR_CPU, MEM_ROM>      (addr);
        case MEM_WOM:           return spypeek16 <ACCESSOR_CPU, MEM_WOM>      (addr);
        case MEM_EXT:           return spypeek16 <ACCESSOR_CPU, MEM_EXT>      (addr);
            
        default: assert(false); return 0;
    }
}


//
// Peek (Agnus)
//

template<> u16
Memory::peek16 <ACCESSOR_AGNUS, MEM_NONE> (u32 addr)
{
    assert((addr & agnus.ptrMask) == addr);
    trace(XFILES, "XFILES (AGNUS): Reading from unmapped RAM\n");
    return peek16 <ACCESSOR_CPU, MEM_NONE> (addr);
}

template<> u16
Memory::spypeek16 <ACCESSOR_AGNUS, MEM_NONE> (u32 addr) const
{
    return spypeek16 <ACCESSOR_CPU, MEM_NONE> (addr);
}

template<> u16
Memory::peek16 <ACCESSOR_AGNUS, MEM_CHIP> (u32 addr)
{
    assert((addr & agnus.ptrMask) == addr);
    dataBus = READ_CHIP_16(addr);
    return dataBus;
}

template<> u16
Memory::spypeek16 <ACCESSOR_AGNUS, MEM_CHIP> (u32 addr) const
{
    assert((addr & agnus.ptrMask) == addr);
    return READ_CHIP_16(addr);
}

template<> u16
Memory::peek16 <ACCESSOR_AGNUS, MEM_SLOW> (u32 addr)
{
    assert((addr & agnus.ptrMask) == addr);
    trace(XFILES, "XFILES (AGNUS): Reading from Slow RAM mirror\n");
    dataBus = READ_SLOW_16(addr & 0x7FFFF);
    return dataBus;
}

template<> u16
Memory::spypeek16 <ACCESSOR_AGNUS, MEM_SLOW> (u32 addr) const
{
    assert((addr & agnus.ptrMask) == addr);
    return READ_SLOW_16(addr);
}

template<> u16
Memory::peek16 <ACCESSOR_AGNUS> (u32 addr)
{
    u16 result;
    
    assert(IS_EVEN(addr));
    addr &= agnus.ptrMask;

    switch (agnusMemSrc[addr >> 16]) {
            
        case MEM_NONE:        result = peek16 <ACCESSOR_AGNUS, MEM_NONE> (addr); break;
        case MEM_CHIP:        result = peek16 <ACCESSOR_AGNUS, MEM_CHIP> (addr); break;
        case MEM_SLOW_MIRROR: result = peek16 <ACCESSOR_AGNUS, MEM_SLOW> (addr); break;
            
        default: assert(false); return 0;
    }
        
    return result;
}

template<> u16
Memory::spypeek16 <ACCESSOR_AGNUS> (u32 addr) const
{
    assert(IS_EVEN(addr));
    addr &= agnus.ptrMask;
    
    switch (agnusMemSrc[addr >> 16]) {
            
        case MEM_NONE:        return spypeek16 <ACCESSOR_AGNUS, MEM_NONE> (addr);
        case MEM_CHIP:        return spypeek16 <ACCESSOR_AGNUS, MEM_CHIP> (addr);
        case MEM_SLOW_MIRROR: return spypeek16 <ACCESSOR_AGNUS, MEM_SLOW> (addr);
            
        default: assert(false); return 0;
    }
}


//
// Poke (CPU)
//

template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_NONE> (u32 addr, u8 value)
{
    trace(MEM_DEBUG, "poke8(%x [NONE], %x)\n", addr, value);
    dataBus = value;
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_NONE> (u32 addr, u16 value)
{
    trace(MEM_DEBUG, "poke16 <CPU> (%x [NONE], %x)\n", addr, value);
    dataBus = value;
}

template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_CHIP> (u32 addr, u8 value)
{
    ASSERT_CHIP_ADDR(addr);
    
    trace(BLT_GUARD && blitter.memguard[addr & mem.chipMask],
          "CPU(8) OVERWRITES BLITTER AT ADDR %x\n", addr);

    agnus.executeUntilBusIsFree();
    
    stats.chipWrites.raw++;
    dataBus = value;
    WRITE_CHIP_8(addr, value);
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_CHIP> (u32 addr, u16 value)
{
    ASSERT_CHIP_ADDR(addr);
    
    trace(BLT_GUARD && blitter.memguard[addr & mem.chipMask],
          "CPU OVERWRITES BLITTER AT ADDR %x\n", addr);
    
    agnus.executeUntilBusIsFree();
    
    stats.chipWrites.raw++;
    dataBus = value;
    WRITE_CHIP_16(addr, value);
}
    
template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_SLOW> (u32 addr, u8 value)
{
    ASSERT_SLOW_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    stats.slowWrites.raw++;
    dataBus = value;
    WRITE_SLOW_8(addr, value);
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_SLOW> (u32 addr, u16 value)
{
    ASSERT_SLOW_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    stats.slowWrites.raw++;
    dataBus = value;
    WRITE_SLOW_16(addr, value);
}

template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_FAST> (u32 addr, u8 value)
{
    ASSERT_FAST_ADDR(addr);
    
    stats.fastWrites.raw++;
    WRITE_FAST_8(addr, value);
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_FAST> (u32 addr, u16 value)
{
    ASSERT_FAST_ADDR(addr);
    
    stats.fastWrites.raw++;
    WRITE_FAST_16(addr, value);
}

template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_CIA> (u32 addr, u8 value)
{
    ASSERT_CIA_ADDR(addr);

    agnus.executeUntilBusIsFreeForCIA();
    
    dataBus = value;
    pokeCIA8(addr, value);
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_CIA> (u32 addr, u16 value)
{
    ASSERT_CIA_ADDR(addr);
    trace(XFILES, "XFILES (CIA): Writing a WORD into %x\n", addr);

    agnus.executeUntilBusIsFreeForCIA();
    
    dataBus = value;
    pokeCIA16(addr, value);
}
    
template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_RTC> (u32 addr, u8 value)
{
    ASSERT_RTC_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = value;
    pokeRTC8(addr, value);
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_RTC> (u32 addr, u16 value)
{
    ASSERT_RTC_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = value;
    pokeRTC16(addr, value);
}

template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_CUSTOM> (u32 addr, u8 value)
{
    ASSERT_CUSTOM_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = value;
    // http://eab.abime.net/showthread.php?p=1156399
    pokeCustom16<ACCESSOR_CPU>(addr & 0x1FE, HI_LO(value, value));
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_CUSTOM> (u32 addr, u16 value)
{
    ASSERT_CUSTOM_ADDR(addr);

    agnus.executeUntilBusIsFree();

    dataBus = value;
    pokeCustom16<ACCESSOR_CPU>(addr, value);
}

template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_AUTOCONF> (u32 addr, u8 value)
{
    ASSERT_AUTO_ADDR(addr);
    
    // agnus.executeUntilBusIsFree();
    
    dataBus = value;
    zorro.pokeFastRamDevice(addr, value);
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_AUTOCONF> (u32 addr, u16 value)
{
    ASSERT_AUTO_ADDR(addr);
    
    // agnus.executeUntilBusIsFree();

    dataBus = value;
    zorro.pokeFastRamDevice(addr, HI_BYTE(value));
    zorro.pokeFastRamDevice(addr + 1, LO_BYTE(value));
}

template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_ROM> (u32 addr, u8 value)
{
    ASSERT_ROM_ADDR(addr);
    
    stats.kickWrites.raw++;
    
    // On Amigas with a WOM, writing into ROM space locks the WOM
    if (hasWom() && !womIsLocked) {
        debug(MEM_DEBUG, "Locking WOM\n");
        womIsLocked = true;
        updateMemSrcTables();
    }
        
    if (!releaseBuild) {
        if (addr == 0xFFFFFF && value == 42) {
            msg("DEBUG STOP\n");
            amiga.signalStop();
        }
    }
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_ROM> (u32 addr, u16 value)
{
    poke8 <ACCESSOR_CPU, MEM_ROM> (addr, value);
}

template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_WOM> (u32 addr, u8 value)
{
    ASSERT_WOM_ADDR(addr);
    
    stats.kickWrites.raw++;
    if (!womIsLocked) WRITE_WOM_8(addr, value);
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_WOM> (u32 addr, u16 value)
{
    ASSERT_WOM_ADDR(addr);

    stats.kickWrites.raw++;
    if (!womIsLocked) WRITE_WOM_16(addr, value);
}

template <> void
Memory::poke8 <ACCESSOR_CPU, MEM_EXT> (u32 addr, u8 value)
{
    ASSERT_EXT_ADDR(addr);
    stats.kickWrites.raw++;
}

template <> void
Memory::poke16 <ACCESSOR_CPU, MEM_EXT> (u32 addr, u16 value)
{
    ASSERT_EXT_ADDR(addr);
    stats.kickWrites.raw++;
}

template<> void
Memory::poke8 <ACCESSOR_CPU> (u32 addr, u8 value)
{
    switch (cpuMemSrc[(addr & 0xFFFFFF) >> 16]) {
            
        case MEM_NONE:          poke8 <ACCESSOR_CPU, MEM_NONE>     (addr, value); return;
        case MEM_CHIP:          poke8 <ACCESSOR_CPU, MEM_CHIP>     (addr, value); return;
        case MEM_CHIP_MIRROR:   poke8 <ACCESSOR_CPU, MEM_CHIP>     (addr, value); return;
        case MEM_SLOW:          poke8 <ACCESSOR_CPU, MEM_SLOW>     (addr, value); return;
        case MEM_SLOW_MIRROR:   poke8 <ACCESSOR_CPU, MEM_SLOW>     (addr, value); return;
        case MEM_FAST:          poke8 <ACCESSOR_CPU, MEM_FAST>     (addr, value); return;
        case MEM_CIA:           poke8 <ACCESSOR_CPU, MEM_CIA>      (addr, value); return;
        case MEM_CIA_MIRROR:    poke8 <ACCESSOR_CPU, MEM_CIA>      (addr, value); return;
        case MEM_RTC:           poke8 <ACCESSOR_CPU, MEM_RTC>      (addr, value); return;
        case MEM_CUSTOM:        poke8 <ACCESSOR_CPU, MEM_CUSTOM>   (addr, value); return;
        case MEM_CUSTOM_MIRROR: poke8 <ACCESSOR_CPU, MEM_CUSTOM>   (addr, value); return;
        case MEM_AUTOCONF:      poke8 <ACCESSOR_CPU, MEM_AUTOCONF> (addr, value); return;
        case MEM_ROM:           poke8 <ACCESSOR_CPU, MEM_ROM>      (addr, value); return;
        case MEM_ROM_MIRROR:    poke8 <ACCESSOR_CPU, MEM_ROM>      (addr, value); return;
        case MEM_WOM:           poke8 <ACCESSOR_CPU, MEM_WOM>      (addr, value); return;
        case MEM_EXT:           poke8 <ACCESSOR_CPU, MEM_EXT>      (addr, value); return;
            
        default: assert(false);
    }
}

template<> void
Memory::poke16 <ACCESSOR_CPU> (u32 addr, u16 value)
{
    assert(IS_EVEN(addr));

    /*
    if (addr == 0xC01EC2) {
        trace("poke16 <ACCESSOR_CPU> (%x,%x)\n", addr, value);
        // if (value == 0x302) amiga.signalStop();
    }

    if (addr == 0xC05CF0) { // WRITTEN AT FD1724
        trace("poke16 <ACCESSOR_CPU> (%x,%x)\n", addr, value);
        if (value == 0x302) amiga.signalStop();
    }

    if (addr == 0x010124 - 2) {
        trace("poke16 <ACCESSOR_CPU> (%x,%x)\n", addr, value);
        if (value == 0x302) amiga.signalStop();
    }
    */
    
    switch (cpuMemSrc[(addr & 0xFFFFFF) >> 16]) {
            
        case MEM_NONE:          poke16 <ACCESSOR_CPU, MEM_NONE>     (addr, value); return;
        case MEM_CHIP:          poke16 <ACCESSOR_CPU, MEM_CHIP>     (addr, value); return;
        case MEM_CHIP_MIRROR:   poke16 <ACCESSOR_CPU, MEM_CHIP>     (addr, value); return;
        case MEM_SLOW:          poke16 <ACCESSOR_CPU, MEM_SLOW>     (addr, value); return;
        case MEM_SLOW_MIRROR:   poke16 <ACCESSOR_CPU, MEM_SLOW>     (addr, value); return;
        case MEM_FAST:          poke16 <ACCESSOR_CPU, MEM_FAST>     (addr, value); return;
        case MEM_CIA:           poke16 <ACCESSOR_CPU, MEM_CIA>      (addr, value); return;
        case MEM_CIA_MIRROR:    poke16 <ACCESSOR_CPU, MEM_CIA>      (addr, value); return;
        case MEM_RTC:           poke16 <ACCESSOR_CPU, MEM_RTC>      (addr, value); return;
        case MEM_CUSTOM:        poke16 <ACCESSOR_CPU, MEM_CUSTOM>   (addr, value); return;
        case MEM_CUSTOM_MIRROR: poke16 <ACCESSOR_CPU, MEM_CUSTOM>   (addr, value); return;
        case MEM_AUTOCONF:      poke16 <ACCESSOR_CPU, MEM_AUTOCONF> (addr, value); return;
        case MEM_ROM:           poke16 <ACCESSOR_CPU, MEM_ROM>      (addr, value); return;
        case MEM_ROM_MIRROR:    poke16 <ACCESSOR_CPU, MEM_ROM>      (addr, value); return;
        case MEM_WOM:           poke16 <ACCESSOR_CPU, MEM_WOM>      (addr, value); return;
        case MEM_EXT:           poke16 <ACCESSOR_CPU, MEM_EXT>      (addr, value); return;
            
        default: assert(false);
    }
}

//
// Poke (Agnus)
//

template <> void
Memory::poke16 <ACCESSOR_AGNUS, MEM_NONE> (u32 addr, u16 value)
{
    trace(MEM_DEBUG, "poke16 <AGNUS> (%x [NONE], %x)\n", addr, value);
    trace(XFILES, "XFILES (AGNUS): Writing to unmapped RAM\n");
    dataBus = value;
}

template <> void
Memory::poke16 <ACCESSOR_AGNUS, MEM_CHIP> (u32 addr, u16 value)
{
    assert((addr & agnus.ptrMask) == addr);

    dataBus = value;
    WRITE_CHIP_16(addr, value);
}

template <> void
Memory::poke16 <ACCESSOR_AGNUS, MEM_SLOW> (u32 addr, u16 value)
{
    assert((addr & agnus.ptrMask) == addr);
    trace(XFILES, "XFILES (AGNUS): Writing to Slow RAM mirror\n");

    dataBus = value;
    WRITE_SLOW_16(addr, value);
}

template<> void
Memory::poke16 <ACCESSOR_AGNUS> (u32 addr, u16 value)
{
    assert(IS_EVEN(addr));
    addr &= agnus.ptrMask;
    
    switch (agnusMemSrc[addr >> 16]) {
            
        case MEM_NONE:          poke16 <ACCESSOR_AGNUS, MEM_NONE> (addr, value); return;
        case MEM_CHIP:          poke16 <ACCESSOR_AGNUS, MEM_CHIP> (addr, value); return;
        case MEM_SLOW_MIRROR:   poke16 <ACCESSOR_AGNUS, MEM_SLOW> (addr, value); return;
            
        default: assert(false);
    }
}

u8
Memory::peekCIA8(u32 addr)
{
    // trace("peekCIA8(%6X)\n", addr);
    
    u32 reg = (addr >> 8)  & 0b1111;
    u32 sel = (addr >> 12) & 0b11;
    bool a0 = addr & 1;
    
    switch (sel) {
            
        case 0b00:
            return a0 ? ciaa.peek(reg) : ciab.peek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(cpu.getIRD()) : ciab.peek(reg);
            
        case 0b10:
            return a0 ? ciaa.peek(reg) : HI_BYTE(cpu.getIRD());
            
        case 0b11:
            return a0 ? LO_BYTE(cpu.getIRD()) : HI_BYTE(cpu.getIRD());
    }
    assert(false);
    return 0;
}

u16
Memory::peekCIA16(u32 addr)
{
    // trace(CIA_DEBUG, "peekCIA16(%6X)\n", addr);
    
    u32 reg = (addr >> 8)  & 0b1111;
    u32 sel = (addr >> 12) & 0b11;
    
    switch (sel) {
            
        case 0b00:
            return HI_LO(ciab.peek(reg), ciaa.peek(reg));
            
        case 0b01:
            return HI_LO(ciab.peek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, ciaa.peek(reg));
            
        case 0b11:
            return cpu.getIRD();
            
    }
    assert(false);
    return 0;
}

u8
Memory::spypeekCIA8(u32 addr) const
{
    u32 reg = (addr >> 8)  & 0b1111;
    u32 sel = (addr >> 12) & 0b11;
    bool a0 = addr & 1;
    
    switch (sel) {
            
        case 0b00:
            return a0 ? ciaa.spypeek(reg) : ciab.spypeek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(cpu.getIRD()) : ciab.spypeek(reg);
            
        case 0b10:
            return a0 ? ciaa.spypeek(reg) : HI_BYTE(cpu.getIRD());
            
        case 0b11:
            return a0 ? LO_BYTE(cpu.getIRD()) : HI_BYTE(cpu.getIRD());
    }
    assert(false);
    return 0;
}

u16
Memory::spypeekCIA16(u32 addr) const
{
    u32 reg = (addr >> 8) & 0b1111;
    u32 sel = (addr >> 12) & 0b11;
    
    switch (sel) {
            
        case 0b00:
            return HI_LO(ciab.spypeek(reg), ciaa.spypeek(reg));
            
        case 0b01:
            return HI_LO(ciab.spypeek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, ciaa.spypeek(reg));
            
        case 0b11:
            return cpu.getIRD();
            
    }
    assert(false);
    return 0;
}

void
Memory::pokeCIA8(u32 addr, u8 value)
{
    // trace(CIA_DEBUG, "pokeCIA8(%6X, %X)\n", addr, value);
    
    u32 reg = (addr >> 8) & 0b1111;
    u32 selA = (addr & 0x1000) == 0;
    u32 selB = (addr & 0x2000) == 0;

    if (selA) ciaa.poke(reg, value);
    if (selB) ciab.poke(reg, value);
}

void
Memory::pokeCIA16(u32 addr, u16 value)
{
    // trace(CIA_DEBUG, "pokeCIA16(%6X, %X)\n", addr, value);
    
    assert(IS_EVEN(addr));
    
    u32 reg = (addr >> 8) & 0b1111;
    u32 selA = (addr & 0x1000) == 0;
    u32 selB = (addr & 0x2000) == 0;
    
    if (selA) ciaa.poke(reg, LO_BYTE(value));
    if (selB) ciab.poke(reg, HI_BYTE(value));
}

u8
Memory::peekRTC8(u32 addr) const
{
    /* Addr: 0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011
     * Reg:   --        --        --        --        --        --
     */
    if (IS_EVEN(addr)) return HI_BYTE(dataBus);
    
    /* Addr: 0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011
     * Reg:        00        00        11        11        22        22
     */
    if (rtc.isPresent()) {
        return rtc.peek((addr >> 2) & 0b1111);
    } else {
        return 0x40; // This is the value I've seen on my A500
    }
}

u16
Memory::peekRTC16(u32 addr) const
{
    return HI_LO(peekRTC8(addr), peekRTC8(addr + 1));
}

void
Memory::pokeRTC8(u32 addr, u8 value)
{
    /* Addr: 0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011
     * Reg:   --   -0   --   -0   --   -1   --   -1   --   -2   --   -2
     */
    if (IS_EVEN(addr)) return;
    
    /* Addr: 0001 0011 0101 0111 1001 1011
     * Reg:   -0   -0   -1   -1   -2   -2
     */
    rtc.poke((addr >> 2) & 0b1111, value);
}

void
Memory::pokeRTC16(u32 addr, u16 value)
{
    pokeRTC8(addr, HI_BYTE(value));
    pokeRTC8(addr + 1, LO_BYTE(value));
}

u16
Memory::peekCustom16(u32 addr)
{
    u32 result;

    assert(IS_EVEN(addr));

    switch ((addr >> 1) & 0xFF) {
            
        case 0x000 >> 1: // BLTDDAT
            result = 0xFFFF; break;
        case 0x002 >> 1: // DMACONR
            result = agnus.peekDMACONR(); break;
        case 0x004 >> 1: // VPOSR
            result = agnus.peekVPOSR(); break;
        case 0x006 >> 1: // VHPOSR
            result = agnus.peekVHPOSR(); break;
        case 0x008 >> 1: // DSKDATR
            result = paula.diskController.peekDSKDATR(); break;
        case 0x00A >> 1: // JOY0DAT
            result = denise.peekJOY0DATR(); break;
        case 0x00C >> 1: // JOY1DAT
            result = denise.peekJOY1DATR(); break;
        case 0x00E >> 1: // CLXDAT
            result = denise.peekCLXDAT(); break;
        case 0x010 >> 1: // ADKCONR
            result = paula.peekADKCONR(); break;
        case 0x012 >> 1: // POT0DAT
            result = paula.peekPOTxDAT<0>(); break;
        case 0x014 >> 1: // POT1DAT
            result = paula.peekPOTxDAT<1>(); break;
        case 0x016 >> 1: // POTGOR
            result = paula.peekPOTGOR(); break;
        case 0x018 >> 1: // SERDATR
            result = uart.peekSERDATR(); break;
        case 0x01A >> 1: // DSKBYTR
            result = diskController.peekDSKBYTR(); break;
        case 0x01C >> 1: // INTENAR
            result = paula.peekINTENAR(); break;
        case 0x01E >> 1: // INTREQR
            result = paula.peekINTREQR(); break;
        case 0x07C >> 1: // DENISEID
            result = denise.peekDENISEID(); break;
        default:
            result = peekCustomFaulty16(addr);

    }

    trace(OCSREG_DEBUG, "peekCustom16(%X [%s]) = %X\n", addr, regName(addr), result);

    dataBus = result;
    return result;
}

u16
Memory::peekCustomFaulty16(u32 addr)
{
    /* This functions is called when a write-only register or a
     * non-existing chipset register is read.
     *
     * Derived from the UAE source code documentation:
     *
     * Reading a write-only OCS register causes the last value of the
     * data bus to be written into this registers.
     *
     * Return values:
     *
     * - BLTDDAT (0x000) always returns the last data bus value.
     * - All other registers return
     *   - DMA cycle data (if DMA happened on the bus).
     *   - 0xFFFF or some some ANDed old data otherwise.
     */
            
    pokeCustom16<ACCESSOR_CPU>(addr, dataBus);
    
    return dataBus;
}

u16
Memory::spypeekCustom16(u32 addr) const
{
    assert(IS_EVEN(addr));
    
    switch (addr & 0x1FE) {
            
    }

    return 42;
}

template <Accessor s> void
Memory::pokeCustom16(u32 addr, u16 value)
{

    if ((addr & 0xFFF) == 0x30) {
        trace(OCSREG_DEBUG, "pokeCustom16(SERDAT, '%c')\n", (char)value);
    } else {
        trace(OCSREG_DEBUG, "pokeCustom16(%X [%s], %X)\n", addr, regName(addr), value);
    }

    assert(IS_EVEN(addr));

    dataBus = value;

    switch ((addr >> 1) & 0xFF) {

        case 0x020 >> 1: // DSKPTH
            agnus.pokeDSKPTH(value); return;
        case 0x022 >> 1: // DSKPTL
            agnus.pokeDSKPTL(value); return;
        case 0x024 >> 1: // DSKLEN
            diskController.pokeDSKLEN(value); return;
        case 0x026 >> 1: // DSKDAT
            diskController.pokeDSKDAT(value); return;
        case 0x028 >> 1: // REFPTR
            return;
        case 0x02A >> 1: // VPOSW
            agnus.pokeVPOS(value); return;
        case 0x02C >> 1: // VHPOSW
            agnus.pokeVHPOS(value); return;
        case 0x02E >> 1: // COPCON
            copper.pokeCOPCON(value); return;
        case 0x030 >> 1: // SERDAT
            uart.pokeSERDAT(value); return;
        case 0x032 >> 1: // SERPER
            uart.pokeSERPER(value); return;
        case 0x034 >> 1: // POTGO
            paula.pokePOTGO(value); return;
        case 0x036 >> 1: // JOYTEST
            denise.pokeJOYTEST(value); return;
        case 0x038 >> 1: // STREQU
        case 0x03A >> 1: // STRVBL
        case 0x03C >> 1: // STRHOR
        case 0x03E >> 1: // STRLONG
            trace(XFILES, "XFILES (STROBE): %x\n", addr);
            return; // ignore
        case 0x040 >> 1: // BLTCON0
            blitter.pokeBLTCON0(value); return;
        case 0x042 >> 1: // BLTCON1
            blitter.pokeBLTCON1(value); return;
        case 0x044 >> 1: // BLTAFWM
            blitter.pokeBLTAFWM(value); return;
        case 0x046 >> 1: // BLTALWM
            blitter.pokeBLTALWM(value); return;
        case 0x048 >> 1: // BLTCPTH
            blitter.pokeBLTCPTH(value); return;
        case 0x04A >> 1: // BLTCPTL
            blitter.pokeBLTCPTL(value); return;
        case 0x04C >> 1: // BLTBPTH
            blitter.pokeBLTBPTH(value); return;
        case 0x04E >> 1: // BLTBPTL
            blitter.pokeBLTBPTL(value); return;
        case 0x050 >> 1: // BLTAPTH
            blitter.pokeBLTAPTH(value); return;
        case 0x052 >> 1: // BLTAPTL
            blitter.pokeBLTAPTL(value); return;
        case 0x054 >> 1: // BLTDPTH
            blitter.pokeBLTDPTH(value); return;
        case 0x056 >> 1: // BLTDPTL
            blitter.pokeBLTDPTL(value); return;
        case 0x058 >> 1: // BLTSIZE
            blitter.pokeBLTSIZE<s>(value); return;
        case 0x05A >> 1: // BLTCON0L (ECS)
            blitter.pokeBLTCON0L(value); return;
        case 0x05C >> 1: // BLTSIZV (ECS)
            blitter.pokeBLTSIZV(value); return;
        case 0x05E >> 1: // BLTSIZH (ECS)
            blitter.pokeBLTSIZH(value); return;
        case 0x060 >> 1: // BLTCMOD
            blitter.pokeBLTCMOD(value); return;
        case 0x062 >> 1: // BLTBMOD
            blitter.pokeBLTBMOD(value); return;
        case 0x064 >> 1: // BLTAMOD
            blitter.pokeBLTAMOD(value); return;
        case 0x066 >> 1: // BLTDMOD
            blitter.pokeBLTDMOD(value); return;
        case 0x068 >> 1: // unused
        case 0x06A >> 1: // unused
        case 0x06C >> 1: // unused
        case 0x06E >> 1: // unused
            break;
        case 0x070 >> 1: // BLTCDAT
            blitter.pokeBLTCDAT(value); return;
        case 0x072 >> 1: // BLTBDAT
            blitter.pokeBLTBDAT(value); return;
        case 0x074 >> 1: // BLTADAT
            blitter.pokeBLTADAT(value); return;
        case 0x076 >> 1: // unused
        case 0x078 >> 1: // unused
        case 0x07A >> 1: // unused
        case 0x07C >> 1: // unused
            break;
        case 0x07E >> 1: // DSKSYNC
            diskController.pokeDSKSYNC(value); return;
        case 0x080 >> 1: // COP1LCH
            copper.pokeCOP1LCH(value); return;
        case 0x082 >> 1: // COP1LCL
            copper.pokeCOP1LCL(value); return;
        case 0x084 >> 1: // COP2LCH
            copper.pokeCOP2LCH(value); return;
        case 0x086 >> 1: // COP2LCL
            copper.pokeCOP2LCL(value); return;
        case 0x088 >> 1: // COPJMP1
            copper.pokeCOPJMP1<s>(); return;
        case 0x08A >> 1: // COPJMP2
            copper.pokeCOPJMP2<s>(); return;
        case 0x08C >> 1: // COPINS
            copper.pokeCOPINS(value); return;
        case 0x08E >> 1: // DIWSTRT
            agnus.pokeDIWSTRT<s>(value); return;
        case 0x090 >> 1: // DIWSTOP
            agnus.pokeDIWSTOP<s>(value); return;
        case 0x092 >> 1: // DDFSTRT
            agnus.pokeDDFSTRT(value); return;
        case 0x094 >> 1: // DDFSTOP
            agnus.pokeDDFSTOP(value); return;
        case 0x096 >> 1: // DMACON
            agnus.pokeDMACON(value); return;
        case 0x098 >> 1:  // CLXCON
            denise.pokeCLXCON(value); return;
        case 0x09A >> 1: // INTENA
            paula.pokeINTENA<s>(value); return;
        case 0x09C >> 1: // INTREQ
            paula.pokeINTREQ<s>(value); return;
        case 0x09E >> 1: // ADKCON
            paula.pokeADKCON(value); return;
        case 0x0A0 >> 1: // AUD0LCH
            agnus.pokeAUDxLCH<0>(value); return;
        case 0x0A2 >> 1: // AUD0LCL
            agnus.pokeAUDxLCL<0>(value); return;
        case 0x0A4 >> 1: // AUD0LEN
            paula.channel0.pokeAUDxLEN(value); return;
        case 0x0A6 >> 1: // AUD0PER
            paula.channel0.pokeAUDxPER(value); return;
        case 0x0A8 >> 1: // AUD0VOL
            paula.channel0.pokeAUDxVOL(value); return;
        case 0x0AA >> 1: // AUD0DAT
            paula.channel0.pokeAUDxDAT(value); return;
        case 0x0AC >> 1: // Unused
        case 0x0AE >> 1: // Unused
            break;
        case 0x0B0 >> 1: // AUD1LCH
            agnus.pokeAUDxLCH<1>(value); return;
        case 0x0B2 >> 1: // AUD1LCL
            agnus.pokeAUDxLCL<1>(value); return;
        case 0x0B4 >> 1: // AUD1LEN
            paula.channel1.pokeAUDxLEN(value); return;
        case 0x0B6 >> 1: // AUD1PER
            paula.channel1.pokeAUDxPER(value); return;
        case 0x0B8 >> 1: // AUD1VOL
            paula.channel1.pokeAUDxVOL(value); return;
        case 0x0BA >> 1: // AUD1DAT
            paula.channel1.pokeAUDxDAT(value); return;
        case 0x0BC >> 1: // Unused
        case 0x0BE >> 1: // Unused
            break;
        case 0x0C0 >> 1: // AUD2LCH
            agnus.pokeAUDxLCH<2>(value); return;
        case 0x0C2 >> 1: // AUD2LCL
            agnus.pokeAUDxLCL<2>(value); return;
        case 0x0C4 >> 1: // AUD2LEN
            paula.channel2.pokeAUDxLEN(value); return;
        case 0x0C6 >> 1: // AUD2PER
            paula.channel2.pokeAUDxPER(value); return;
        case 0x0C8 >> 1: // AUD2VOL
            paula.channel2.pokeAUDxVOL(value); return;
        case 0x0CA >> 1: // AUD2DAT
            paula.channel2.pokeAUDxDAT(value); return;
        case 0x0CC >> 1: // Unused
        case 0x0CE >> 1: // Unused
            break;
        case 0x0D0 >> 1: // AUD3LCH
            agnus.pokeAUDxLCH<3>(value); return;
        case 0x0D2 >> 1: // AUD3LCL
            agnus.pokeAUDxLCL<3>(value); return;
        case 0x0D4 >> 1: // AUD3LEN
            paula.channel3.pokeAUDxLEN(value); return;
        case 0x0D6 >> 1: // AUD3PER
            paula.channel3.pokeAUDxPER(value); return;
        case 0x0D8 >> 1: // AUD3VOL
            paula.channel3.pokeAUDxVOL(value); return;
        case 0x0DA >> 1: // AUD3DAT
            paula.channel3.pokeAUDxDAT(value); return;
        case 0x0DC >> 1: // Unused
        case 0x0DE >> 1: // Unused
            break;
        case 0x0E0 >> 1: // BPL1PTH
            agnus.pokeBPLxPTH<1>(value); return;
        case 0x0E2 >> 1: // BPL1PTL
            agnus.pokeBPLxPTL<1>(value); return;
        case 0x0E4 >> 1: // BPL2PTH
            agnus.pokeBPLxPTH<2>(value); return;
        case 0x0E6 >> 1: // BPL2PTL
            agnus.pokeBPLxPTL<2>(value); return;
        case 0x0E8 >> 1: // BPL3PTH
            agnus.pokeBPLxPTH<3>(value); return;
        case 0x0EA >> 1: // BPL3PTL
            agnus.pokeBPLxPTL<3>(value); return;
        case 0x0EC >> 1: // BPL4PTH
            agnus.pokeBPLxPTH<4>(value); return;
        case 0x0EE >> 1: // BPL4PTL
            agnus.pokeBPLxPTL<4>(value); return;
        case 0x0F0 >> 1: // BPL5PTH
            agnus.pokeBPLxPTH<5>(value); return;
        case 0x0F2 >> 1: // BPL5PTL
            agnus.pokeBPLxPTL<5>(value); return;
        case 0x0F4 >> 1: // BPL6PTH
            agnus.pokeBPLxPTH<6>(value); return;
        case 0x0F6 >> 1: // BPL6PTL
            agnus.pokeBPLxPTL<6>(value); return;
        case 0x0F8 >> 1: // Unused
        case 0x0FA >> 1: // Unused
        case 0x0FC >> 1: // Unused
        case 0x0FE >> 1: // Unused
            break;
        case 0x100 >> 1: // BPLCON0
            agnus.pokeBPLCON0(value);
            denise.pokeBPLCON0(value);
            return;
        case 0x102 >> 1: // BPLCON1
            agnus.pokeBPLCON1(value);
            denise.pokeBPLCON1(value);
            return;
        case 0x104 >> 1: // BPLCON2
            denise.pokeBPLCON2(value); return;
        case 0x106 >> 1: // BPLCON3 (ECS)
            denise.pokeBPLCON3(value);
            break;
        case 0x108 >> 1: // BPL1MOD
            agnus.pokeBPL1MOD(value); return;
        case 0x10A >> 1: // BPL2MOD
            agnus.pokeBPL2MOD(value); return;
        case 0x10C >> 1: // Unused
        case 0x10E >> 1: // Unused
            break;
        case 0x110 >> 1: // BPL1DAT
            denise.pokeBPLxDAT<0,s>(value); return;
        case 0x112 >> 1: // BPL2DAT
            denise.pokeBPLxDAT<1,s>(value); return;
        case 0x114 >> 1: // BPL3DAT
            denise.pokeBPLxDAT<2,s>(value); return;
        case 0x116 >> 1: // BPL4DAT
            denise.pokeBPLxDAT<3,s>(value); return;
        case 0x118 >> 1: // BPL5DAT
            denise.pokeBPLxDAT<4,s>(value); return;
        case 0x11A >> 1: // BPL6DAT
            denise.pokeBPLxDAT<5,s>(value); return;
        case 0x11C >> 1: // Unused
        case 0x11E >> 1: // Unused
            break;
        case 0x120 >> 1: // SPR0PTH
            agnus.pokeSPRxPTH<0>(value); return;
        case 0x122 >> 1: // SPR0PTL
            agnus.pokeSPRxPTL<0>(value); return;
        case 0x124 >> 1: // SPR1PTH
            agnus.pokeSPRxPTH<1>(value); return;
        case 0x126 >> 1: // SPR1PTL
            agnus.pokeSPRxPTL<1>(value); return;
        case 0x128 >> 1: // SPR2PTH
            agnus.pokeSPRxPTH<2>(value); return;
        case 0x12A >> 1: // SPR2PTL
            agnus.pokeSPRxPTL<2>(value); return;
        case 0x12C >> 1: // SPR3PTH
            agnus.pokeSPRxPTH<3>(value); return;
        case 0x12E >> 1: // SPR3PTL
            agnus.pokeSPRxPTL<3>(value); return;
        case 0x130 >> 1: // SPR4PTH
            agnus.pokeSPRxPTH<4>(value); return;
        case 0x132 >> 1: // SPR4PTL
            agnus.pokeSPRxPTL<4>(value); return;
        case 0x134 >> 1: // SPR5PTH
            agnus.pokeSPRxPTH<5>(value); return;
        case 0x136 >> 1: // SPR5PTL
            agnus.pokeSPRxPTL<5>(value); return;
        case 0x138 >> 1: // SPR6PTH
            agnus.pokeSPRxPTH<6>(value); return;
        case 0x13A >> 1: // SPR6PTL
            agnus.pokeSPRxPTL<6>(value); return;
        case 0x13C >> 1: // SPR7PTH
            agnus.pokeSPRxPTH<7>(value); return;
        case 0x13E >> 1: // SPR7PTL
            agnus.pokeSPRxPTL<7>(value); return;
        case 0x140 >> 1: // SPR0POS
            agnus.pokeSPRxPOS<0>(value);
            denise.pokeSPRxPOS<0>(value);
            return;
        case 0x142 >> 1: // SPR0CTL
            agnus.pokeSPRxCTL<0>(value);
            denise.pokeSPRxCTL<0>(value);
            return;
        case 0x144 >> 1: // SPR0DATA
            denise.pokeSPRxDATA<0>(value); return;
        case 0x146 >> 1: // SPR0DATB
            denise.pokeSPRxDATB<0>(value); return;
        case 0x148 >> 1: // SPR1POS
            agnus.pokeSPRxPOS<1>(value);
            denise.pokeSPRxPOS<1>(value);
            return;
        case 0x14A >> 1: // SPR1CTL
            agnus.pokeSPRxCTL<1>(value);
            denise.pokeSPRxCTL<1>(value);
            return;
        case 0x14C >> 1: // SPR1DATA
            denise.pokeSPRxDATA<1>(value); return;
        case 0x14E >> 1: // SPR1DATB
            denise.pokeSPRxDATB<1>(value); return;
        case 0x150 >> 1: // SPR2POS
            agnus.pokeSPRxPOS<2>(value);
            denise.pokeSPRxPOS<2>(value);
            return;
        case 0x152 >> 1: // SPR2CTL
            agnus.pokeSPRxCTL<2>(value);
            denise.pokeSPRxCTL<2>(value);
            return;
        case 0x154 >> 1: // SPR2DATA
            denise.pokeSPRxDATA<2>(value); return;
        case 0x156 >> 1: // SPR2DATB
            denise.pokeSPRxDATB<2>(value); return;
        case 0x158 >> 1: // SPR3POS
            agnus.pokeSPRxPOS<3>(value);
            denise.pokeSPRxPOS<3>(value);
            return;
        case 0x15A >> 1: // SPR3CTL
            agnus.pokeSPRxCTL<3>(value);
            denise.pokeSPRxCTL<3>(value);
            return;
        case 0x15C >> 1: // SPR3DATA
            denise.pokeSPRxDATA<3>(value); return;
        case 0x15E >> 1: // SPR3DATB
            denise.pokeSPRxDATB<3>(value); return;
        case 0x160 >> 1: // SPR4POS
            agnus.pokeSPRxPOS<4>(value);
            denise.pokeSPRxPOS<4>(value);
            return;
        case 0x162 >> 1: // SPR4CTL
            agnus.pokeSPRxCTL<4>(value);
            denise.pokeSPRxCTL<4>(value);
            return;
        case 0x164 >> 1: // SPR4DATA
            denise.pokeSPRxDATA<4>(value); return;
        case 0x166 >> 1: // SPR4DATB
            denise.pokeSPRxDATB<4>(value); return;
        case 0x168 >> 1: // SPR5POS
            agnus.pokeSPRxPOS<5>(value);
            denise.pokeSPRxPOS<5>(value);
            return;
        case 0x16A >> 1: // SPR5CTL
            agnus.pokeSPRxCTL<5>(value);
            denise.pokeSPRxCTL<5>(value);
            return;
        case 0x16C >> 1: // SPR5DATA
            denise.pokeSPRxDATA<5>(value); return;
        case 0x16E >> 1: // SPR5DATB
            denise.pokeSPRxDATB<5>(value); return;
        case 0x170 >> 1: // SPR6POS
            agnus.pokeSPRxPOS<6>(value);
            denise.pokeSPRxPOS<6>(value);
            return;
        case 0x172 >> 1: // SPR6CTL
            agnus.pokeSPRxCTL<6>(value);
            denise.pokeSPRxCTL<6>(value);
            return;
        case 0x174 >> 1: // SPR6DATA
            denise.pokeSPRxDATA<6>(value); return;
        case 0x176 >> 1: // SPR6DATB
            denise.pokeSPRxDATB<6>(value); return;
        case 0x178 >> 1: // SPR7POS
            agnus.pokeSPRxPOS<7>(value);
            denise.pokeSPRxPOS<7>(value);
            return;
        case 0x17A >> 1: // SPR7CTL
            agnus.pokeSPRxCTL<7>(value);
            denise.pokeSPRxCTL<7>(value);
            return;
        case 0x17C >> 1: // SPR7DATA
            denise.pokeSPRxDATA<7>(value); return;
        case 0x17E >> 1: // SPR7DATB
            denise.pokeSPRxDATB<7>(value); return;
        case 0x180 >> 1: // COLOR00
            denise.pokeCOLORxx<s,0>(value); return;
        case 0x182 >> 1: // COLOR01
            denise.pokeCOLORxx<s,1>(value); return;
        case 0x184 >> 1: // COLOR02
            denise.pokeCOLORxx<s,2>(value); return;
        case 0x186 >> 1: // COLOR03
            denise.pokeCOLORxx<s,3>(value); return;
        case 0x188 >> 1: // COLOR04
            denise.pokeCOLORxx<s,4>(value); return;
        case 0x18A >> 1: // COLOR05
            denise.pokeCOLORxx<s,5>(value); return;
        case 0x18C >> 1: // COLOR06
            denise.pokeCOLORxx<s,6>(value); return;
        case 0x18E >> 1: // COLOR07
            denise.pokeCOLORxx<s,7>(value); return;
        case 0x190 >> 1: // COLOR08
            denise.pokeCOLORxx<s,8>(value); return;
        case 0x192 >> 1: // COLOR09
            denise.pokeCOLORxx<s,9>(value); return;
        case 0x194 >> 1: // COLOR10
            denise.pokeCOLORxx<s,10>(value); return;
        case 0x196 >> 1: // COLOR11
            denise.pokeCOLORxx<s,11>(value); return;
        case 0x198 >> 1: // COLOR12
            denise.pokeCOLORxx<s,12>(value); return;
        case 0x19A >> 1: // COLOR13
            denise.pokeCOLORxx<s,13>(value); return;
        case 0x19C >> 1: // COLOR14
            denise.pokeCOLORxx<s,14>(value); return;
        case 0x19E >> 1: // COLOR15
            denise.pokeCOLORxx<s,15>(value); return;
        case 0x1A0 >> 1: // COLOR16
            denise.pokeCOLORxx<s,16>(value); return;
        case 0x1A2 >> 1: // COLOR17
            denise.pokeCOLORxx<s,17>(value); return;
        case 0x1A4 >> 1: // COLOR18
            denise.pokeCOLORxx<s,18>(value); return;
        case 0x1A6 >> 1: // COLOR19
            denise.pokeCOLORxx<s,19>(value); return;
        case 0x1A8 >> 1: // COLOR20
            denise.pokeCOLORxx<s,20>(value); return;
        case 0x1AA >> 1: // COLOR21
            denise.pokeCOLORxx<s,21>(value); return;
        case 0x1AC >> 1: // COLOR22
            denise.pokeCOLORxx<s,22>(value); return;
        case 0x1AE >> 1: // COLOR23
            denise.pokeCOLORxx<s,23>(value); return;
        case 0x1B0 >> 1: // COLOR24
            denise.pokeCOLORxx<s,24>(value); return;
        case 0x1B2 >> 1: // COLOR25
            denise.pokeCOLORxx<s,25>(value); return;
        case 0x1B4 >> 1: // COLOR26
            denise.pokeCOLORxx<s,26>(value); return;
        case 0x1B6 >> 1: // COLOR27
            denise.pokeCOLORxx<s,27>(value); return;
        case 0x1B8 >> 1: // COLOR28
            denise.pokeCOLORxx<s,28>(value); return;
        case 0x1BA >> 1: // COLOR29
            denise.pokeCOLORxx<s,29>(value); return;
        case 0x1BC >> 1: // COLOR30
            denise.pokeCOLORxx<s,30>(value); return;
        case 0x1BE >> 1: // COLOR31
            denise.pokeCOLORxx<s,31>(value); return;
        case 0x1FE >> 1: // NO-OP
            copper.pokeNOOP(value); return;
    }
    
    if (addr <= 0x1E) {
        trace(INVREG_DEBUG,
              "pokeCustom16(%X [%s]): READ-ONLY\n", addr, regName(addr));
    } else {
        trace(INVREG_DEBUG,
              "pokeCustom16(%X [%s]): NON-OCS\n", addr, regName(addr));
    }
}

template <Accessor A> const char *
Memory::ascii(u32 addr)
{
    for (isize i = 0; i < 16; i += 2) {
        u16 word = spypeek16 <A> ((u32)(addr + i));
        str[i] = isprint(HI_BYTE(word)) ? HI_BYTE(word) : '.';
        str[i+1] = isprint(LO_BYTE(word)) ? LO_BYTE(word) : '.';
    }
    str[16] = 0;
    return str;
}

template <Accessor A> const char *
Memory::hex(u32 addr, isize bytes)
{
    assert(bytes % 2 == 0);
    char *p = str;
    
    for (isize i = 0; i < bytes / 2; i += 2, p += 5) {

        u16 word = spypeek16 <A> ((u32)(addr + i));
        
        u8 digit1 = (word >> 12) & 0xF;
        u8 digit2 = (word >> 8) & 0xF;
        u8 digit3 = (word >> 4) & 0xF;
        u8 digit4 = (word >> 0) & 0xF;
        
        p[0] = digit1 < 10 ? '0' + digit1 : 'A' + digit1 - 10;
        p[1] = digit2 < 10 ? '0' + digit2 : 'A' + digit2 - 10;
        p[2] = digit3 < 10 ? '0' + digit3 : 'A' + digit3 - 10;
        p[3] = digit4 < 10 ? '0' + digit4 : 'A' + digit4 - 10;
        p[4] = i == bytes - 2 ? '0' : ' ';
    }

    return str;
}

template void Memory::pokeCustom16 <ACCESSOR_CPU> (u32 addr, u16 value);
template void Memory::pokeCustom16 <ACCESSOR_AGNUS> (u32 addr, u16 value);

template const char *Memory::ascii <ACCESSOR_CPU> (u32 addr);
template const char *Memory::ascii <ACCESSOR_AGNUS> (u32 addr);

template const char *Memory::hex <ACCESSOR_CPU> (u32 addr, isize bytes);
template const char *Memory::hex <ACCESSOR_AGNUS> (u32 addr, isize bytes);
