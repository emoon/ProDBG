// -----------------------------------------------------------------------------
// This file is part of vAmiga Bare Metal
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Interpreter.h"
#include "RetroShell.h"

void
Interpreter::registerInstructions()
{
    //
    // Commands
    //
    
    root.add({"clear"},
             "command", "Clears the console window",
             &RetroShell::exec <Token::clear>);
    root.seek("clear")->hidden = true;

    root.add({"close"},
             "command", "Hides the debug console",
             &RetroShell::exec <Token::close>);
    root.seek("close")->hidden = true;

    root.add({"joshua"},
             "command", "",
             &RetroShell::exec <Token::easteregg>);
    root.seek("joshua")->hidden = true;

    root.add({"source"},
             "command", "Processes a command script",
             &RetroShell::exec <Token::source>, 1);
    
    
    //
    // Amiga
    //
    
    root.add({"amiga"},
             "component", "The virtual Amiga");
        
    root.add({"amiga", "power"},
             "command", "Switches the Amiga on or off");
    
    root.add({"amiga", "power", "on"},
             "state", "Switches the Amiga on",
             &RetroShell::exec <Token::amiga, Token::on>);

    root.add({"amiga", "power", "off"},
             "state", "Switches the Amiga off",
             &RetroShell::exec <Token::amiga, Token::off>);

    root.add({"amiga", "run"},
             "command", "Starts the emulator thread",
             &RetroShell::exec <Token::amiga, Token::run>);

    root.add({"amiga", "pause"},
             "command", "Halts the emulator thread",
             &RetroShell::exec <Token::amiga, Token::pause>);
    
    root.add({"amiga", "reset"},
             "command", "Performs a hard reset",
             &RetroShell::exec <Token::amiga, Token::reset>);
    
    root.add({"amiga", "inspect"},
             "command", "Displays the component state",
             &RetroShell::exec <Token::amiga, Token::inspect>);

    
    //
    // Memory
    //
    
    root.add({"memory"},
             "component", "Ram and Rom");
    
    root.add({"memory", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::memory, Token::config>);

    root.add({"memory", "set"},
             "command", "Configures the component");
        
    root.add({"memory", "set", "chip"},
             "key", "Configures the amouts of chip memory",
             &RetroShell::exec <Token::memory, Token::set, Token::chip>, 1);

    root.add({"memory", "set", "slow"},
             "key", "Configures the amouts of slow memory",
             &RetroShell::exec <Token::memory, Token::set, Token::slow>, 1);

    root.add({"memory", "set", "fast"},
             "key", "Configures the amouts of flow memory",
             &RetroShell::exec <Token::memory, Token::set, Token::fast>, 1);

    root.add({"memory", "set", "extstart"},
             "key", "Sets the start address for Rom extensions",
             &RetroShell::exec <Token::memory, Token::set, Token::extstart>, 1);

    root.add({"memory", "set", "slowramdelay"},
             "key", "Enables or disables slow Ram bus delays",
             &RetroShell::exec <Token::memory, Token::set, Token::slowramdelay>, 1);

    root.add({"memory", "set", "bankmap"},
             "key", "Selects the bank mapping scheme",
             &RetroShell::exec <Token::memory, Token::set, Token::bankmap>, 1);

    root.add({"memory", "set", "unmapped"},
             "key", "Determines the behaviour of unmapped memory",
             &RetroShell::exec <Token::memory, Token::set, Token::unmappingtype>, 1);

    root.add({"memory", "set", "raminit"},
             "key", "Determines how Ram is initialized on startup",
             &RetroShell::exec <Token::memory, Token::set, Token::raminitpattern>, 1);
    
    root.add({"memory", "load"},
             "command", "Installs a Rom image");
            
    root.add({"memory", "load", "rom"},
             "command", "Installs a Kickstart Rom",
             &RetroShell::exec <Token::memory, Token::load, Token::rom>, 1);

    root.add({"memory", "load", "extrom"},
             "command", "Installs a Rom extension",
             &RetroShell::exec <Token::memory, Token::load, Token::extrom>, 1);

    root.add({"memory", "inspect"},
             "command", "Displays the component state");

    root.add({"memory", "inspect", "state"},
             "command", "Displays the current state",
             &RetroShell::exec <Token::memory, Token::inspect, Token::state>);

    root.add({"memory", "inspect", "bankmap"},
             "command", "Displays the bank map",
             &RetroShell::exec <Token::memory, Token::inspect, Token::bankmap>);

    root.add({"memory", "inspect", "checksum"},
             "command", "Computes memory checksums",
             &RetroShell::exec <Token::memory, Token::inspect, Token::checksums>);

    
    //
    // CPU
    //
    
    root.add({"cpu"},
             "component", "Motorola 68k CPU");
    
    root.add({"cpu", "inspect"},
             "command", "Displays the component state");

    root.add({"cpu", "inspect", "state"},
             "command", "Displays the current state",
             &RetroShell::exec <Token::cpu, Token::inspect, Token::state>);

    root.add({"cpu", "inspect", "registers"},
             "command", "Displays the current register values",
             &RetroShell::exec <Token::cpu, Token::inspect, Token::registers>);

    
    //
    // CIA
    //
    
    root.add({"ciaa"},
             "component", "Complex Interface Adapter A");

    root.add({"ciab"},
             "component", "Complex Interface Adapter B");

    root.add({"ciaa","ciab"}, {"", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::cia, Token::config>);

    root.add({"ciaa","ciab"}, {"", "set"},
             "command", "Configures the component");
        
    root.add({"ciaa","ciab"}, {"", "set", "revision"},
             "key", "Selects the emulated chip model",
             &RetroShell::exec <Token::cia, Token::set, Token::revision>, 1);

    root.add({"ciaa","ciab"}, {"", "set", "todbug"},
             "key", "Enables or disables the TOD hardware bug",
             &RetroShell::exec <Token::cia, Token::set, Token::todbug>, 1);

    root.add({"ciaa","ciab"}, {"", "set", "esync"},
             "key", "Turns E-clock syncing on or off",
             &RetroShell::exec <Token::cia, Token::set, Token::esync>, 1);

    root.add({"ciaa","ciab"}, {"", "inspect"},
             "command", "Displays the component state");

    root.add({"ciaa","ciab"}, {"", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::cia, Token::inspect, Token::state>);

    root.add({"ciaa","ciab"}, {"", "inspect", "registers"},
             "category", "Displays the current register values",
             &RetroShell::exec <Token::cia, Token::inspect, Token::registers>);

    root.add({"ciaa","ciab"}, {"", "inspect", "tod"},
             "category", "Displays the state of the 24-bit counter",
             &RetroShell::exec <Token::cia, Token::inspect, Token::tod>);

    
    //
    // Agnus
    //

    root.add({"agnus"},
             "component", "Custom chip");

    root.add({"agnus", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::agnus, Token::config>);
    
    root.add({"agnus", "set"},
             "command", "Configures the component");
        
    root.add({"agnus", "set", "revision"},
             "key", "Selects the emulated chip model",
             &RetroShell::exec <Token::agnus, Token::set, Token::revision>, 1);

    root.add({"agnus", "set", "slowrammirror"},
             "key", "Enables or disables ECS Slow Ram mirroring",
             &RetroShell::exec <Token::agnus, Token::set, Token::slowrammirror>, 1);

    root.add({"agnus", "inspect"},
             "command", "Displays the internal state");

    root.add({"agnus", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::state>);

    root.add({"agnus", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::registers>);

    root.add({"agnus", "inspect", "events"},
             "category", "Displays scheduled events",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::events>);
    
    
    //
    // Blitter
    //
    
    root.add({"blitter"},
             "component", "Custom Chip (Agnus)");
    
    root.add({"blitter", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::blitter, Token::config>);
    
    root.add({"blitter", "set"},
             "command", "Configures the component");
        
    root.add({"blitter", "set", "accuracy"},
             "level", "Selects the emulation accuracy level",
             &RetroShell::exec <Token::blitter, Token::set, Token::accuracy>, 1);

    root.add({"blitter", "inspect"},
             "command", "Displays the internal state");

    root.add({"blitter", "inspect", "state"},
             "category", "Displays the internal state",
             &RetroShell::exec <Token::blitter, Token::inspect, Token::state>);

    root.add({"blitter", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::blitter, Token::inspect, Token::registers>);

    
    //
    // Copper
    //
    
    root.add({"copper"},
             "component", "Custom Chip (Agnus)");
    
    root.add({"copper", "inspect"},
             "command", "Displays the internal state");

    root.add({"copper", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::copper, Token::inspect, Token::state>);

    root.add({"copper", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::copper, Token::inspect, Token::registers>);

    
    //
    // Denise
    //
    
    root.add({"denise"},
             "component", "Custom chip");
    
        root.add({"denise", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::denise, Token::config>);

    root.add({"denise", "set"},
             "command", "Configures the component");

    root.add({"denise", "set", "revision"},
             "key", "Selects the emulated chip model",
             &RetroShell::exec <Token::denise, Token::set, Token::revision>, 1);
    
    root.add({"denise", "set", "clxsprspr"},
             "key", "Enables or disables sprite-sprite collision detection",
             &RetroShell::exec <Token::denise, Token::set, Token::clxsprspr>, 1);

    root.add({"denise", "set", "clxsprplf"},
             "key", "Enables or disables sprite-playfield collision detection",
             &RetroShell::exec <Token::denise, Token::set, Token::clxsprplf>, 1);

    root.add({"denise", "set", "clxplfplf"},
             "key", "Enables or disables playfield-playfield collision detection",
             &RetroShell::exec <Token::denise, Token::set, Token::clxplfplf>, 1);
    
    root.add({"denise", "inspect"},
             "command", "Displays the internal state");

    root.add({"denise", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::denise, Token::inspect, Token::state>);

    root.add({"denise", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::denise, Token::inspect, Token::registers>);

    
    //
    // Monitor
    //

    root.add({"monitor"},
             "component", "Amiga monitor");

    root.add({"monitor", "set"},
             "command", "Configures the component");

    root.add({"monitor", "set", "palette"},
             "key", "Selects the color palette",
             &RetroShell::exec <Token::monitor, Token::set, Token::palette>, 1);

    root.add({"monitor", "set", "brightness"},
             "key", "Adjusts the brightness of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::brightness>, 1);

    root.add({"monitor", "set", "contrast"},
             "key", "Adjusts the contrast of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::contrast>, 1);

    root.add({"monitor", "set", "saturation"},
             "key", "Adjusts the saturation of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::saturation>, 1);

    
    //
    // Audio
    //
    
    root.add({"audio"},
             "component", "Audio Unit (Paula)");
    
    root.add({"audio", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::audio, Token::config>);

    root.add({"audio", "set"},
             "command", "Configures the component");

    root.add({"audio", "set", "sampling"},
             "key", "Selects the sampling method",
             &RetroShell::exec <Token::audio, Token::set, Token::sampling>, 1);

    root.add({"audio", "set", "filter"},
             "key", "Configures the audio filter",
             &RetroShell::exec <Token::audio, Token::set, Token::filter>, 1);
    
    root.add({"audio", "set", "volume"},
             "key", "Sets the volume");

    root.add({"audio", "set", "volume", "channel0"},
             "key", "Sets the volume for audio channel 0",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 0);
    
    root.add({"audio", "set", "volume", "channel1"},
             "key", "Sets the volume for audio channel 1",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 1);
    
    root.add({"audio", "set", "volume", "channel2"},
             "key", "Sets the volume for audio channel 2",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 2);
    
    root.add({"audio", "set", "volume", "channel3"},
             "key", "Sets the volume for audio channel 3",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 3);
    
    root.add({"audio", "set", "volume", "left"},
             "key", "Sets the master volume for the left speaker",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 4);
    
    root.add({"audio", "set", "volume", "right"},
             "key", "Sets the master volume for the right speaker",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 5);

    root.add({"audio", "set", "pan"},
             "key", "Sets the pan for one of the four audio channels");
    
    root.add({"audio", "set", "pan", "channel0"},
             "key", "Sets the pan for audio channel 0",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 0);
    
    root.add({"audio", "set", "pan", "channel1"},
             "key", "Sets the pan for audio channel 1",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 1);
    
    root.add({"audio", "set", "pan", "channel2"},
             "key", "Sets the pan for audio channel 2",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 2);
    
    root.add({"audio", "set", "pan", "channel3"},
             "key", "Sets the pan for audio channel 3",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 3);

    root.add({"audio", "inspect"},
             "command", "Displays the internal state");

    root.add({"audio", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::audio, Token::inspect, Token::state>);

    root.add({"audio", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::audio, Token::inspect, Token::registers>);
    
    
    //
    // Paula
    //
    
    root.add({"paula"},
             "component", "Custom chip");

    root.add({"paula", "inspect"},
             "command", "Displays the internal state");

    root.add({"paula", "inspect", "state"},
             "command", "Displays the current register value",
             &RetroShell::exec <Token::paula, Token::inspect, Token::state>);

    root.add({"paula", "inspect", "registers"},
             "command", "Displays the current register value",
             &RetroShell::exec <Token::paula, Token::inspect, Token::registers>);

    
    //
    // RTC
    //

    root.add({"rtc"},
             "component", "Real-time clock");

    root.add({"rtc", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::rtc, Token::config>);

    root.add({"rtc", "set"},
             "command", "Configures the component");
        
    root.add({"rtc", "set", "revision"},
             "key", "Selects the emulated chip model",
             &RetroShell::exec <Token::rtc, Token::set, Token::revision>, 1);

    root.add({"rtc", "inspect"},
             "command", "Displays the internal state");

    root.add({"rtc", "inspect", "registers"},
             "command", "Displays the current register value",
             &RetroShell::exec <Token::rtc, Token::inspect, Token::registers>);

    
    //
    // Control port
    //
    
    root.add({"controlport1"},
             "component", "Control port 1");
    
    root.add({"controlport2"},
             "component", "Control port 2");

    root.add({"controlport1", "controlport2"}, {"", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::controlport, Token::config>);
    
    /*
    root.add({"controlport1", "controlport2"}, {"", "connect"},
             "command", "Connects a device");

    root.add({"controlport1", "controlport2"}, {"", "connect", "joystick"},
             "device", "Connects a joystick",
             &RetroShell::exec <Token::controlport, Token::connect, Token::joystick>, 1);

    root.add({"controlport1", "controlport2"}, {"", "connect", "keyset"},
             "device", "Connects a joystick keyset",
             &RetroShell::exec <Token::controlport, Token::connect, Token::keyset>, 1);

    root.add({"controlport1", "controlport2"}, {"", "connect", "mouse"},
             "device", "Connects a mouse",
             &RetroShell::exec <Token::controlport, Token::connect, Token::mouse>, 1);
    */
    
    root.add({"controlport1", "controlport2"}, {"", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::controlport, Token::inspect>);
    

    //
    // Keyboard
    //

    root.add({"keyboard"},
             "component", "Keyboard");

    root.add({"keyboard", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::keyboard, Token::config>);
    
    root.add({"keyboard", "set"},
             "command", "Configures the component");
        
    root.add({"keyboard", "set", "accuracy"},
             "key", "Determines the emulation accuracy level",
             &RetroShell::exec <Token::keyboard, Token::set, Token::accuracy>, 1);

    root.add({"keyboard", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::keyboard, Token::inspect>);

    
    //
    // Mouse
    //

    root.add({"mouse"},
             "component", "Mouse");

    root.add({"mouse", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::mouse, Token::config>);
    
    root.add({"mouse", "set"},
             "command", "Configures the component");
        
    root.add({"mouse", "set", "pullup"},
             "key", "Enables or disables the emulation of pull-up resistors",
             &RetroShell::exec <Token::mouse, Token::set, Token::pullup>, 1);

    root.add({"mouse", "set", "shakedetector"},
             "key", "Enables or disables the shake detector",
             &RetroShell::exec <Token::mouse, Token::set, Token::shakedetector>, 1);

    root.add({"mouse", "set", "velocity"},
             "key", "Sets the horizontal and vertical mouse velocity",
             &RetroShell::exec <Token::mouse, Token::set, Token::velocity>, 1);

    root.add({"mouse", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::mouse, Token::inspect>);
    
    
    //
    // Serial port
    //
    
    root.add({"serial"},
             "component", "Serial port");

    root.add({"serial", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::serial, Token::config>);

    root.add({"serial", "set"},
             "command", "Configures the component");
        
    root.add({"serial", "set", "device"},
             "key", "",
             &RetroShell::exec <Token::serial, Token::set, Token::device>, 1);

    root.add({"serial", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::serial, Token::inspect>);

    
    //
    // Disk controller
    //
    
    root.add({"diskcontroller"},
             "component", "Disk Controller");

    root.add({"diskcontroller", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::dc, Token::config>);

    root.add({"diskcontroller", "set"},
             "command", "Configures the component");
        
    root.add({"diskcontroller", "set", "speed"},
             "key", "Configures the drive speed",
             &RetroShell::exec <Token::dc, Token::speed>, 1);

    root.add({"diskcontroller", "dsksync"},
             "command", "Secures the DSKSYNC register");

    root.add({"diskcontroller", "dsksync", "auto"},
             "key", "Always receive a SYNC signal",
             &RetroShell::exec <Token::dc, Token::dsksync, Token::autosync>, 1);

    root.add({"diskcontroller", "dsksync", "lock"},
             "key", "Prevents writes to DSKSYNC",
             &RetroShell::exec <Token::dc, Token::dsksync, Token::lock>, 1);
        
    root.add({"diskcontroller", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::dc, Token::inspect>);


    //
    // Df0, Df1, Df2, Df3
    //
    
    root.add({"df0"},
             "component", "Floppy drive 0");

    root.add({"df1"},
             "component", "Floppy drive 1");

    root.add({"df2"},
             "component", "Floppy drive 2");

    root.add({"df3"},
             "component", "Floppy drive 3");

    root.add({"dfn"},
             "component", "All connected drives");

    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::dfn, Token::config>);

    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "connect"},
             "command", "Connects the drive",
             &RetroShell::exec <Token::dfn, Token::connect>);
    root.seek("df0")->remove("connect");

    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "disconnect"},
             "command", "Disconnects the drive",
             &RetroShell::exec <Token::dfn, Token::disconnect>);
    root.seek("df0")->remove("disconnect");

    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "eject"},
             "command", "Ejects a floppy disk",
             &RetroShell::exec <Token::dfn, Token::eject>);

    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "insert"},
             "command", "Inserts a floppy disk",
             &RetroShell::exec <Token::dfn, Token::insert>, 1);

    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "set"},
             "command", "Configures the component");
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "set", "model"},
             "key", "Selects the drive model",
             &RetroShell::exec <Token::dfn, Token::set, Token::model>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "set", "mechanics"},
             "key", "Enables or disables the emulation of mechanical delays",
             &RetroShell::exec <Token::dfn, Token::set, Token::mechanics>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "set", "searchpath"},
             "key", "Sets the search path for media files",
             &RetroShell::exec <Token::dfn, Token::set, Token::searchpath>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "set", "defaultfs"},
             "key", "Determines the default file system type for blank disks",
             &RetroShell::exec <Token::dfn, Token::set, Token::defaultfs>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "set", "defaultbb"},
             "key", "Determines the default boot block type for blank disks",
             &RetroShell::exec <Token::dfn, Token::set, Token::defaultbb>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "set", "pan"},
             "key", "Sets the pan for drive sounds",
             &RetroShell::exec <Token::dfn, Token::set, Token::pan>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "audiate"},
             "command", "Sets the volume of drive sounds",
             &RetroShell::exec <Token::dfn, Token::set, Token::mechanics>);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "audiate", "insert"},
             "command", "Makes disk insertions audible",
             &RetroShell::exec <Token::dfn, Token::audiate, Token::insert>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "audiate", "eject"},
             "command", "Makes disk ejections audible",
             &RetroShell::exec <Token::dfn, Token::audiate, Token::eject>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "audiate", "step"},
             "command", "Makes disk ejections audible",
             &RetroShell::exec <Token::dfn, Token::audiate, Token::step>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "audiate", "poll"},
             "command", "Makes polling clicks audible",
             &RetroShell::exec <Token::dfn, Token::audiate, Token::poll>, 1);
    
    root.add({"df0", "df1", "df2", "df3", "dfn"}, {"", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::dfn, Token::inspect>);
}
