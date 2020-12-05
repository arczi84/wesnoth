	section .text
	XDEF    _ApolloRLEAlphaBlit565

; void FastRLEAlphaBlit565(const void *source __asm("a0"), void *dest __asm("a1"), Uint32 dstModulo __asm("d0"), Uint32 width __asm("d1"), Uint32 height __asm("d2"));

_ApolloRLEAlphaBlit565
	movem.l	d1-d7/a1-a6,-(a7)

	lsl.l	#1,d1
	lea		(a1,d1),a3 ; addr of line end
	move.l	#$7e0f81f,d1

.yloop
	moveq.l	#0,d3
	tst.w	(a0) ; line starts with both skip and run == 0, end of sprite data
	beq.s	.done

; opaque line
	move.l	a1,a2
.opaquex:
	move.b	(a0)+,d3 ; skip
	lea		(a2,d3*2),a2
	move.b	(a0)+,d3 ; run
	beq.s	.noopaque
.opaqueloop:
	move.w	(a0)+,(a2)+

	subq.b	#1,d3
	bne.s	.opaqueloop

.noopaque:
	cmp.l	a3,a2
	blt.s	.opaquex

	move.l	a0,d4
	and.l	#2,d4
	add.l	d4,a0

; blend line
	move.l	a1,a2
.blendx:
	move.w	(a0)+,d3
	lea		(a2,d3*2),a2
	move.w	(a0)+,d3
	beq.s	.noblend
.blendloop:
	move.l	(a0)+,d4
	move.w	(a2),d5

	; lerp d4-d5 -> d5
	move.l	d4,d6
	and.l	#$3e0,d6 ; alpha
	lsr.l	#5,d6
	and.l	d1,d4 ; src

	move.w	d5,d7 ; split colours and aligned as in src
	swap	d5
	move.w	d7,d5
	and.l	d1,d5 ; dst

	move.l	d5,d7
	sub.l	d7,d4 ; b-a

	mulu.l	d6,d4 ; (b-a)*alpha
	lsr.l	#5,d4

	add.l	d5,d4 ; (b-a)*alpha + a
	and.l	d1,d4 ; final blended colours
	move.w	d4,d5
	swap	d4
	or.w	d4,d5

	move.w	d5,(a2)+

	subq.w	#1,d3
	bne.s	.blendloop
.noblend:
	cmp.l	a3,a2
	blt.s	.blendx

	add.l	d0,a1
	add.l	d0,a3

	subq.l	#1,d2
	bne.s	.yloop
.done:
	movem.l	(a7)+,d1-d7/a1-a6
	rts
