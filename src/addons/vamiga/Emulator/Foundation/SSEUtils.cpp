// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "SSEUtils.h"

#if (defined(__i386__) || defined(__x86_64__)) && defined(__MACH__)

#include <x86intrin.h>

void transposeSSE(u16 *source, u8* target)
{
    // We receive the matrix rows in little endian format
    // 0.lo 0.hi 1.lo 1.hi 2.lo 2.hi 3.lo 3.hi  ........  7.lo 7.hi
    //
    // Rearrange the byte order to
    // 0.hi 1.hi 2.hi 3.hi ...  7.hi 0.lo 1.lo 2.lo 3.lo  ...  7.lo
    
    const u8 mask1[16] = { 1,3,5,7,9,11,13,15,0,2,4,6,8,10,12,14 };
    __m128i shuffled = _mm_shuffle_epi8(*(__m128i *)source, *(__m128i *)mask1);
    
    // Cut off column values in the order
    // col0 col8 col1 col9 col2 col10 col3 col11 ... col7 col15
    
    union { u16 column[8]; __m128i sse; } result;
    for (unsigned i = 0; i < 8; i++) {
        result.column[i] = _mm_movemask_epi8(shuffled);
        shuffled = _mm_slli_epi64(shuffled, 1);
    }
    
    // Shuffle back to
    // col0 col1 col2 col3 col4 col5 col6 col7 ...  col14 col15
    
    const u8 mask2[16] = { 0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15 };
    shuffled = _mm_shuffle_epi8(result.sse, *(__m128i *)mask2);
    
    // Read the result back from the SSE registers
    _mm_store_si128((__m128i *)target, shuffled);
}

#else

void transposeSSE(u16 *source, u8* target)
{
    assert(false);
}

#endif
