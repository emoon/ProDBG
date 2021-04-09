// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Error.h"

const char *
VAError::what() const throw()
{
    return  ErrorCodeEnum::key(data);
}

const char *
ConfigError::what() const throw()
{
    return  description.c_str();
}
