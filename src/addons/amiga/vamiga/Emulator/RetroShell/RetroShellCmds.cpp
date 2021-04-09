// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RetroShell.h"
#include "Amiga.h"
#include "BootBlockImage.h"
#include "FSTypes.h"
#include "IO.h"
#include "Parser.h"
#include <fstream>
#include <sstream>

//
// Top-level commands
//

template <> void
RetroShell::exec <Token::clear> (Arguments &argv, long param)
{
    clear();
}

template <> void
RetroShell::exec <Token::close> (Arguments &argv, long param)
{
    messageQueue.put(MSG_CLOSE_CONSOLE);
}

template <> void
RetroShell::exec <Token::easteregg> (Arguments& argv, long param)
{
    retroShell << "GREETINGS PROFESSOR HOFFMANN." << '\n' << '\n';
    retroShell << "THE ONLY WINNING MOVE IS NOT TO PLAY." << '\n' << '\n';
    retroShell << "HOW ABOUT A NICE GAME OF CHESS?" << '\n';
}

template <> void
RetroShell::exec <Token::source> (Arguments &argv, long param)
{
    string filename = argv.front();
    
    std::ifstream stream(filename);
    if (!stream.is_open()) throw ConfigFileReadError(filename);
    
    try {
        exec(stream);
    } catch (util::Exception &e) {
        retroShell << "Error in line " << (isize)e.data << '\n';
        retroShell << e.what() << '\n';
    }
}

//
// Amiga
//

template <> void
RetroShell::exec <Token::amiga, Token::on> (Arguments &argv, long param)
{
    amiga.powerOn();
}

template <> void
RetroShell::exec <Token::amiga, Token::off> (Arguments &argv, long param)
{
    amiga.powerOff();
}

template <> void
RetroShell::exec <Token::amiga, Token::run> (Arguments &argv, long param)
{
    amiga.run();
}

template <> void
RetroShell::exec <Token::amiga, Token::pause> (Arguments &argv, long param)
{
    amiga.pause();
}

template <> void
RetroShell::exec <Token::amiga, Token::reset> (Arguments &argv, long param)
{
    amiga.reset(true);
}

template <> void
RetroShell::exec <Token::amiga, Token::inspect> (Arguments &argv, long param)
{
    dump(amiga, Dump::State);
}

//
// Memory
//

template <> void
RetroShell::exec <Token::memory, Token::config> (Arguments& argv, long param)
{
    dump(amiga.mem, Dump::Config);
}

template <> void
RetroShell::exec <Token::memory, Token::load, Token::rom> (Arguments& argv, long param)
{
    auto path = argv.front();
    if (!util::fileExists(path)) throw ConfigFileNotFoundError(path);

    amiga.mem.loadRomFromFile(path.c_str());
}

