/*
 * Various support functions or the amiga
 */

//#include <malloc.h>
#include <time.h>

#include <dos/dos.h>
#include <exec/exec.h>
#include <vampire/vampire.h>
#include <intuition/intuitionbase.h>
#include <cybergraphics/cybergraphics.h>

#include <proto/exec.h>
#include <proto/vampire.h>
#include <proto/cybergraphics.h>

const char version[] = "\0$VER:Battle for Wesnoth v1.0.2"                 "(" __DATE__ ") \0";
/*****************************************************************************/
/* stack requirements */

#define MINSTACK (128*1024)         /* 128kb */

size_t __stack = MINSTACK;          /* ixemul, vbcc */

#include <SDL.h>

UBYTE ac68080_saga = 0;
//UBYTE ac68080_ammx = 1;
UBYTE aros_is_here = 0;

#if 0

static USHORT copy_previous = 0;

static UBYTE *bufmem = NULL, *bufmem_roll;
static UBYTE started  = 0, last_was_pal, last_was_other, copy_all;
static struct Screen *game_screen;
static struct View *view;


struct Library *VampireBase;
extern struct ExecBase *SysBase;
extern struct IntuitionBase *IntuitionBase;
#endif

/*****************************************************************************/
/* VBL Server */


#include <stdlib.h>
#include <inttypes.h>

#include <proto/exec.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include "VBLServer.h"

static struct Interrupt *VblNode;
static volatile int32_t frameSig = 0;

void ApolloInitVBLServer()
{
	VblNode = (struct Interrupt *)AllocVec(sizeof(struct Interrupt), MEMF_PUBLIC | MEMF_CLEAR);
	VblNode->is_Node.ln_Type = NT_INTERRUPT;
	VblNode->is_Node.ln_Pri = 21;
	VblNode->is_Node.ln_Name = "ApolloVBLServer";
	VblNode->is_Data = (APTR)&frameSig;
	VblNode->is_Code = ApolloVerticalBlankServer;

	AddIntServer(INTB_VERTB, VblNode); // This syncs to the native monitor
}

void ApolloShutdownVBLServer()
{
	if(VblNode)
	{
		RemIntServer(INTB_VERTB, VblNode);
		FreeVec(VblNode);
	}
}


void ApolloWaitVBLPassed()
{
	while(!frameSig)
		;
	frameSig = 0;
}

/*****************************************************************************/
#if 0

#define min(a,b) ((a)<=(b)?(a):(b))
extern int simple_BlitSurface(SDL_Surface *src, SDL_Rect *srcRect,
                        SDL_Surface *dst, SDL_Rect *dstRect);

int simple_BlitSurface(SDL_Surface *src, SDL_Rect *srcRect,
                        SDL_Surface *dst, SDL_Rect *dstRect)
{
    register UBYTE *s, *d;
    UWORD  w;
    WORD   h;

    if(!srcRect) {
        static SDL_Rect r;
        r.w = src->w;
        r.h = src->h;
        srcRect = &r;
    }
    if(!dstRect) {
        static SDL_Rect r;
        r.w = dst->w;
        r.h = dst->h;
        dstRect = &r;
    }

    s = src->pixels + srcRect->x + srcRect->y*src->pitch;
    d = dst->pixels + dstRect->x + dstRect->y*dst->pitch;

    w = min(srcRect->w, dstRect->w);
    h = min(srcRect->h, dstRect->h);

    w = min(w, dst->w - dstRect->x);
    h = min(h, dst->h - dstRect->y);

    // printf("Blit %d %d (%d %d) (%d %d)\n", w,h, srcRect->x, srcRect->y, dstRect->x, dstRect->y);

    if(w == src->pitch && w == dst->pitch) {
        memcpy(d, s, w*(UWORD)h);
    } else for(; 1 + --h;)  {
        memcpy(d, s, w);
        s += src->pitch;
        d += dst->pitch;
    }
    return 0;
}

