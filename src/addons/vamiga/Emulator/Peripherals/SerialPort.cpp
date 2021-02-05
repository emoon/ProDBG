// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

SerialPort::SerialPort(Amiga& ref) : AmigaComponent(ref)
{
    config.device = SPD_LOOPBACK;
}

long
SerialPort::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_SERIAL_DEVICE: return (long)config.device;
        
        default:
            assert(false);
            return 0;
    }
}

bool
SerialPort::setConfigItem(Option option, long value)
{
    switch (option) {
            
        case OPT_SERIAL_DEVICE:
            
            if (!isSerialPortDevice(value)) {
                warn("Invalid serial port device: %ld\n", value);
                return false;
            }
            if (config.device == value) {
                return false;
            }
            
            config.device = (SerialPortDevice)value;
            return true;
                        
        default:
            return false;
    }
}

void
SerialPort::_inspect()
{
    synchronized {
        
        info.port = port;
        info.txd = getTXD();
        info.rxd = getRXD();
        info.rts = getRTS();
        info.cts = getCTS();
        info.dsr = getDSR();
        info.cd = getCD();
        info.dtr = getDTR();
    }
}

void
SerialPort::_dump() const
{
    msg("    device: %ld\n", (long)config.device);
    msg("      port: %X\n", port);
}

bool
SerialPort::getPin(isize nr) const
{
    assert(nr >= 1 && nr <= 25);

    bool result = GET_BIT(port, nr);

    // debug(SER_DEBUG, "getPin(%d) = %d port = %X\n", nr, result, port);
    return result;
}

void
SerialPort::setPin(isize nr, bool value)
{
    // debug(SER_DEBUG, "setPin(%d,%d)\n", nr, value);
    assert(nr >= 1 && nr <= 25);

    setPort(1 << nr, value);
}

void
SerialPort::setPort(u32 mask, bool value)
{
    u32 oldPort = port;

    /* Emulate the loopback cable (if connected)
     *
     *     Connected pins: A: 2 - 3       (TXD - RXD)
     *                     B: 4 - 5 - 6   (RTS - CTS - DSR)
     *                     C: 8 - 20 - 22 (CD - DTR - RI)
     */
    if (config.device == SPD_LOOPBACK) {

        u32 maskA = TXD_MASK | RXD_MASK;
        u32 maskB = RTS_MASK | CTS_MASK | DSR_MASK;
        u32 maskC = CD_MASK | DTR_MASK | RI_MASK;

        if (mask & maskA) mask |= maskA;
        if (mask & maskB) mask |= maskB;
        if (mask & maskC) mask |= maskC;
    }

    // Change the port pins
    if (value) port |= mask; else port &= ~mask;

    // Let the UART know if RXD has changed
    if ((oldPort ^ port) & RXD_MASK) uart.rxdHasChanged(value);
}

