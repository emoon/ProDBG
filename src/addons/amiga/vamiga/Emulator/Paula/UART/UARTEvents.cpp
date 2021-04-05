// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
UART::serviceTxdEvent(EventID id)
{
    // debug(SER_DEBUG, "serveTxdEvent(%d)\n", id);

    switch (id) {

        case TXD_BIT:

            // This event should not occurr if the shift register is empty
            assert(!shiftRegEmpty());

            // Shift out bit and let it appear on the TXD line
            trace(SER_DEBUG, "Transmitting bit %d\n", transmitShiftReg & 1);
            outBit = transmitShiftReg & 1;
            transmitShiftReg >>= 1;
            updateTXD();

            // Check if the shift register is empty
            if (!transmitShiftReg) {

                // Check if there is a new data packet to send
                if (transmitBuffer) {

                    // Copy new packet into shift register
                    // debug("Transmission continues with packet %X '%c'\n", transmitBuffer, transmitBuffer & 0xFF);
                    copyToTransmitShiftRegister();

                } else {

                    // Abort the transmission
                    trace(SER_DEBUG, "End of transmission\n");
                    agnus.cancel<TXD_SLOT>();
                    break;
                }
            }

            // Schedule the next event
            agnus.scheduleRel<TXD_SLOT>(rate(), TXD_BIT);
            break;

        default:
            assert(false);
    }
}

void
UART::serviceRxdEvent(EventID id)
{
    // debug(SER_DEBUG, "serveRxdEvent(%d)\n", id);

    bool rxd = serialPort.getRXD();
    // debug(SER_DEBUG, "Receiving bit %d: %d\n", recCnt, rxd);

    // Shift in bit from RXD line
    REPLACE_BIT(receiveShiftReg, recCnt++, rxd);

    // Check if this was the last bit to receive
    if (recCnt >= packetLength() + 2) {

        // Copy shift register contents into the receive buffer
        copyFromReceiveShiftRegister();
        trace(SER_DEBUG, "Received packet %X\n", receiveBuffer);

        // Stop receiving if the last bit was a stop bit
        if (rxd) {
            agnus.cancel<RXD_SLOT>();
            return;
        } else {
            // Prepare for the next packet
            recCnt = 0;
        }
    }

    // Schedule the next reception event
    agnus.scheduleRel<RXD_SLOT>(rate(), RXD_BIT);
}
