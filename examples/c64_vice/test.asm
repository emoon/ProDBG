:BasicUpstart2(start)

start:
	lda #$22
	ldx #$32
	ldy #$42
	inc $d020
	inc $d021
	jmp start
