#include "pd_backend.h"
#include "pd_host.h"
#include "pd_menu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define sizeof_array(t) (sizeof(t) / sizeof(t[0]))

typedef struct DisasmData {
	uint16_t address;
	const char* string;
} DisasmData;

static DisasmData s_disasm_data[] = {
	{ 0x0000e003, "jmp 0xe0c1" },
	{ 0x0000e006, "rti" },
	{ 0x0000e007, "rti" },
	{ 0x0000e008, "rti" },
	{ 0x0000e009, "rti" },
	{ 0x0000e00a, "rti" },
	{ 0x0000e00b, "lsr a" },
	{ 0x0000e00c, "lsr a" },
	{ 0x0000e00d, "lsr 0x4e4e" },
	{ 0x0000e010, "lsr 0x5757" },
	{ 0x0000e013, "sre 0x57,x" },
	{ 0x0000e015, "sre 0x61,x" },
	{ 0x0000e017, "rra 0x67" },
	{ 0x0000e019, "nop 0x64" },
	{ 0x0000e01b, "php" },
	{ 0x0000e01c, "ora 0x00" },
	{ 0x0000e01e, "brk" },
	{ 0x0000e01f, "brk" },
	{ 0x0000e020, "eor (0x6c,x)" },
	{ 0x0000e022, "adc 0x78" },
	{ 0x0000e024, "adc (0x6e,x)" },
	{ 0x0000e026, "nop 0x65" },
	{ 0x0000e028, "hlt" },
	{ 0x0000e029, "jsr 0x6957" },
	{ 0x0000e02c, "arr #0x6c" },
	{ 0x0000e02e, "adc 0x6e,x" },
	{ 0x0000e030, "nop 0x20" },
	{ 0x0000e032, "plp" },
	{ 0x0000e033, "sre 0x69,x" },
	{ 0x0000e035, "arr #0x6c" },
	{ 0x0000e037, "adc 0x6e,x" },
	{ 0x0000e039, "nop 0x29" },
	{ 0x0000e03b, "jsr 0x2020" },
	{ 0x0000e03e, "jsr 0x9d20" },
	{ 0x0000e041, "adc #0xe3" },
	{ 0x0000e043, "lda 0xe358,x" },
	{ 0x0000e046, "sta 0xe368,x" },
	{ 0x0000e049, "rts" },
	{ 0x0000e04a, "sta 0xe3a8,x" },
	{ 0x0000e04d, "rts" },
	{ 0x0000e04e, "ldy #0x00" },
	{ 0x0000e050, "sty 0xe0fc" },
	{ 0x0000e053, "sta 0xe0f8" },
	{ 0x0000e056, "rts" },
	{ 0x0000e057, "sta 0xe37f" },
	{ 0x0000e05a, "sta 0xe386" },
	{ 0x0000e05d, "sta 0xe38d" },
	{ 0x0000e060, "rts" },
	{ 0x0000e061, "jmp 0xe262" },
	{ 0x0000e064, "tya" },
	{ 0x0000e065, "beq 0x00e0b7" },
	{ 0x0000e067, "lda 0xe5c5,y" },
	{ 0x0000e06a, "sta 0xff" },
	{ 0x0000e06c, "lda 0xe368,x" },
	{ 0x0000e06f, "cmp #0x02" },
	{ 0x0000e071, "bcc 0x00e090" },
	{ 0x0000e073, "beq 0x00e0a7" },
	{ 0x0000e075, "ldy 0x1381,x" },
	{ 0x0000e078, "lda 0x1395,x" },
	{ 0x0000e07b, "sbc 0x13b5,y" },
	{ 0x0000e07e, "pha" },
	{ 0x0000e07f, "lda 0x1396,x" },
	{ 0x0000e082, "sbc 0x140e,y" },
	{ 0x0000e085, "tay" },
	{ 0x0000e086, "pla" },
	{ 0x0000e087, "bcs 0x00e0a0" },
	{ 0x0000e089, "adc 0xfe" },
	{ 0x0000e08b, "tya" },
	{ 0x0000e08c, "adc 0xff" },
	{ 0x0000e08e, "bpl 0x00e0b7" },
	{ 0x0000e090, "lda 0xe395,x" },
	{ 0x0000e093, "adc 0xfe" },
	{ 0x0000e095, "sta 0xe395,x" },
	{ 0x0000e098, "lda 0xe396,x" },
	{ 0x0000e09b, "adc 0xff" },
	{ 0x0000e09d, "jmp 0xe25f" },
	{ 0x0000e0a0, "sbc 0xfe" },
	{ 0x0000e0a2, "tya" },
	{ 0x0000e0a3, "sbc 0xff" },
	{ 0x0000e0a5, "bmi 0x00e0b7" },
	{ 0x0000e0a7, "lda 0xe395,x" },
	{ 0x0000e0aa, "sbc 0xfe" },
	{ 0x0000e0ac, "sta 0xe395,x" },
	{ 0x0000e0af, "lda 0xe396,x" },
	{ 0x0000e0b2, "sbc 0xff" },
	{ 0x0000e0b4, "jmp 0xe25f" },
	{ 0x0000e0b7, "ldy 0xe381,x" },
	{ 0x0000e0ba, "jmp 0xe256" },
	{ 0x0000e0bd, "sta 0xe0c4" },
	{ 0x0000e0c0, "rts" },
	{ 0x0000e0c1, "ldx #0x00" },
	{ 0x0000e0c3, "ldy #0x00" },
	{ 0x0000e0c5, "bmi 0x00e0f7" },
	{ 0x0000e0c7, "txa" },
	{ 0x0000e0c8, "ldx #0x29" },
	{ 0x0000e0ca, "sta 0xe353,x" },
	{ 0x0000e0cd, "dex" },
	{ 0x0000e0ce, "bpl 0x00e0ca" },
	{ 0x0000e0d0, "sta 0xd415" },
	{ 0x0000e0d3, "sta 0xe146" },
	{ 0x0000e0d6, "sta 0xe0f8" },
	{ 0x0000e0d9, "stx 0xe0c4" },
	{ 0x0000e0dc, "tax" },
	{ 0x0000e0dd, "jsr 0xe0e7" },
	{ 0x0000e0e0, "ldx #0x07" },
	{ 0x0000e0e2, "jsr 0xe0e7" },
	{ 0x0000e0e5, "ldx #0x0e" },
	{ 0x0000e0e7, "lda #0x05" },
	{ 0x0000e0e9, "sta 0xe37f,x" },
	{ 0x0000e0ec, "lda #0x01" },
	{ 0x0000e0ee, "sta 0xe380,x" },
	{ 0x0000e0f1, "sta 0xe382,x" },
	{ 0x0000e0f4, "jmp 0xe349" },
	{ 0x0000e0f7, "ldy #0x00" },
	{ 0x0000e0f9, "beq 0x00e140" },
	{ 0x0000e0fb, "lda #0x00" },
	{ 0x0000e0fd, "bne 0x00e122" },
	{ 0x0000e0ff, "lda 0xe596,y" },
	{ 0x0000e102, "beq 0x00e116" },
	{ 0x0000e104, "bpl 0x00e11f" },
	{ 0x0000e106, "asl a" },
	{ 0x0000e107, "sta 0xe14b" },
	{ 0x0000e10a, "lda 0xe5ad,y" },
	{ 0x0000e10d, "sta 0xe146" },
	{ 0x0000e110, "lda 0xe597,y" },
	{ 0x0000e113, "bne 0x00e134" },
	{ 0x0000e115, "iny" },
	{ 0x0000e116, "lda 0xe5ad,y" },
	{ 0x0000e119, "sta 0xe141" },
	{ 0x0000e11c, "jmp 0xe131" },
	{ 0x0000e11f, "sta 0xe0fc" },
	{ 0x0000e122, "lda 0xe5ad,y" },
	{ 0x0000e125, "clc" },
	{ 0x0000e126, "adc 0xe141" },
	{ 0x0000e129, "sta 0xe141" },
	{ 0x0000e12c, "dec 0xe0fc" },
	{ 0x0000e12f, "bne 0x00e142" },
	{ 0x0000e131, "lda 0xe597,y" },
	{ 0x0000e134, "cmp #0xff" },
	{ 0x0000e136, "iny" },
	{ 0x0000e137, "tya" },
	{ 0x0000e138, "bcc 0x00e13d" },
	{ 0x0000e13a, "lda 0xe5ad,y" },
	{ 0x0000e13d, "sta 0xe0f8" },
	{ 0x0000e140, "lda #0x00" },
	{ 0x0000e142, "sta 0xd416" },
	{ 0x0000e145, "lda #0x00" },
	{ 0x0000e147, "sta 0xd417" },
	{ 0x0000e14a, "lda #0x00" },
	{ 0x0000e14c, "ora #0x0f" },
	{ 0x0000e14e, "sta 0xd418" },
	{ 0x0000e151, "jsr 0xe15b" },
	{ 0x0000e154, "ldx #0x07" },
	{ 0x0000e156, "jsr 0xe15b" },
	{ 0x0000e159, "ldx #0x0e" },
	{ 0x0000e15b, "dec 0xe380,x" },
	{ 0x0000e15e, "beq 0x00e16b" },
	{ 0x0000e160, "bpl 0x00e168" },
	{ 0x0000e162, "lda 0xe37f,x" },
	{ 0x0000e165, "sta 0xe380,x" },
	{ 0x0000e168, "jmp 0xe213" },
	{ 0x0000e16b, "ldy 0xe358,x" },
	{ 0x0000e16e, "lda 0xe006,y" },
	{ 0x0000e171, "sta 0xe208" },
	{ 0x0000e174, "sta 0xe211" },
	{ 0x0000e177, "lda 0xe356,x" },
	{ 0x0000e17a, "bne 0x00e1ac" },
	{ 0x0000e17c, "ldy 0xe37d,x" },
	{ 0x0000e17f, "lda 0xe46e,y" },
	{ 0x0000e182, "sta 0xfe" },
	{ 0x0000e184, "lda 0xe471,y" },
	{ 0x0000e187, "sta 0xff" },
	{ 0x0000e189, "ldy 0xe353,x" },
	{ 0x0000e18c, "lda (0xfe),y" },
	{ 0x0000e18e, "cmp #0xff" },
	{ 0x0000e190, "bcc 0x00e198" },
	{ 0x0000e192, "iny" },
	{ 0x0000e193, "lda (0xfe),y" },
	{ 0x0000e195, "tay" },
	{ 0x0000e196, "lda (0xfe),y" },
	{ 0x0000e198, "cmp #0xe0" },
	{ 0x0000e19a, "bcc 0x00e1a4" },
	{ 0x0000e19c, "sbc #0xf0" },
	{ 0x0000e19e, "sta 0xe354,x" },
	{ 0x0000e1a1, "iny" },
	{ 0x0000e1a2, "lda (0xfe),y" },
	{ 0x0000e1a4, "sta 0xe37e,x" },
	{ 0x0000e1a7, "iny" },
	{ 0x0000e1a8, "tya" },
	{ 0x0000e1a9, "sta 0xe353,x" },
	{ 0x0000e1ac, "ldy 0xe382,x" },
	{ 0x0000e1af, "lda 0xe4e6,y" },
	{ 0x0000e1b2, "sta 0xe3ac,x" },
	{ 0x0000e1b5, "lda 0xe36a,x" },
	{ 0x0000e1b8, "beq 0x00e20d" },
	{ 0x0000e1ba, "sec" },
	{ 0x0000e1bb, "sbc #0x60" },
	{ 0x0000e1bd, "sta 0xe381,x" },
	{ 0x0000e1c0, "lda #0x00" },
	{ 0x0000e1c2, "sta 0xe368,x" },
	{ 0x0000e1c5, "sta 0xe36a,x" },
	{ 0x0000e1c8, "lda 0xe358,x" },
	{ 0x0000e1cb, "cmp #0x03" },
	{ 0x0000e1cd, "beq 0x00e20d" },
	{ 0x0000e1cf, "lda 0xe4f1,y" },
	{ 0x0000e1d2, "sta 0xe36c,x" },
	{ 0x0000e1d5, "inc 0xe383,x" },
	{ 0x0000e1d8, "lda 0xe4d0,y" },
	{ 0x0000e1db, "beq 0x00e1e5" },
	{ 0x0000e1dd, "sta 0xe36d,x" },
	{ 0x0000e1e0, "lda #0x00" },
	{ 0x0000e1e2, "sta 0xe36e,x" },
	{ 0x0000e1e5, "lda 0xe4db,y" },
	{ 0x0000e1e8, "beq 0x00e1f2" },
	{ 0x0000e1ea, "sta 0xe0f8" },
	{ 0x0000e1ed, "lda #0x00" },
	{ 0x0000e1ef, "sta 0xe0fc" },
	{ 0x0000e1f2, "lda 0xe4c5,y" },
	{ 0x0000e1f5, "sta 0xe36b,x" },
	{ 0x0000e1f8, "lda 0xe4ba,y" },
	{ 0x0000e1fb, "sta 0xe3a8,x" },
	{ 0x0000e1fe, "lda 0xe4af,y" },
	{ 0x0000e201, "sta 0xe3a7,x" },
	{ 0x0000e204, "lda 0xe359,x" },
	{ 0x0000e207, "jsr 0xe040" },
	{ 0x0000e20a, "jmp 0xe328" },
	{ 0x0000e20d, "lda 0xe359,x" },
	{ 0x0000e210, "jsr 0xe040" },
	{ 0x0000e213, "ldy 0xe36b,x" },
	{ 0x0000e216, "beq 0x00e235" },
	{ 0x0000e218, "lda 0xe4fc,y" },
	{ 0x0000e21b, "beq 0x00e220" },
	{ 0x0000e21d, "sta 0xe36c,x" },
	{ 0x0000e220, "lda 0xe4fd,y" },
	{ 0x0000e223, "cmp #0xff" },
	{ 0x0000e225, "iny" },
	{ 0x0000e226, "tya" },
	{ 0x0000e227, "bcc 0x00e22d" },
	{ 0x0000e229, "clc" },
	{ 0x0000e22a, "lda 0xe53c,y" },
	{ 0x0000e22d, "sta 0xe36b,x" },
	{ 0x0000e230, "lda 0xe53b,y" },
	{ 0x0000e233, "bne 0x00e24e" },
	{ 0x0000e235, "lda 0xe380,x" },
	{ 0x0000e238, "beq 0x00e265" },
	{ 0x0000e23a, "ldy 0xe368,x" },
	{ 0x0000e23d, "lda 0xe016,y" },
	{ 0x0000e240, "sta 0xe24c" },
	{ 0x0000e243, "ldy 0xe369,x" },
	{ 0x0000e246, "lda 0xe5c8,y" },
	{ 0x0000e249, "sta 0xfe" },
	{ 0x0000e24b, "jmp 0xe061" },
	{ 0x0000e24e, "bpl 0x00e255" },
	{ 0x0000e250, "adc 0xe381,x" },
	{ 0x0000e253, "and #0x7f" },
	{ 0x0000e255, "tay" },
	{ 0x0000e256, "lda 0xe3b5,y" },
	{ 0x0000e259, "sta 0xe395,x" },
	{ 0x0000e25c, "lda 0xe40e,y" },
	{ 0x0000e25f, "sta 0xe396,x" },
	{ 0x0000e262, "lda 0xe380,x" },
	{ 0x0000e265, "cmp 0xe3ac,x" },
	{ 0x0000e268, "beq 0x00e2ad" },
	{ 0x0000e26a, "ldy 0xe36d,x" },
	{ 0x0000e26d, "beq 0x00e2aa" },
	{ 0x0000e26f, "ora 0xe356,x" },
	{ 0x0000e272, "beq 0x00e2aa" },
	{ 0x0000e274, "lda 0xe36e,x" },
	{ 0x0000e277, "bne 0x00e28a" },
	{ 0x0000e279, "lda 0xe57c,y" },
	{ 0x0000e27c, "bpl 0x00e287" },
	{ 0x0000e27e, "lda 0xe589,y" },
	{ 0x0000e281, "sta 0xe397,x" },
	{ 0x0000e284, "jmp 0xe29b" },
	{ 0x0000e287, "sta 0xe36e,x" },
	{ 0x0000e28a, "lda 0xe397,x" },
	{ 0x0000e28d, "clc" },
	{ 0x0000e28e, "adc 0xe589,y" },
	{ 0x0000e291, "adc #0x00" },
	{ 0x0000e293, "sta 0xe397,x" },
	{ 0x0000e296, "dec 0xe36e,x" },
	{ 0x0000e299, "bne 0x00e2aa" },
	{ 0x0000e29b, "lda 0xe57d,y" },
	{ 0x0000e29e, "cmp #0xff" },
	{ 0x0000e2a0, "iny" },
	{ 0x0000e2a1, "tya" },
	{ 0x0000e2a2, "bcc 0x00e2a7" },
	{ 0x0000e2a4, "lda 0xe589,y" },
	{ 0x0000e2a7, "sta 0xe36d,x" },
	{ 0x0000e2aa, "jmp 0xe328" },
	{ 0x0000e2ad, "ldy 0xe37e,x" },
	{ 0x0000e2b0, "lda 0xe474,y" },
	{ 0x0000e2b3, "sta 0xfe" },
	{ 0x0000e2b5, "lda 0xe492,y" },
	{ 0x0000e2b8, "sta 0xff" },
	{ 0x0000e2ba, "ldy 0xe356,x" },
	{ 0x0000e2bd, "lda (0xfe),y" },
	{ 0x0000e2bf, "cmp #0x40" },
	{ 0x0000e2c1, "bcc 0x00e2db" },
	{ 0x0000e2c3, "cmp #0x60" },
	{ 0x0000e2c5, "bcc 0x00e2e5" },
	{ 0x0000e2c7, "cmp #0xc0" },
	{ 0x0000e2c9, "bcc 0x00e2f9" },
	{ 0x0000e2cb, "lda 0xe357,x" },
	{ 0x0000e2ce, "bne 0x00e2d2" },
	{ 0x0000e2d0, "lda (0xfe),y" },
	{ 0x0000e2d2, "adc #0x00" },
	{ 0x0000e2d4, "sta 0xe357,x" },
	{ 0x0000e2d7, "beq 0x00e31f" },
	{ 0x0000e2d9, "bne 0x00e328" },
	{ 0x0000e2db, "sta 0xe382,x" },
	{ 0x0000e2de, "iny" },
	{ 0x0000e2df, "lda (0xfe),y" },
	{ 0x0000e2e1, "cmp #0x60" },
	{ 0x0000e2e3, "bcs 0x00e2f9" },
	{ 0x0000e2e5, "cmp #0x50" },
	{ 0x0000e2e7, "and #0x0f" },
	{ 0x0000e2e9, "sta 0xe358,x" },
	{ 0x0000e2ec, "beq 0x00e2f4" },
	{ 0x0000e2ee, "iny" },
	{ 0x0000e2ef, "lda (0xfe),y" },
	{ 0x0000e2f1, "sta 0xe359,x" },
	{ 0x0000e2f4, "bcs 0x00e31f" },
	{ 0x0000e2f6, "iny" },
	{ 0x0000e2f7, "lda (0xfe),y" },
	{ 0x0000e2f9, "cmp #0xbd" },
	{ 0x0000e2fb, "bcc 0x00e303" },
	{ 0x0000e2fd, "beq 0x00e31f" },
	{ 0x0000e2ff, "ora #0xf0" },
	{ 0x0000e301, "bne 0x00e31c" },
	{ 0x0000e303, "adc 0xe354,x" },
	{ 0x0000e306, "sta 0xe36a,x" },
	{ 0x0000e309, "lda 0xe358,x" },
	{ 0x0000e30c, "cmp #0x03" },
	{ 0x0000e30e, "beq 0x00e31f" },
	{ 0x0000e310, "lda #0x00" },
	{ 0x0000e312, "sta 0xe3a8,x" },
	{ 0x0000e315, "lda #0x0f" },
	{ 0x0000e317, "sta 0xe3a7,x" },
	{ 0x0000e31a, "lda #0xfe" },
	{ 0x0000e31c, "sta 0xe383,x" },
	{ 0x0000e31f, "iny" },
	{ 0x0000e320, "lda (0xfe),y" },
	{ 0x0000e322, "beq 0x00e325" },
	{ 0x0000e324, "tya" },
	{ 0x0000e325, "sta 0xe356,x" },
	{ 0x0000e328, "lda 0xe397,x" },
	{ 0x0000e32b, "sta 0xd402,x" },
	{ 0x0000e32e, "sta 0xd403,x" },
	{ 0x0000e331, "lda 0xe3a8,x" },
	{ 0x0000e334, "sta 0xd406,x" },
	{ 0x0000e337, "lda 0xe3a7,x" },
	{ 0x0000e33a, "sta 0xd405,x" },
	{ 0x0000e33d, "lda 0xe395,x" },
	{ 0x0000e340, "sta 0xd400,x" },
	{ 0x0000e343, "lda 0xe396,x" },
	{ 0x0000e346, "sta 0xd401,x" },
	{ 0x0000e349, "lda 0xe36c,x" },
	{ 0x0000e34c, "and 0xe383,x" },
	{ 0x0000e34f, "sta 0xd404,x" },
	{ 0x0000e352, "rts" },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DummyPlugin {
	int exception_location;

} DummyPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* create_instance(ServiceFunc* serviceFunc) {
	(void)serviceFunc;

	DummyPlugin* plugin = (DummyPlugin*)malloc(sizeof(DummyPlugin));
	memset(plugin, 0, sizeof(DummyPlugin));
	plugin->exception_location = s_disasm_data[0].address;

    return plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroy_instance(void* user_data) {
	free(user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void write_register(PDWriter* writer, const char* name, uint8_t size, uint16_t reg, uint8_t read_only) {
    PDWrite_array_entry_begin(writer);
    PDWrite_string(writer, "name", name);
    PDWrite_u8(writer, "size", size);

    if (read_only) {
        PDWrite_u8(writer, "read_only", 1);
	}

    if (size == 2) {
        PDWrite_u16(writer, "register", reg);
	} else {
        PDWrite_u8(writer, "register", (uint8_t)reg);
	}

    PDWrite_entry_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void send_6502_registers(PDWriter* writer) {
	PDWrite_event_begin(writer, PDEventType_SetRegisters);
	PDWrite_array_begin(writer, "registers");

	write_register(writer, "pc", 2, 0x4444, 1);
	write_register(writer, "sp", 1, 1, 0);
	write_register(writer, "a", 1, 2, 0);
	write_register(writer, "x", 1, 3, 0);
	write_register(writer, "y", 1, 4, 0);

	PDWrite_array_end(writer);
	PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void step_to_next_location(DummyPlugin* plugin) {
	int i;

	for (i = 0; i < (int)sizeof_array(s_disasm_data) - 1; ++i) {
		if (s_disasm_data[i].address == plugin->exception_location) {
			plugin->exception_location = s_disasm_data[i + 1].address;
			printf("set exception to 0x%x\n", plugin->exception_location);
			return;
		}
	}

	plugin->exception_location = s_disasm_data[0].address;
	printf("reseting to start\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_exception_location(DummyPlugin* data, PDWriter* writer) {
    PDWrite_event_begin(writer, PDEventType_SetExceptionLocation);
    PDWrite_u64(writer, "address", (uint64_t)data->exception_location);
    PDWrite_u8(writer, "address_size", 2);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void on_menu(PDReader* reader) {
    uint32_t menuId;

    PDRead_find_u32(reader, &menuId, "menu_id", 0);

    switch (menuId) {
        case 1:
        {
        	printf("id 1 pressed!\n");
            break;
        }

        case 2:
        {
        	printf("id 2 pressed!\n");
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int find_instruction_index(uint64_t address) {
	int i = 0;

	for (i = 0; i < (int)sizeof_array(s_disasm_data) - 1; ++i) {
		const DisasmData* t0 = &s_disasm_data[i + 0];
		const DisasmData* t1 = &s_disasm_data[i + 1];
		if (address >= t0->address &&
			address < t1->address) {
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

static PDDebugState update(void* user_data,
						   PDAction action,
						   PDReader* reader,
						   PDWriter* writer) {
    uint32_t event;
    DummyPlugin* data = (DummyPlugin*)user_data;

	switch (action)
	{
		case PDAction_Step:
		{
			step_to_next_location(data);
			break;
		}

		case PDAction_StepOut:
		{

			break;
		}

		case PDAction_StepOver:
		{

			break;
		}

		default :
		{

			break;
		}
	}


    while ((event = PDRead_get_event(reader))) {
        switch (event) {
            case PDEventType_MenuEvent:
			{
                on_menu(reader);
                break;
            }

			case PDEventType_GetDisassembly:
			{
				get_disassembly(reader, writer);
				break;
			}
		}
	}

	set_exception_location(data, writer);
	send_6502_registers(writer);

    // printf("Update backend\n");

    return PDDebugState_NoTarget;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenuHandle register_menu(void* user_data, PDMenuFuncs* menu_funcs) {
	(void)user_data;

	PDMenuHandle menu = PDMenu_create_menu(menu_funcs, "Dummy Backend Menu");

	PDMenu_add_menu_item(menu_funcs, menu, "Id 1", 1, 0, 0);
	PDMenu_add_menu_item(menu_funcs, menu, "Id 2", 2, 0, 0);

	return menu;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin =
{
    "Dummy Backend",
    create_instance,
    destroy_instance,
	register_menu,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
    registerPlugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