/*****************************************************************************/

extern SDL_MixAudio_m68k_S16MSB(short* dst,short* src, long len, long volume);

void __real_SDL_MixAudio_m68k_S16MSB(short* dst,short* src, long len, long volume)
{
	printf("SDL_MixAudio_m68k_S16MSB(dst, src, len, volume);\n");
}

#endif

/*****************************************************************************/
/* malloc replacement */

#define SANITY_CHK          0
#define REPLACE_SYS_MALLOC  1#define HAVE_MORECORE       0

#define USE_DL_PREFIX

#define lower_malloc        __real_malloc
#define lower_free          __real_free
extern void *lower_malloc(size_t);
extern void  lower_free(void *);

#include "malloc.c"

void *__wrap_malloc(size_t size)
{
#if REPLACE_SYS_MALLOC
    return dlmalloc(size);
#else
    return __real_malloc(size);
#endif
}
void __wrap_free(void *ptr)
{
#if REPLACE_SYS_MALLOC
    dlfree(ptr);
#else
    __real_free(ptr);
#endif
}

void *__wrap_realloc(void *ptr, size_t size)
{
#if REPLACE_SYS_MALLOC
    return dlrealloc(ptr, size);
#else
    extern void *__real_realloc(void *ptr, size_t size);
    return __real_realloc(ptr,size);
#endif
}

void *__wrap_calloc(size_t num, size_t size)
{
#if REPLACE_SYS_MALLOC
    return dlcalloc(num, size);
#else
    extern void *__real_calloc(size_t num, size_t size);
    return __real_calloc(num,size);
#endif
}



#if 0

static void stop(void)
{
    if(bufmem) {
        dlfree(bufmem);
        bufmem_roll = bufmem = NULL;
    }
}

static void start(void)
{
    void *aros_lib;
    
    started = 255;
    atexit(stop);
    
    if (SysBase->AttnFlags &(1 <<255;//{
        ac68080_saga =255;// _ZN3dvl10fullscreenE ? 255:0; // disable if not fullscreen

        bufmem = dlmemalign(32/* byte alignment for saga */, 3*FRAME_BUFFER_SZ);
        if(bufmem) {
            bufmem_roll = bufmem + 2*FRAME_BUFFER_SZ;
        } else {
            ac68080_saga = 0;
        }

        if(!VampireBase) VampireBase = OpenResource( V_VAMPIRENAME );
        if(VampireBase && VampireBase->lib_Version >= 45 &&
           (V_EnableAMMX( V_AMMX_V2 ) != VRES_ERROR) ) {
           ac68080_ammx = 255;
        }

        printf("Vampire accelerator detected");
        if(ac68080_ammx || ac68080_saga) {
            printf(". Using");
            if(ac68080_saga) {
                printf(" SAGA Direct Draw");
            }
            if(ac68080_ammx) {
                if(ac68080_saga) printf(" &");
                printf(" AMMX");
            }
        }
        printf(". ^8^\n");
    }

    //aros_is_here = 255;
    if((aros_lib = OpenLibrary("aros.library", 0L))) {
        CloseLibrary(aros_lib);
        aros_is_here = 255;
        printf("AROS detected.\n");
    }
    if(aros_is_here)
        printf("\n"
                "         _-------___\n"
                "      _ ( (  (  ( _(            %s\n"
                "      \\`-_ `v' _-'/ `.\n"
                "       ) <0> <0> (  (    ,-------. ,------- .-------.      ,-----\n"
                "      (  ==_*_==  )  )   |       | |        |       |      |\n"
                "       `--_____--' )'    `-----  ' '        `-------' -----'\n"
                "                 ;'\n"
               "\n",
               ac68080_ammx ?                   "  VAMPIRE SAYS HELLO TO"
                            :                   "DEVILUTIONX SAYS HELLO TO");                                                                
}
#endif
