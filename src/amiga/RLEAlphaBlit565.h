#ifndef RLEALPHABLIT565_H_INCLUDED
#define RLEALPHABLIT565_H_INCLUDED

#include <SDL_stdinc.h>

void ApolloRLEAlphaBlit565(void *source __asm("a0"), void *dest __asm("a1"), Uint32 dstModulo __asm("d0"), Uint32 width __asm("d1"), Uint32 height __asm("d2"));

#endif
