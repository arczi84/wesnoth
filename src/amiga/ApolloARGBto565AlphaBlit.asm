
; void ApolloARGBto565PixelAlpha(Uint32 *src __asm("a0"), Uint16 *dst __asm("a1"), Uint32 len __asm("d0"));

	XDEF _ApolloARGBto565PixelAlpha

_ApolloARGBto565PixelAlpha:

	load.w		#$ffff,e6
	load.w		#$0000,e7
.loop:
	unpack1632	(a1),e0:e1
	load		(a0)+,e2
	load		(a0)+,e3
	vperm		#$00004444,e2,e3,e4 ; Alpha0
	vperm		#$00004444,e3,e3,e5 ; Alpha1
	load		e7,e8
	load		e7,e9
	pmula		e2,e4,e8
	pmula		e3,e5,e9
	psubb		e4,e6,e4
	psubb		e5,e6,e5
	pmula		e0,e4,e8
	pmula		e1,e5,e9

	pack3216	e8,e9,(a1)+

	subq.l		#1,d0
	bne.s		.loop

	rts

