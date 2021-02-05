// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Concurrency.h"

Mutex::Mutex()
{
    pthread_mutex_init(&mutex, nullptr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mutex);
}

int
Mutex::lock()
{
    return pthread_mutex_lock(&mutex);
}

int
Mutex::unlock()
{
    return pthread_mutex_unlock(&mutex);
}
