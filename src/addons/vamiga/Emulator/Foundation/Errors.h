// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "AmigaTypes.h"
#include <exception>

struct VAError : public std::exception
{
    ErrorCode errorCode;
    
    VAError(ErrorCode code) : errorCode(code) { }
    
    const char *what() const throw() override {

        return  ErrorCodeEnum::key(errorCode);
    }
};
