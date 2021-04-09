// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include <utility>

namespace util {

/* The emulator uses buffers at various places. Most of them are derived from
 * one of the following two classes:
 *
 *           RingBuffer : A standard ringbuffer data structure
 *     SortedRingBuffer : A ringbuffer that keeps the entries sorted
 */

template <class T, isize capacity> struct RingBuffer
{
    // Element storage
    T elements[capacity];

    // Read and write pointers
    isize r, w;

    
    //
    // Initializing
    //

    RingBuffer() { clear(); }
    
    void clear() { r = w = 0; }
    void clear(T t) { for (isize i = 0; i < capacity; i++) elements[i] = t; clear(); }
    void align(isize offset) { w = (r + offset) % capacity; }

    
    //
    // Querying the fill status
    //

    isize cap() const { return capacity; }
    isize count() const { return (capacity + w - r) % capacity; }
    double fillLevel() const { return (double)count() / capacity; }
    bool isEmpty() const { return r == w; }
    bool isFull() const { return count() == capacity - 1; }

    
    //
    // Working with indices
    //

    isize begin() const { return r; }
    isize end() const { return w; }
    static int next(isize i) { return (capacity + i + 1) % capacity; }
    static int prev(isize i) { return (capacity + i - 1) % capacity; }


    //
    // Reading and writing elements
    //

    const T& current() const
    {
        return elements[r];
    }

    T *currentAddr()
    {
        return &elements[r];
    }

    const T& current(isize offset) const
    {
        return elements[(r + offset) % capacity];
    }
    
    T& read()
    {
        assert(!isEmpty());

        i64 oldr = r;
        r = next(r);
        return elements[oldr];
    }

    void write(T element)
    {
        assert(!isFull());

        isize oldw = w;
        w = next(w);
        elements[oldw] = element;
    }
    
    void skip(isize n)
    {
        r = (r + n) % capacity;
    }
    
    //
    // Examining the element storage
    //

};

template <class T, isize capacity>
struct SortedRingBuffer : public RingBuffer<T, capacity>
{
    // Key storage
    i64 keys[capacity];
    
    // Inserts an element at the proper position
    void insert(i64 key, T element)
    {
        assert(!this->isFull());

        // Add the new element
        isize oldw = this->w;
        this->write(element);
        keys[oldw] = key;

        // Keep the elements sorted
        while (oldw != this->r) {

            // Get the index of the preceeding element
            isize p = this->prev(oldw);

            // Exit the loop once we've found the correct position
            if (key >= keys[p]) break;

            // Otherwise, swap elements
            std::swap(this->elements[oldw], this->elements[p]);
            std::swap(keys[oldw], keys[p]);
            oldw = p;
        }
    }
};

}
