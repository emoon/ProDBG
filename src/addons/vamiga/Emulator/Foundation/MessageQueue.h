// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _MESSAGE_QUEUE_H
#define _MESSAGE_QUEUE_H

#include "AmigaObject.h"

class MessageQueue : public AmigaObject {
        
    // Maximum number of queued messages
    const static size_t capacity = 64;
    
    // Ring buffer storing all pending messages
    Message queue[capacity];
    
    // The ring buffer's read and write pointers
    int r = 0;
    int w = 0;
        
    // List of all registered listeners
    map <const void *, Callback *> listeners;
    
public:
    
    MessageQueue();
    
    // Registers a listener together with it's callback function
    void addListener(const void *listener, Callback *func);
    
    // Unregisters a listener
    void removeListener(const void *listener);
    
    // Returns the next pending message, or NULL if the queue is empty
    Message get();
    
    // Writes a message into the queue and propagates it to all listeners
    void put(MessageType type, u64 data = 0);
    
private:
    
    // Used by 'put' to propagates a single message to all registered listeners
    void propagate(Message *msg);
};

#endif
