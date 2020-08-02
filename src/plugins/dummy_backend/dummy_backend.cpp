#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pd_backend.h"
#include "pd_host.h"
#include "pd_io.h"
//#include <tinyexpr/tinyexpr.h>

#define sizeof_array(t) (sizeof(t) / sizeof(t[0]))

typedef struct DisasmData {
    uint16_t address;
    const char* string;
} DisasmData;

static DisasmData s_disasm_data[] = {
    {0x0000e003, "jmp 0xe0c1"},   {0x0000e006, "rti"},          {0x0000e007, "rti"},
    {0x0000e008, "rti"},          {0x0000e009, "rti"},          {0x0000e00a, "rti"},
    {0x0000e00b, "lsr a"},        {0x0000e00c, "lsr a"},        {0x0000e00d, "lsr 0x4e4e"},
    {0x0000e010, "lsr 0x5757"},   {0x0000e013, "sre 0x57,x"},   {0x0000e015, "sre 0x61,x"},
    {0x0000e017, "rra 0x67"},     {0x0000e019, "nop 0x64"},     {0x0000e01b, "php"},
    {0x0000e01c, "ora 0x00"},     {0x0000e01e, "brk"},          {0x0000e01f, "brk"},
    {0x0000e020, "eor (0x6c,x)"}, {0x0000e022, "adc 0x78"},     {0x0000e024, "adc (0x6e,x)"},
    {0x0000e026, "nop 0x65"},     {0x0000e028, "hlt"},          {0x0000e029, "jsr 0x6957"},
    {0x0000e02c, "arr #0x6c"},    {0x0000e02e, "adc 0x6e,x"},   {0x0000e030, "nop 0x20"},
    {0x0000e032, "plp"},          {0x0000e033, "sre 0x69,x"},   {0x0000e035, "arr #0x6c"},
    {0x0000e037, "adc 0x6e,x"},   {0x0000e039, "nop 0x29"},     {0x0000e03b, "jsr 0x2020"},
    {0x0000e03e, "jsr 0x9d20"},   {0x0000e041, "adc #0xe3"},    {0x0000e043, "lda 0xe358,x"},
    {0x0000e046, "sta 0xe368,x"}, {0x0000e049, "rts"},          {0x0000e04a, "sta 0xe3a8,x"},
    {0x0000e04d, "rts"},          {0x0000e04e, "ldy #0x00"},    {0x0000e050, "sty 0xe0fc"},
    {0x0000e053, "sta 0xe0f8"},   {0x0000e056, "rts"},          {0x0000e057, "sta 0xe37f"},
    {0x0000e05a, "sta 0xe386"},   {0x0000e05d, "sta 0xe38d"},   {0x0000e060, "rts"},
    {0x0000e061, "jmp 0xe262"},   {0x0000e064, "tya"},          {0x0000e065, "beq 0x00e0b7"},
    {0x0000e067, "lda 0xe5c5,y"}, {0x0000e06a, "sta 0xff"},     {0x0000e06c, "lda 0xe368,x"},
    {0x0000e06f, "cmp #0x02"},    {0x0000e071, "bcc 0x00e090"}, {0x0000e073, "beq 0x00e0a7"},
    {0x0000e075, "ldy 0x1381,x"}, {0x0000e078, "lda 0x1395,x"}, {0x0000e07b, "sbc 0x13b5,y"},
    {0x0000e07e, "pha"},          {0x0000e07f, "lda 0x1396,x"}, {0x0000e082, "sbc 0x140e,y"},
    {0x0000e085, "tay"},          {0x0000e086, "pla"},          {0x0000e087, "bcs 0x00e0a0"},
    {0x0000e089, "adc 0xfe"},     {0x0000e08b, "tya"},          {0x0000e08c, "adc 0xff"},
    {0x0000e08e, "bpl 0x00e0b7"}, {0x0000e090, "lda 0xe395,x"}, {0x0000e093, "adc 0xfe"},
    {0x0000e095, "sta 0xe395,x"}, {0x0000e098, "lda 0xe396,x"}, {0x0000e09b, "adc 0xff"},
    {0x0000e09d, "jmp 0xe25f"},   {0x0000e0a0, "sbc 0xfe"},     {0x0000e0a2, "tya"},
    {0x0000e0a3, "sbc 0xff"},     {0x0000e0a5, "bmi 0x00e0b7"}, {0x0000e0a7, "lda 0xe395,x"},
    {0x0000e0aa, "sbc 0xfe"},     {0x0000e0ac, "sta 0xe395,x"}, {0x0000e0af, "lda 0xe396,x"},
    {0x0000e0b2, "sbc 0xff"},     {0x0000e0b4, "jmp 0xe25f"},   {0x0000e0b7, "ldy 0xe381,x"},
    {0x0000e0ba, "jmp 0xe256"},   {0x0000e0bd, "sta 0xe0c4"},   {0x0000e0c0, "rts"},
    {0x0000e0c1, "ldx #0x00"},    {0x0000e0c3, "ldy #0x00"},    {0x0000e0c5, "bmi 0x00e0f7"},
    {0x0000e0c7, "txa"},          {0x0000e0c8, "ldx #0x29"},    {0x0000e0ca, "sta 0xe353,x"},
    {0x0000e0cd, "dex"},          {0x0000e0ce, "bpl 0x00e0ca"}, {0x0000e0d0, "sta 0xd415"},
    {0x0000e0d3, "sta 0xe146"},   {0x0000e0d6, "sta 0xe0f8"},   {0x0000e0d9, "stx 0xe0c4"},
    {0x0000e0dc, "tax"},          {0x0000e0dd, "jsr 0xe0e7"},   {0x0000e0e0, "ldx #0x07"},
    {0x0000e0e2, "jsr 0xe0e7"},   {0x0000e0e5, "ldx #0x0e"},    {0x0000e0e7, "lda #0x05"},
    {0x0000e0e9, "sta 0xe37f,x"}, {0x0000e0ec, "lda #0x01"},    {0x0000e0ee, "sta 0xe380,x"},
    {0x0000e0f1, "sta 0xe382,x"}, {0x0000e0f4, "jmp 0xe349"},   {0x0000e0f7, "ldy #0x00"},
    {0x0000e0f9, "beq 0x00e140"}, {0x0000e0fb, "lda #0x00"},    {0x0000e0fd, "bne 0x00e122"},
    {0x0000e0ff, "lda 0xe596,y"}, {0x0000e102, "beq 0x00e116"}, {0x0000e104, "bpl 0x00e11f"},
    {0x0000e106, "asl a"},        {0x0000e107, "sta 0xe14b"},   {0x0000e10a, "lda 0xe5ad,y"},
    {0x0000e10d, "sta 0xe146"},   {0x0000e110, "lda 0xe597,y"}, {0x0000e113, "bne 0x00e134"},
    {0x0000e115, "iny"},          {0x0000e116, "lda 0xe5ad,y"}, {0x0000e119, "sta 0xe141"},
    {0x0000e11c, "jmp 0xe131"},   {0x0000e11f, "sta 0xe0fc"},   {0x0000e122, "lda 0xe5ad,y"},
    {0x0000e125, "clc"},          {0x0000e126, "adc 0xe141"},   {0x0000e129, "sta 0xe141"},
    {0x0000e12c, "dec 0xe0fc"},   {0x0000e12f, "bne 0x00e142"}, {0x0000e131, "lda 0xe597,y"},
    {0x0000e134, "cmp #0xff"},    {0x0000e136, "iny"},          {0x0000e137, "tya"},
    {0x0000e138, "bcc 0x00e13d"}, {0x0000e13a, "lda 0xe5ad,y"}, {0x0000e13d, "sta 0xe0f8"},
    {0x0000e140, "lda #0x00"},    {0x0000e142, "sta 0xd416"},   {0x0000e145, "lda #0x00"},
    {0x0000e147, "sta 0xd417"},   {0x0000e14a, "lda #0x00"},    {0x0000e14c, "ora #0x0f"},
    {0x0000e14e, "sta 0xd418"},   {0x0000e151, "jsr 0xe15b"},   {0x0000e154, "ldx #0x07"},
    {0x0000e156, "jsr 0xe15b"},   {0x0000e159, "ldx #0x0e"},    {0x0000e15b, "dec 0xe380,x"},
    {0x0000e15e, "beq 0x00e16b"}, {0x0000e160, "bpl 0x00e168"}, {0x0000e162, "lda 0xe37f,x"},
    {0x0000e165, "sta 0xe380,x"}, {0x0000e168, "jmp 0xe213"},   {0x0000e16b, "ldy 0xe358,x"},
    {0x0000e16e, "lda 0xe006,y"}, {0x0000e171, "sta 0xe208"},   {0x0000e174, "sta 0xe211"},
    {0x0000e177, "lda 0xe356,x"}, {0x0000e17a, "bne 0x00e1ac"}, {0x0000e17c, "ldy 0xe37d,x"},
    {0x0000e17f, "lda 0xe46e,y"}, {0x0000e182, "sta 0xfe"},     {0x0000e184, "lda 0xe471,y"},
    {0x0000e187, "sta 0xff"},     {0x0000e189, "ldy 0xe353,x"}, {0x0000e18c, "lda (0xfe),y"},
    {0x0000e18e, "cmp #0xff"},    {0x0000e190, "bcc 0x00e198"}, {0x0000e192, "iny"},
    {0x0000e193, "lda (0xfe),y"}, {0x0000e195, "tay"},          {0x0000e196, "lda (0xfe),y"},
    {0x0000e198, "cmp #0xe0"},    {0x0000e19a, "bcc 0x00e1a4"}, {0x0000e19c, "sbc #0xf0"},
    {0x0000e19e, "sta 0xe354,x"}, {0x0000e1a1, "iny"},          {0x0000e1a2, "lda (0xfe),y"},
    {0x0000e1a4, "sta 0xe37e,x"}, {0x0000e1a7, "iny"},          {0x0000e1a8, "tya"},
    {0x0000e1a9, "sta 0xe353,x"}, {0x0000e1ac, "ldy 0xe382,x"}, {0x0000e1af, "lda 0xe4e6,y"},
    {0x0000e1b2, "sta 0xe3ac,x"}, {0x0000e1b5, "lda 0xe36a,x"}, {0x0000e1b8, "beq 0x00e20d"},
    {0x0000e1ba, "sec"},          {0x0000e1bb, "sbc #0x60"},    {0x0000e1bd, "sta 0xe381,x"},
    {0x0000e1c0, "lda #0x00"},    {0x0000e1c2, "sta 0xe368,x"}, {0x0000e1c5, "sta 0xe36a,x"},
    {0x0000e1c8, "lda 0xe358,x"}, {0x0000e1cb, "cmp #0x03"},    {0x0000e1cd, "beq 0x00e20d"},
    {0x0000e1cf, "lda 0xe4f1,y"}, {0x0000e1d2, "sta 0xe36c,x"}, {0x0000e1d5, "inc 0xe383,x"},
    {0x0000e1d8, "lda 0xe4d0,y"}, {0x0000e1db, "beq 0x00e1e5"}, {0x0000e1dd, "sta 0xe36d,x"},
    {0x0000e1e0, "lda #0x00"},    {0x0000e1e2, "sta 0xe36e,x"}, {0x0000e1e5, "lda 0xe4db,y"},
    {0x0000e1e8, "beq 0x00e1f2"}, {0x0000e1ea, "sta 0xe0f8"},   {0x0000e1ed, "lda #0x00"},
    {0x0000e1ef, "sta 0xe0fc"},   {0x0000e1f2, "lda 0xe4c5,y"}, {0x0000e1f5, "sta 0xe36b,x"},
    {0x0000e1f8, "lda 0xe4ba,y"}, {0x0000e1fb, "sta 0xe3a8,x"}, {0x0000e1fe, "lda 0xe4af,y"},
    {0x0000e201, "sta 0xe3a7,x"}, {0x0000e204, "lda 0xe359,x"}, {0x0000e207, "jsr 0xe040"},
    {0x0000e20a, "jmp 0xe328"},   {0x0000e20d, "lda 0xe359,x"}, {0x0000e210, "jsr 0xe040"},
    {0x0000e213, "ldy 0xe36b,x"}, {0x0000e216, "beq 0x00e235"}, {0x0000e218, "lda 0xe4fc,y"},
    {0x0000e21b, "beq 0x00e220"}, {0x0000e21d, "sta 0xe36c,x"}, {0x0000e220, "lda 0xe4fd,y"},
    {0x0000e223, "cmp #0xff"},    {0x0000e225, "iny"},          {0x0000e226, "tya"},
    {0x0000e227, "bcc 0x00e22d"}, {0x0000e229, "clc"},          {0x0000e22a, "lda 0xe53c,y"},
    {0x0000e22d, "sta 0xe36b,x"}, {0x0000e230, "lda 0xe53b,y"}, {0x0000e233, "bne 0x00e24e"},
    {0x0000e235, "lda 0xe380,x"}, {0x0000e238, "beq 0x00e265"}, {0x0000e23a, "ldy 0xe368,x"},
    {0x0000e23d, "lda 0xe016,y"}, {0x0000e240, "sta 0xe24c"},   {0x0000e243, "ldy 0xe369,x"},
    {0x0000e246, "lda 0xe5c8,y"}, {0x0000e249, "sta 0xfe"},     {0x0000e24b, "jmp 0xe061"},
    {0x0000e24e, "bpl 0x00e255"}, {0x0000e250, "adc 0xe381,x"}, {0x0000e253, "and #0x7f"},
    {0x0000e255, "tay"},          {0x0000e256, "lda 0xe3b5,y"}, {0x0000e259, "sta 0xe395,x"},
    {0x0000e25c, "lda 0xe40e,y"}, {0x0000e25f, "sta 0xe396,x"}, {0x0000e262, "lda 0xe380,x"},
    {0x0000e265, "cmp 0xe3ac,x"}, {0x0000e268, "beq 0x00e2ad"}, {0x0000e26a, "ldy 0xe36d,x"},
    {0x0000e26d, "beq 0x00e2aa"}, {0x0000e26f, "ora 0xe356,x"}, {0x0000e272, "beq 0x00e2aa"},
    {0x0000e274, "lda 0xe36e,x"}, {0x0000e277, "bne 0x00e28a"}, {0x0000e279, "lda 0xe57c,y"},
    {0x0000e27c, "bpl 0x00e287"}, {0x0000e27e, "lda 0xe589,y"}, {0x0000e281, "sta 0xe397,x"},
    {0x0000e284, "jmp 0xe29b"},   {0x0000e287, "sta 0xe36e,x"}, {0x0000e28a, "lda 0xe397,x"},
    {0x0000e28d, "clc"},          {0x0000e28e, "adc 0xe589,y"}, {0x0000e291, "adc #0x00"},
    {0x0000e293, "sta 0xe397,x"}, {0x0000e296, "dec 0xe36e,x"}, {0x0000e299, "bne 0x00e2aa"},
    {0x0000e29b, "lda 0xe57d,y"}, {0x0000e29e, "cmp #0xff"},    {0x0000e2a0, "iny"},
    {0x0000e2a1, "tya"},          {0x0000e2a2, "bcc 0x00e2a7"}, {0x0000e2a4, "lda 0xe589,y"},
    {0x0000e2a7, "sta 0xe36d,x"}, {0x0000e2aa, "jmp 0xe328"},   {0x0000e2ad, "ldy 0xe37e,x"},
    {0x0000e2b0, "lda 0xe474,y"}, {0x0000e2b3, "sta 0xfe"},     {0x0000e2b5, "lda 0xe492,y"},
    {0x0000e2b8, "sta 0xff"},     {0x0000e2ba, "ldy 0xe356,x"}, {0x0000e2bd, "lda (0xfe),y"},
    {0x0000e2bf, "cmp #0x40"},    {0x0000e2c1, "bcc 0x00e2db"}, {0x0000e2c3, "cmp #0x60"},
    {0x0000e2c5, "bcc 0x00e2e5"}, {0x0000e2c7, "cmp #0xc0"},    {0x0000e2c9, "bcc 0x00e2f9"},
    {0x0000e2cb, "lda 0xe357,x"}, {0x0000e2ce, "bne 0x00e2d2"}, {0x0000e2d0, "lda (0xfe),y"},
    {0x0000e2d2, "adc #0x00"},    {0x0000e2d4, "sta 0xe357,x"}, {0x0000e2d7, "beq 0x00e31f"},
    {0x0000e2d9, "bne 0x00e328"}, {0x0000e2db, "sta 0xe382,x"}, {0x0000e2de, "iny"},
    {0x0000e2df, "lda (0xfe),y"}, {0x0000e2e1, "cmp #0x60"},    {0x0000e2e3, "bcs 0x00e2f9"},
    {0x0000e2e5, "cmp #0x50"},    {0x0000e2e7, "and #0x0f"},    {0x0000e2e9, "sta 0xe358,x"},
    {0x0000e2ec, "beq 0x00e2f4"}, {0x0000e2ee, "iny"},          {0x0000e2ef, "lda (0xfe),y"},
    {0x0000e2f1, "sta 0xe359,x"}, {0x0000e2f4, "bcs 0x00e31f"}, {0x0000e2f6, "iny"},
    {0x0000e2f7, "lda (0xfe),y"}, {0x0000e2f9, "cmp #0xbd"},    {0x0000e2fb, "bcc 0x00e303"},
    {0x0000e2fd, "beq 0x00e31f"}, {0x0000e2ff, "ora #0xf0"},    {0x0000e301, "bne 0x00e31c"},
    {0x0000e303, "adc 0xe354,x"}, {0x0000e306, "sta 0xe36a,x"}, {0x0000e309, "lda 0xe358,x"},
    {0x0000e30c, "cmp #0x03"},    {0x0000e30e, "beq 0x00e31f"}, {0x0000e310, "lda #0x00"},
    {0x0000e312, "sta 0xe3a8,x"}, {0x0000e315, "lda #0x0f"},    {0x0000e317, "sta 0xe3a7,x"},
    {0x0000e31a, "lda #0xfe"},    {0x0000e31c, "sta 0xe383,x"}, {0x0000e31f, "iny"},
    {0x0000e320, "lda (0xfe),y"}, {0x0000e322, "beq 0x00e325"}, {0x0000e324, "tya"},
    {0x0000e325, "sta 0xe356,x"}, {0x0000e328, "lda 0xe397,x"}, {0x0000e32b, "sta 0xd402,x"},
    {0x0000e32e, "sta 0xd403,x"}, {0x0000e331, "lda 0xe3a8,x"}, {0x0000e334, "sta 0xd406,x"},
    {0x0000e337, "lda 0xe3a7,x"}, {0x0000e33a, "sta 0xd405,x"}, {0x0000e33d, "lda 0xe395,x"},
    {0x0000e340, "sta 0xd400,x"}, {0x0000e343, "lda 0xe396,x"}, {0x0000e346, "sta 0xd401,x"},
    {0x0000e349, "lda 0xe36c,x"}, {0x0000e34c, "and 0xe383,x"}, {0x0000e34f, "sta 0xd404,x"},
    {0x0000e352, "rts"},
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Register {
    char* name;
    uint8_t size;
    uint8_t read_only;
    void* data;
} Register;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double get_reg_for_expression(const Register* reg) {
    double t = 0.0;
    uint8_t* data = (uint8_t*)reg->data;

    switch (reg->size) {
        case 4: {
            t = (((uint32_t)data[0]) << 24) | (((uint32_t)data[1]) << 16) | (((uint32_t)data[2]) << 8) |
                (((uint32_t)data[3]));
            break;
        }
    }

    return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DummyPlugin {
    int exception_location;
    int prev_exception_location;
    // 1 MB of memory, range is 0x10000
    uint8_t* memory;
    int64_t memory_start;
    int64_t memory_end;
    int register_type;
    Register* registers;
    int registers_count;
} DummyPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fill_register(Register* reg, char* name, uint8_t size, void* initial_data, uint8_t read_only) {
    reg->name = name;
    reg->size = size;
    reg->read_only = read_only;
    reg->data = malloc(size);
    memcpy(reg->data, initial_data, size);
}

void* create_instance(ServiceFunc* serviceFunc) {
    (void)serviceFunc;
    int i = 0;

    DummyPlugin* plugin = (DummyPlugin*)malloc(sizeof(DummyPlugin));
    memset(plugin, 0, sizeof(DummyPlugin));
    plugin->exception_location = s_disasm_data[0].address;
    plugin->prev_exception_location = s_disasm_data[0].address - 1;
    plugin->memory = malloc(1 * 1024 * 1024);
    plugin->memory_start = 0;
    plugin->memory_end = (1 * 1024 * 1024) + plugin->memory_start;

    srand(0xc0cac01a);

    for (i = 0; i < 1024 * 1024; ++i) {
        plugin->memory[i] = rand() & 0xff;
    }

    // Allocate memory for 100 registers. Should be enough for dummy plugin
    plugin->registers = malloc(sizeof(Register[100]));
    int reg_counter = 0;
    fill_register(&plugin->registers[reg_counter++], "eax", 4, (uint8_t[]){0xCA, 0xCB, 0xCC, 0xCD}, 0);
    fill_register(&plugin->registers[reg_counter++], "ebx", 4, (uint8_t[]){0x7E, 0xFD, 0xE0, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "ecx", 4, (uint8_t[]){0x00, 0x01, 0x21, 0x21}, 0);
    fill_register(&plugin->registers[reg_counter++], "edx", 4, (uint8_t[]){0x00, 0xD0, 0x95, 0x80}, 0);
    fill_register(&plugin->registers[reg_counter++], "esi", 4, (uint8_t[]){0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "edi", 4, (uint8_t[]){0x00, 0x39, 0xFA, 0x28}, 0);
    fill_register(&plugin->registers[reg_counter++], "eip", 4, (uint8_t[]){0x00, 0xD0, 0x17, 0xAE}, 0);
    fill_register(&plugin->registers[reg_counter++], "esp", 4, (uint8_t[]){0x00, 0x39, 0xF9, 0x5C}, 0);
    fill_register(&plugin->registers[reg_counter++], "ebp", 4, (uint8_t[]){0x00, 0x39, 0xFA, 0x28}, 0);
    fill_register(&plugin->registers[reg_counter++], "efl", 4, (uint8_t[]){0x00, 0x00, 0x02, 0x00}, 1);

    fill_register(&plugin->registers[reg_counter++], "cs", 2, (uint8_t[]){0x00, 0x23}, 0);
    fill_register(&plugin->registers[reg_counter++], "ds", 2, (uint8_t[]){0x00, 0x2B}, 0);
    fill_register(&plugin->registers[reg_counter++], "es", 2, (uint8_t[]){0x00, 0x2B}, 0);
    fill_register(&plugin->registers[reg_counter++], "ss", 2, (uint8_t[]){0x00, 0x2B}, 0);
    fill_register(&plugin->registers[reg_counter++], "fs", 2, (uint8_t[]){0x00, 0x53}, 0);
    fill_register(&plugin->registers[reg_counter++], "gs", 2, (uint8_t[]){0x00, 0x2B}, 0);

    fill_register(&plugin->registers[reg_counter++], "st0", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "st1", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "st2", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "st3", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "st4", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "st5", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "st6", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "st7", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "ctrl", 2, (uint8_t[]){0x02, 0x7F}, 0);
    fill_register(&plugin->registers[reg_counter++], "stat", 2, (uint8_t[]){0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "tags", 2, (uint8_t[]){0xFF, 0xFF}, 0);
    fill_register(&plugin->registers[reg_counter++], "edo", 4, (uint8_t[]){0x00, 0x00, 0x00, 0x00}, 0);

    fill_register(&plugin->registers[reg_counter++], "mm0", 8,
                  (uint8_t[]){0xB1, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9A}, 0);
    fill_register(&plugin->registers[reg_counter++], "mm1", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "mm2", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "mm3", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "mm4", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "mm5", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "mm6", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "mm7", 8,
                  (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);

    fill_register(
        &plugin->registers[reg_counter++], "xmm0", 16,
        (uint8_t[]){0x40, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(
        &plugin->registers[reg_counter++], "xmm1", 16,
        (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(
        &plugin->registers[reg_counter++], "xmm2", 16,
        (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(
        &plugin->registers[reg_counter++], "xmm3", 16,
        (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(
        &plugin->registers[reg_counter++], "xmm4", 16,
        (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(
        &plugin->registers[reg_counter++], "xmm5", 16,
        (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(
        &plugin->registers[reg_counter++], "xmm6", 16,
        (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(
        &plugin->registers[reg_counter++], "xmm7", 16,
        (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0);
    fill_register(&plugin->registers[reg_counter++], "mxcsr", 4, (uint8_t[]){0x00, 0x00, 0x1F, 0x80}, 0);
    plugin->registers_count = reg_counter;

    return plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroy_instance(void* user_data) {
    free(user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void write_register(PDWriter* writer, Register* reg) {
    PDWrite_array_entry_begin(writer);
    PDWrite_string(writer, "name", reg->name);
    PDWrite_u8(writer, "read_only", reg->read_only);
    PDWrite_data(writer, "register", reg->data, reg->size);
    PDWrite_entry_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static void send_6502_registers(PDWriter* writer) {
//  PDWrite_event_begin(writer, PDEventType_SetRegisters);
//  PDWrite_array_begin(writer, "registers");
//
//  write_register(writer, "pc", 2, 0x4444, 1);
//  write_register(writer, "sp", 1, 1, 0);
//  write_register(writer, "a", 1, 2, 0);
//  write_register(writer, "x", 1, 3, 0);
//  write_register(writer, "y", 1, 4, 0);
//
//  PDWrite_array_end(writer);
//  PDWrite_event_end(writer);
//}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void send_registers(DummyPlugin* data, PDWriter* writer) {
    int i = 0;

    PDWrite_event_begin(writer, PDEventType_SetRegisters);
    PDWrite_array_begin(writer, "registers");

    for (i = 0; i < data->registers_count; i++) {
        write_register(writer, &data->registers[i]);
    }

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Register* get_register_by_name(DummyPlugin* plugin, const char* name) {
    int i = 0;

    for (i = 0; i < plugin->registers_count; i++) {
        if (strcmp(name, plugin->registers[i].name) == 0) {
            return &plugin->registers[i];
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void update_register(DummyPlugin* plugin, PDReader* reader) {
    const char* name;
    void* data;
    uint64_t size;
    Register* reg;

    if (PDRead_find_string(reader, &name, "name", 0) == PDReadStatus_NotFound) {
        printf("Could not find 'name' field in SetRegisters request\n");
        return;
    }

    if (PDRead_find_data(reader, &data, &size, "data", 0) == PDReadStatus_NotFound) {
        printf("Could not find 'data' field in SetRegisters request\n");
        return;
    }

    reg = get_register_by_name(plugin, name);

    if (!reg) {
        printf("Could not find register with name %s in SetRegisters request\n", name);
        return;
    }

    if (reg->size != size) {
        printf("Size of data for register %s %d does not match its original size %d\n", name, (int)size, reg->size);
        return;
    }

    if (reg->read_only != 0) {
        printf("Tried to change register %s which is read-only\n", name);
        return;
    }

    printf("Setting value for register %s\n", name);
    memcpy(reg->data, data, size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void step_to_next_location(DummyPlugin* plugin) {
    int i;

    for (i = 0; i < (int)sizeof_array(s_disasm_data) - 1; ++i) {
        if (s_disasm_data[i].address == plugin->exception_location) {
            plugin->exception_location = s_disasm_data[i + 1].address;
            // printf("set exception to 0x%x\n", plugin->exception_location);
            return;
        }
    }

    plugin->exception_location = s_disasm_data[0].address;
    printf("reseting to start\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_exception_location(DummyPlugin* data, PDWriter* writer) {
    if (data->exception_location == data->prev_exception_location)
        return;

    PDWrite_event_begin(writer, PDEventType_SetExceptionLocation);
    PDWrite_u64(writer, "address", (uint64_t)data->exception_location);
    PDWrite_u8(writer, "address_size", 2);
    PDWrite_event_end(writer);

    data->prev_exception_location = data->exception_location;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void get_memory(DummyPlugin* data, PDReader* reader, PDWriter* writer) {
    int64_t address_start = 0;
    int64_t size = 0;
    int64_t end_address;

    PDRead_find_s64(reader, &address_start, "address_start", 0);
    PDRead_find_s64(reader, &size, "size", 0);

    // clamp the range we can fetch memory from

    if (address_start < data->memory_start) {
        size -= (data->memory_start - address_start);
        address_start = data->memory_start;
    }

    end_address = address_start + size;

    if (end_address > data->memory_end) {
        size -= end_address - data->memory_end;
    }

    // Make sure we have some data to read from

    if (size <= 0) {
        return;
    }

    PDWrite_event_begin(writer, PDEventType_SetMemory);
    PDWrite_u64(writer, "address", (uint64_t)address_start);
    PDWrite_u32(writer, "address_width", 4);
    PDWrite_data(writer, "data", data->memory + (address_start - data->memory_start), (uint32_t)size);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void update_memory(DummyPlugin* plugin, PDReader* reader) {
    void* data;
    uint64_t address = 0;
    uint64_t size = 0;

    PDRead_find_u64(reader, &address, "address", 0);

    if (PDRead_find_data(reader, &data, &size, "data", 0) == PDReadStatus_NotFound)
        return;

    // TODO: Not to assume that this is always within range?

    memcpy(plugin->memory + (address - (uint64_t)plugin->memory_start), data, size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int find_instruction_index(uint64_t address) {
    int i = 0;

    for (i = 0; i < (int)sizeof_array(s_disasm_data) - 1; ++i) {
        const DisasmData* t0 = &s_disasm_data[i + 0];
        const DisasmData* t1 = &s_disasm_data[i + 1];
        if (address >= t0->address && address < t1->address) {
            return i;
        }
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void get_disassembly(PDReader* reader, PDWriter* writer) {
    uint64_t address_start = 0;
    uint32_t instruction_count = 0;
    uint32_t i = 0;
    int index;
    int total_instruction_count = 0;
    uint64_t last_address = 0;

    PDRead_find_u64(reader, &address_start, "address_start", 0);
    PDRead_find_u32(reader, &instruction_count, "instruction_count", 0);

    index = find_instruction_index(address_start);

    if (index == -1) {
        index = 0;
    }

    PDWrite_event_begin(writer, PDEventType_SetDisassembly);
    PDWrite_u32(writer, "address_width", 4);

    PDWrite_array_begin(writer, "disassembly");

    total_instruction_count = sizeof_array(s_disasm_data);

    last_address = s_disasm_data[total_instruction_count - 1].address;

    printf("requested count %d, total count %d\n", instruction_count, total_instruction_count);

    for (i = 0; i < instruction_count; ++i) {
        PDWrite_array_entry_begin(writer);

        if (index >= (int)sizeof_array(s_disasm_data)) {
            PDWrite_u32(writer, "address", (uint32_t)last_address);
            PDWrite_string(writer, "line", "????");
            last_address += 1;
        } else {
            PDWrite_u32(writer, "address", s_disasm_data[index].address);
            PDWrite_string(writer, "line", s_disasm_data[index].string);
            last_address += 1;
        }

        index += 1;
        PDWrite_entry_end(writer);
    }

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*

void eval_expression(DummyPlugin* data, PDReader* reader, PDWriter* writer) {
    int i;
    int err;
    uint64_t request_id;
    const char* expression;
    te_expr* expr = 0;
    (void)reader;
    (void)writer;

    if (PDRead_find_u64(reader, &request_id, "_request_id", 0) == PDReadStatus_NotFound) {
        printf("Unable to find _request_id in expression request\n");
        return;
    }

    if (PDRead_find_string(reader, &expression, "expression", 0) == PDReadStatus_NotFound) {
        printf("Unable to find expression in expression request\n");
        return;
    }

    double temp_regs[8];
    te_variable vars[8];

    // Use just the 8 first registers for expressions.

    for (i = 0; i < 8; ++i) {
        temp_regs[i] = get_reg_for_expression(data->registers[i].data);
        vars[i].name = data->registers[i].name;
        vars[i].address = &temp_regs;
        vars[i].type = TE_VARIABLE;
        vars[i].context = 0;
    }

    PDWrite_event_begin(writer, PDEventType_ReplyEvalExpression);
    PDWrite_u64(writer, "_reply_request", request_id);

    expr = te_compile(expression, vars, 8, &err);

    if (expr) {
        double ret = te_eval(expr);
        PDWrite_u64(writer, "result", (uint64_t)ret);
    } else {
        PDWrite_string(writer, "error", expression);
    }

    PDWrite_event_end(writer);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data, PDAction action, PDReader* reader, PDWriter* writer) {
    uint32_t event;
    DummyPlugin* data = (DummyPlugin*)user_data;

    switch (action) {
        case PDAction_Step: {
            step_to_next_location(data);
            break;
        }

        case PDAction_StepOut: {
            break;
        }

        case PDAction_StepOver: {
            break;
        }

        default: { break; }
    }

    while ((event = PDRead_get_event(reader))) {
        switch (event) {
            case PDEventType_GetDisassembly: {
                get_disassembly(reader, writer);
                break;
            }

            case PDEventType_GetMemory: {
                get_memory(data, reader, writer);
                break;
            }

            case PDEventType_UpdateMemory: {
                update_memory(data, reader);
                break;
            }

            case PDEventType_GetRegisters: {
                send_registers(data, writer);
                break;
            }

            case PDEventType_UpdateRegister: {
                update_register(data, reader);
                break;
            }

                /*
                case PDEventType_RequestEvalExpression:
                {
                    eval_expression(data, reader, writer);
                    break;
                }
                */
        }
    }

    set_exception_location(data, writer);
    // printf("Update backend\n");

    return PDDebugState_Running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int save_state(void* user_data, PDSaveState* save) {
    DummyPlugin* data = (DummyPlugin*)user_data;
    PDIO_write_int(save, data->register_type);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int load_state(void* user_data, PDLoadState* load) {
    int64_t reg_status = 0;
    DummyPlugin* data = (DummyPlugin*)user_data;
    if (PDIO_read_int(load, &reg_status) == PDLoadStatus_Ok) {
        data->register_type = (int)reg_status;
        printf("loaded register_type %d\n", data->register_type);
    } else {
        printf("failed to load register_type\n");
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin = {
    "Dummy Backend", create_instance, destroy_instance, update, save_state, load_state,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void pd_init_plugin(RegisterPlugin* register_plugin, void* private_data) {
    register_plugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
