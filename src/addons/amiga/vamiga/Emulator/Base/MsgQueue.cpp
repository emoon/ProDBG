// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "MsgQueue.h"

void
MsgQueue::setListener(const void *listener, Callback *callback)
{
    synchronized {
        
        this->listener = listener;
        this->callback = callback;
        
        // Send all pending messages
        while (!queue.isEmpty()) {
            Message &msg = queue.read();
            callback(listener, msg.type, msg.data);
        }
        put(MSG_REGISTER);
    }
}

void
MsgQueue::put(MsgType type, long data)
{
    synchronized {
        
        debug(QUEUE_DEBUG, "%s [%ld]\n", MsgTypeEnum::key(type), data);
        
        // Send the message immediately if a lister has been registered
        if (listener) { callback(listener, type, data); return; }
        
        // Otherwise, store it in the ring buffer
        Message msg = { type, data }; queue.write(msg);
    }
}
