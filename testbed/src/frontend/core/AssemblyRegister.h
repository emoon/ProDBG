#pragma once

#include <stdint.h>

struct PDReader;

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

union AssemblyRegisterValue
{
    double d;
    uint64_t u64;
    float f;
    uint32_t u32;
    uint16_t u16;
    uint8_t u8;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum AssemblyRegisterType
{
    AssemblyRegisterType_u8,
    AssemblyRegisterType_u16,
    AssemblyRegisterType_u32,
    AssemblyRegisterType_u64,
    AssemblyRegisterType_float,
    AssemblyRegisterType_double
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct AssemblyRegister
{
    AssemblyRegisterValue value;    // the actual value of the registr (\todo support more than one value)
    AssemblyRegisterType type;        // Register type
    char name[64];                    // Name of the register
    int nameLength;                    // length of the name
    int count;                        // number of "internal" registers (4 x u32 for SSE for example)
    uint16_t readOnly;                // Set if the register is read-only (can't be changed by user)
    uint16_t statusFlags;            // Flags (usually carry, overflow, etc)
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AssemblyRegister* AssemblyRegister_buildFromReader(PDReader* reader, AssemblyRegister* currentArray, int* count);

}