template <> void
RetroShell::exec <Token::memory, Token::load, Token::extrom> (Arguments& argv, long param)
{
    auto path = argv.front();
    if (!util::fileExists(path)) throw ConfigFileNotFoundError(path);

    amiga.mem.loadExtFromFile(path.c_str());
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::chip> (Arguments& argv, long param)
{
    amiga.configure(OPT_CHIP_RAM, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::slow> (Arguments& argv, long param)
{
    amiga.configure(OPT_SLOW_RAM, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::fast> (Arguments& argv, long param)
{
    amiga.configure(OPT_FAST_RAM, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::extstart> (Arguments& argv, long param)
{
    amiga.configure(OPT_EXT_START, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::slowramdelay> (Arguments& argv, long param)
{
    amiga.configure(OPT_SLOW_RAM_DELAY, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::bankmap> (Arguments& argv, long param)
{
    amiga.configure(OPT_BANKMAP, util::parseEnum <BankMapEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::unmappingtype> (Arguments& argv, long param)
{
    amiga.configure(OPT_UNMAPPING_TYPE, util::parseEnum <UnmappedMemoryEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::set, Token::raminitpattern> (Arguments& argv, long param)
{
    amiga.configure(OPT_RAM_INIT_PATTERN, util::parseEnum <RamInitPatternEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::memory, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dump(amiga.mem, Dump::State);
}

template <> void
RetroShell::exec <Token::memory, Token::inspect, Token::bankmap> (Arguments& argv, long param)
{
    dump(amiga.mem, Dump::BankMap);
}

template <> void
RetroShell::exec <Token::memory, Token::inspect, Token::checksums> (Arguments& argv, long param)
{
    dump(amiga.mem, Dump::Checksums);
}

//
// CPU
//

template <> void
RetroShell::exec <Token::cpu, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dump(amiga.cpu, Dump::State);
}

template <> void
RetroShell::exec <Token::cpu, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.cpu, Dump::Registers);
}

//
// CIA
//

template <> void
RetroShell::exec <Token::cia, Token::config> (Arguments &argv, long param)
{
    if (param == 0) {
        dump(amiga.ciaA, Dump::Config);
    } else {
        dump(amiga.ciaB, Dump::Config);
    }
}

template <> void
RetroShell::exec <Token::cia, Token::set, Token::revision> (Arguments &argv, long param)
{
    if (param == 0) {
        amiga.ciaA.configure(OPT_CIA_REVISION, util::parseEnum <CIARevisionEnum> (argv.front()));
    } else {
        amiga.ciaB.configure(OPT_CIA_REVISION, util::parseEnum <CIARevisionEnum> (argv.front()));
    }
}

template <> void
RetroShell::exec <Token::cia, Token::set, Token::todbug> (Arguments &argv, long param)
{
    if (param == 0) {
        amiga.ciaA.configure(OPT_TODBUG, util::parseBool(argv.front()));
    } else {
        amiga.ciaB.configure(OPT_TODBUG, util::parseBool(argv.front()));
    }
}

template <> void
RetroShell::exec <Token::cia, Token::set, Token::esync> (Arguments &argv, long param)
{
    if (param == 0) {
        amiga.ciaA.configure(OPT_ECLOCK_SYNCING, util::parseBool(argv.front()));
    } else {
        amiga.ciaB.configure(OPT_ECLOCK_SYNCING, util::parseBool(argv.front()));
    }
}

template <> void
RetroShell::exec <Token::cia, Token::inspect, Token::state> (Arguments& argv, long param)
{
    if (param == 0) {
        dump(amiga.ciaA, Dump::State);
    } else {
        dump(amiga.ciaB, Dump::State);
    }
}

template <> void
RetroShell::exec <Token::cia, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    if (param == 0) {
        dump(amiga.ciaA, Dump::Registers);
    } else {
        dump(amiga.ciaB, Dump::Registers);
    }
}

template <> void
RetroShell::exec <Token::cia, Token::inspect, Token::tod> (Arguments& argv, long param)
{
    if (param == 0) {
        dump(amiga.ciaA.tod, Dump::State);
    } else {
        dump(amiga.ciaB.tod, Dump::State);
    }
}

//
// Agnus
//

template <> void
RetroShell::exec <Token::agnus, Token::config> (Arguments &argv, long param)
{
    dump(amiga.agnus, Dump::Config);
}

template <> void
RetroShell::exec <Token::agnus, Token::set, Token::revision> (Arguments &argv, long param)
{
    amiga.configure(OPT_AGNUS_REVISION, util::parseEnum <AgnusRevisionEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::agnus, Token::set, Token::slowrammirror> (Arguments &argv, long param)
{
    amiga.configure(OPT_SLOW_RAM_MIRROR, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::agnus, Token::inspect, Token::state> (Arguments &argv, long param)
{
    dump(amiga.agnus, Dump::State);
}

template <> void
RetroShell::exec <Token::agnus, Token::inspect, Token::registers> (Arguments &argv, long param)
{
    dump(amiga.agnus, Dump::Registers);
}

template <> void
RetroShell::exec <Token::agnus, Token::inspect, Token::events> (Arguments &argv, long param)
{
    dump(amiga.agnus, Dump::Events);
}

//
// Blitter
//

template <> void
RetroShell::exec <Token::blitter, Token::config> (Arguments& argv, long param)
{
    dump(amiga.agnus.blitter, Dump::Config);
}

template <> void
RetroShell::exec <Token::blitter, Token::set, Token::accuracy> (Arguments &argv, long param)
{
    amiga.configure(OPT_BLITTER_ACCURACY, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::blitter, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dump(amiga.agnus.blitter, Dump::State);
}

template <> void
RetroShell::exec <Token::blitter, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.agnus.blitter, Dump::Registers);
}

//
// Copper
//

template <> void
RetroShell::exec <Token::copper, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dump(amiga.agnus.copper, Dump::State);
}

template <> void
RetroShell::exec <Token::copper, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.agnus.copper, Dump::Registers);
}

//
// Denise
//

template <> void
RetroShell::exec <Token::denise, Token::config> (Arguments& argv, long param)
{
    dump(amiga.denise, Dump::Config);
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::revision> (Arguments &argv, long param)
{
    amiga.configure(OPT_DENISE_REVISION, util::parseEnum <DeniseRevisionEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::clxsprspr> (Arguments &argv, long param)
{
    amiga.configure(OPT_CLX_SPR_SPR, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::clxsprplf> (Arguments &argv, long param)
{
    amiga.configure(OPT_CLX_SPR_PLF, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::set, Token::clxplfplf> (Arguments &argv, long param)
{
    amiga.configure(OPT_CLX_PLF_PLF, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::denise, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dump(amiga.denise, Dump::State);
}

template <> void
RetroShell::exec <Token::denise, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.denise, Dump::Registers);
}

//
// Monitor
//

template <> void
RetroShell::exec <Token::monitor, Token::set, Token::palette> (Arguments& argv, long param)
{
    amiga.configure(OPT_PALETTE, util::parseEnum <PaletteEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::monitor, Token::set, Token::brightness> (Arguments& argv, long param)
{
    amiga.configure(OPT_BRIGHTNESS, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::monitor, Token::set, Token::contrast> (Arguments& argv, long param)
{
    amiga.configure(OPT_CONTRAST, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::monitor, Token::set, Token::saturation> (Arguments& argv, long param)
{
    amiga.configure(OPT_SATURATION, util::parseNum(argv.front()));
}

//
// Audio
//

template <> void
RetroShell::exec <Token::audio, Token::config> (Arguments& argv, long param)
{
    dump(amiga.paula.muxer, Dump::Config);
}

template <> void
RetroShell::exec <Token::audio, Token::set, Token::sampling> (Arguments& argv, long param)
{
    amiga.configure(OPT_SAMPLING_METHOD, util::parseEnum <SamplingMethodEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::audio, Token::set, Token::filter> (Arguments& argv, long param)
{
    amiga.configure(OPT_FILTER_TYPE, util::parseEnum <FilterTypeEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::audio, Token::set, Token::volume> (Arguments& argv, long param)
{
    switch (param) {
            
        case 0: amiga.configure(OPT_AUDVOL, 0, util::parseNum(argv.front())); break;
        case 1: amiga.configure(OPT_AUDVOL, 1, util::parseNum(argv.front())); break;
        case 2: amiga.configure(OPT_AUDVOL, 2, util::parseNum(argv.front())); break;
        case 3: amiga.configure(OPT_AUDVOL, 3, util::parseNum(argv.front())); break;
        case 4: amiga.configure(OPT_AUDVOLL, util::parseNum(argv.front())); break;
        case 5: amiga.configure(OPT_AUDVOLR, util::parseNum(argv.front())); break;
            
        default:
            assert(false);
    }
}

template <> void
RetroShell::exec <Token::audio, Token::set, Token::pan> (Arguments& argv, long param)
{
    amiga.configure(OPT_AUDPAN, param, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::audio, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dump(amiga.paula.muxer, Dump::State);
}

template <> void
RetroShell::exec <Token::audio, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.paula.muxer, Dump::Registers);
}

//
// Paula
//

template <> void
RetroShell::exec <Token::paula, Token::inspect, Token::state> (Arguments& argv, long param)
{
    dump(amiga.paula, Dump::State);
}

template <> void
RetroShell::exec <Token::paula, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.paula, Dump::Registers);
}

//
// RTC
//

template <> void
RetroShell::exec <Token::rtc, Token::config> (Arguments& argv, long param)
{
    dump(amiga.rtc, Dump::Config);
}

template <> void
RetroShell::exec <Token::rtc, Token::inspect, Token::registers> (Arguments& argv, long param)
{
    dump(amiga.rtc, Dump::Registers);
}

template <> void
RetroShell::exec <Token::rtc, Token::set, Token::revision> (Arguments &argv, long param)
{
    amiga.configure(OPT_RTC_MODEL, util::parseEnum <RTCRevisionEnum> (argv.front()));
}

//
// Control port
//

template <> void
RetroShell::exec <Token::controlport, Token::config> (Arguments& argv, long param)
{
    dump(param == 0 ? amiga.controlPort1 : amiga.controlPort2, Dump::Config);
}

/*
template <> void
RetroShell::exec <Token::controlport, Token::connect, Token::mouse> (Arguments& argv, long param)
{
    //
}

template <> void
RetroShell::exec <Token::controlport, Token::connect, Token::joystick> (Arguments& argv, long param)
{
    //
}

template <> void
RetroShell::exec <Token::controlport, Token::connect, Token::keyset> (Arguments& argv, long param)
{
    //
}
*/

template <> void
RetroShell::exec <Token::controlport, Token::inspect> (Arguments& argv, long param)
{
    dump(param == 0 ? amiga.controlPort1 : amiga.controlPort2, Dump::State);
}

//
// Keyboard
//

template <> void
RetroShell::exec <Token::keyboard, Token::config> (Arguments& argv, long param)
{
    dump(amiga.keyboard, Dump::Config);
}

template <> void
RetroShell::exec <Token::keyboard, Token::set, Token::accuracy> (Arguments &argv, long param)
{
    amiga.configure(OPT_ACCURATE_KEYBOARD, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::keyboard, Token::inspect> (Arguments& argv, long param)
{
    dump(amiga.keyboard, Dump::State);
}

//
// Mouse
//

template <> void
RetroShell::exec <Token::mouse, Token::config> (Arguments& argv, long param)
{
    dump(amiga.controlPort1.mouse, Dump::Config);
}

template <> void
RetroShell::exec <Token::mouse, Token::set, Token::pullup> (Arguments &argv, long param)
{
    amiga.configure(OPT_PULLUP_RESISTORS, PORT_1, util::parseBool(argv.front()));
    amiga.configure(OPT_PULLUP_RESISTORS, PORT_2, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::mouse, Token::set, Token::shakedetector> (Arguments &argv, long param)
{
    amiga.configure(OPT_SHAKE_DETECTION, PORT_1, util::parseBool(argv.front()));
    amiga.configure(OPT_SHAKE_DETECTION, PORT_2, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::mouse, Token::set, Token::velocity> (Arguments &argv, long param)
{
    amiga.configure(OPT_MOUSE_VELOCITY, PORT_1, util::parseNum(argv.front()));
    amiga.configure(OPT_MOUSE_VELOCITY, PORT_2, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::mouse, Token::inspect> (Arguments& argv, long param)
{
    dump(amiga.keyboard, Dump::State);
}

//
// Serial port
//

template <> void
RetroShell::exec <Token::serial, Token::config> (Arguments& argv, long param)
{
    dump(amiga.serialPort, Dump::Config);
}

template <> void
RetroShell::exec <Token::serial, Token::set, Token::device> (Arguments &argv, long param)
{
    amiga.configure(OPT_SERIAL_DEVICE, util::parseEnum <SerialPortDeviceEnum> (argv.front()));
}

template <> void
RetroShell::exec <Token::serial, Token::inspect> (Arguments& argv, long param)
{
    dump(amiga.serialPort, Dump::State);
}

//
// Disk controller
//

template <> void
RetroShell::exec <Token::dc, Token::config> (Arguments& argv, long param)
{
    dump(amiga.paula.diskController, Dump::Config);
}

template <> void
RetroShell::exec <Token::dc, Token::inspect> (Arguments& argv, long param)
{
    dump(amiga.paula.diskController, Dump::Registers);
}

template <> void
RetroShell::exec <Token::dc, Token::speed> (Arguments& argv, long param)
{
    amiga.configure(OPT_DRIVE_SPEED, util::parseNum(argv.front()));
}

template <> void
RetroShell::exec <Token::dc, Token::dsksync, Token::autosync> (Arguments& argv, long param)
{
    amiga.configure(OPT_AUTO_DSKSYNC, util::parseBool(argv.front()));
}

template <> void
RetroShell::exec <Token::dc, Token::dsksync, Token::lock> (Arguments& argv, long param)
{
    amiga.configure(OPT_LOCK_DSKSYNC, util::parseBool(argv.front()));
}

//
// Df0, Df1, Df2, Df3
//

template <> void
RetroShell::exec <Token::dfn, Token::config> (Arguments& argv, long param)
{
    dump(*amiga.df[param], Dump::Config);
}

template <> void
RetroShell::exec <Token::dfn, Token::audiate, Token::insert> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());

    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_INSERT_VOLUME, param, num);
    } else {
        amiga.configure(OPT_INSERT_VOLUME, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::audiate, Token::eject> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_EJECT_VOLUME, param, num);
    } else {
        amiga.configure(OPT_EJECT_VOLUME, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::audiate, Token::step> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_STEP_VOLUME, param, num);
    } else {
        amiga.configure(OPT_STEP_VOLUME, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::audiate, Token::poll> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_POLL_VOLUME, param, num);
    } else {
        amiga.configure(OPT_POLL_VOLUME, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::eject> (Arguments& argv, long param)
{
    amiga.df[param]->ejectDisk();
}

template <> void
RetroShell::exec <Token::dfn, Token::connect> (Arguments& argv, long param)
{
    amiga.configure(OPT_DRIVE_CONNECT, param, true);
}

template <> void
RetroShell::exec <Token::dfn, Token::disconnect> (Arguments& argv, long param)
{
    amiga.configure(OPT_DRIVE_CONNECT, param, false);
}

template <> void
RetroShell::exec <Token::dfn, Token::insert> (Arguments& argv, long param)
{
    auto path = argv.front();
    if (!util::fileExists(path)) throw ConfigFileNotFoundError(path);
    
    amiga.paula.diskController.insertDisk(path, param);
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::model> (Arguments& argv, long param)
{
    long num = util::parseEnum <DriveTypeEnum> (argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_DRIVE_TYPE, param, num);
    } else {
        amiga.configure(OPT_DRIVE_TYPE, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::pan> (Arguments& argv, long param)
{
    long num = util::parseNum(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_DRIVE_PAN, param, num);
    } else {
        amiga.configure(OPT_DRIVE_PAN, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::mechanics> (Arguments& argv, long param)
{
    long num = util::parseBool(argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_EMULATE_MECHANICS, param, num);
    } else {
        amiga.configure(OPT_EMULATE_MECHANICS, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::searchpath> (Arguments& argv, long param)
{
    string path = argv.front();
    
    if (param >= 0 && param <= 3) {
        amiga.paula.diskController.setSearchPath(path, param);
    } else {
        amiga.paula.diskController.setSearchPath(path);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::defaultbb> (Arguments& argv, long param)
{
    long num = util::parseEnum <BootBlockIdEnum> (argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_DEFAULT_BOOTBLOCK, param, num);
    } else {
        amiga.configure(OPT_DEFAULT_BOOTBLOCK, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::set, Token::defaultfs> (Arguments& argv, long param)
{
    long num = util::parseEnum <FSVolumeTypeEnum> (argv.front());
    
    if (param >= 0 && param <= 3) {
        amiga.configure(OPT_DEFAULT_FILESYSTEM, param, num);
    } else {
        amiga.configure(OPT_DEFAULT_FILESYSTEM, num);
    }
}

template <> void
RetroShell::exec <Token::dfn, Token::inspect> (Arguments& argv, long param)
{
    dump(*amiga.df[param], Dump::State);
}
