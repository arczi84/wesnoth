	section .text
	XDEF    _ApolloVerticalBlankServer

_ApolloVerticalBlankServer:
	move.l	#$1,(a1)
	lea		$dff000,a0
	move.l	#0,d0
	rts

