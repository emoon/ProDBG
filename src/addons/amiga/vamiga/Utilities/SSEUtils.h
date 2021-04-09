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

namespace util {

/* Transposes a 8 x 16 bit matrix using SSE3 extensions.
 *
 *     Input:   A pointer to a u16[8] array.
 *              Each array element stores a row of the matrix.
 *     Output:  A pointer to a u8[16] array.
 *              Array element at index i will contain the value on the i-column.
 *              The least significant bit comes from the first row.
 *
 *     Example: Input:  0xFF00, 0xF0F0, 0xCCCC, 0xAAAA, 0x8181
 *
 *                      0xFF00 -> 11111111 00000000
 *                      0xF0F0 -> 11110000 11110000
 *                      0xCCCC -> 11001100 11001100
 *                      0xAAAA -> 10101010 10101010
 *                      0x8181 -> 10000001 10000001
 *
 *                                        | Column values
 *                                        v
 *              Output: 31, 7, 11, 3, 13, 5, 9, 17, 30, 6, 10, 2, 12, 4, 8, 16
 */
void transposeSSE(u16 *source, u8* target);

}
