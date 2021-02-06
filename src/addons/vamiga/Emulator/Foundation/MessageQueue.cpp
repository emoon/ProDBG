// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "MessageQueue.h"

MessageQueue::MessageQueue()
{
    setDescription("MessageQueue");
}

void
MessageQueue::addListener(const void *listener, Callback *func)
{
    synchronized {
        listeners.insert(pair <const void *, Callback *> (listener, func));
    }
    
    // Distribute all pending messages
    Message msg;
    while ((msg = get()).type != MSG_NONE) {
        propagate(&msg);
    }

    put(MSG_REGISTER);
}

void
MessageQueue::removeListener(const void *listener)
{
    put(MSG_UNREGISTER);

    synchronized { listeners.erase(listener); }
}

Message
MessageQueue::get()
{ 
	Message result;
    
    synchronized {
        
        if (r == w) {
            result.type = MSG_NONE; // Queue is empty
            result.data = 0;
        } else {
            result = queue[r];
            r = (r + 1) % capacity;
        }
    }
    
	return result; 
}

void
MessageQueue::put(MessageType type, u64 data)
{
    synchronized {
        
        // Write data
        Message msg;
        msg.type = type;
        msg.data = data;
        queue[w] = msg;
        
        // Move write pointer to next location
        w = (w + 1) % capacity;
        
        if (w == r) {
            // warn("Queue overflow. Oldest message is lost.\n");
            r = (r + 1) % capacity;
        }
        
        // Serve registered callbacks
        propagate(&msg);
    }
}

void
MessageQueue::propagate(Message *msg)
{
    map <const void *, Callback *> :: iterator i;
    
    for (i = listeners.begin(); i != listeners.end(); i++) {
        i->second(i->first, msg->type, msg->data);
    }
}
