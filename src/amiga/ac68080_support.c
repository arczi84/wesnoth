/*
 * Various support functions or the amiga
 */
#if 1
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

//#define FRAME_BUFFER_SZ     ((BUFFER_WIDTH)*(BUFFER_HEIGHT))

#define DIRTY               0       // 1 = 32 fps     0 = 29fps

#define CHECK_FIRSTSCREEN   1       // costs 0 fps
#define ROLL_PTR            1

#include <SDL.h>

#include "RLEAlphaBlit565.h"

UBYTE ac68080_saga = 0;
UBYTE ac68080_ammx = 0;
UBYTE aros_is_here = 0;

static USHORT copy_previous = 0;

static UBYTE *bufmem = NULL, *bufmem_roll;
static UBYTE started  = 0, last_was_pal, last_was_other, copy_all;
static struct Screen *game_screen;
static struct View *view;

struct Library *VampireBase;
extern struct ExecBase *SysBase;
extern struct IntuitionBase *IntuitionBase;

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
/* stack requirements */

#define MINSTACK (128*1024)         /* 128kb */
size_t __stack = MINSTACK;          /* ixemul, vbcc */

/*****************************************************************************/
/* Ammx RLEAlphaBlit565 */


void __wrap_SDL_RLEAlphaBlit(void *source __asm("a0"), void *dest __asm("a1"), Uint32 dstModulo __asm("d0"), Uint32 width __asm("d1"), Uint32 height __asm("d2"))
{
	//ApolloRLEAlphaBlit565(source ,dest , dstModulo, width , height );
}

/*****************************************************************************/

extern SDL_MixAudio_m68k_S16MSB(short* dst,short* src, long len, long volume);

void __real_SDL_MixAudio_m68k_S16MSB(short* dst,short* src, long len, long volume)
{

	printf("SDL_MixAudio_m68k_S16MSB(dst, src, len, volume);\n");
}

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

#endif
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

SDL_Surface* vampire_MakeTripleBuffer(SDL_Surface *surf)
{
    if(!started) start();

    if(ac68080_saga
    &&  surf->w==BUFFER_WIDTH
    &&  surf->h==BUFFER_HEIGHT
    &&  surf->pitch==BUFFER_WIDTH
    ) {
        surf->flags |= SDL_PREALLOC;
        SDL_free(surf->pixels);
        surf->pixels = bufmem;
    } else ac68080_saga = 0;
    return surf;
}

static __attribute__((noinline)) void doChkSignals(void)
{
    static UBYTE closing;
    ULONG signal = SetSignal(0,0);
    if(closing) {
        SDL_Event sdlevent;
        sdlevent.type = SDL_KEYDOWN;
        sdlevent.key.keysym.sym = SDLK_ESCAPE;
        SDL_PushEvent(&sdlevent);
        sdlevent.type = SDL_KEYUP;
        sdlevent.key.keysym.sym = SDLK_ESCAPE;
        SDL_PushEvent(&sdlevent);
    }
    if(signal & SIGBREAKF_CTRL_E) {
        time_t t;
        SetSignal(0, SIGBREAKF_CTRL_E);

        t = time(0);
        printf("\nMemory statistics on %s", ctime(&t));
        dlmalloc_stats();
        printf("\n");
    }
    if(signal & SIGBREAKF_CTRL_C) {
        SetSignal(0, SIGBREAKF_CTRL_C);

        printf("Ctrl-C received\n");
        gamemenu_quit_game(0);
        closing = 255;
    }
}

static void chkSignals(void)
{
    static UBYTE closing, ctr;
    if(!ctr) {
        ctr = 4;
        doChkSignals();
    } else {
        --ctr;
    }
}

static void blitRect(UBYTE *dst, UBYTE *src, UWORD x, UWORD y, size_t w, UWORD h)
{
    src += SCREENXY(x,y);
    dst += SCREENXY(x,y);
    memcpy(dst-x, src-x, BUFFER_WIDTH*h);

    // do {
        // memcpy(dst, src, w);
        // src += BUFFER_WIDTH;
        // dst += BUFFER_WIDTH;
    // } while(--h);
}

// check if palette has changed
static void doPalette(void)
{
    static int last_version = 0;
    if(last_version!=pal_palette_version) {
        last_version = pal_palette_version;
        SDL_SetColors(SDL_GetVideoSurface(), pal_palette->colors, 0, pal_palette->ncolors);
    }
}

// set saga regs to dsplay screen at ptr (modulo bytes to skip at end of line)
static void setFrameBufferRegs(UBYTE *ptr, UWORD modulo)
{
    volatile UBYTE **dpy = (UBYTE**)0xDFF1EC; /* Frame buffer address */
    volatile UWORD  *mod =  (UWORD*)0xDFF1E6; /* Frame buffer modulo */

    *dpy = ptr;
    *mod = modulo;
}

static void doFlip(void)
{
    struct Screen *first_screen;

#if DIRTY
    // hacky way to debug
    *dpy = (void*)(~31&(int)pal_surface->pixels);
    return;
#endif

#if CHECK_FIRSTSCREEN
    // check if screen has changed
    if(game_screen != (first_screen = IntuitionBase->FirstScreen)
    && first_screen->Height == SCREEN_HEIGHT
    && first_screen->Width  == SCREEN_WIDTH)
        game_screen = first_screen;

    // if we are running on the game scree
    if(first_screen == game_screen)
#endif
    {
        UBYTE *ptr = pal_surface->pixels;
        LONG   dlt = ptr == bufmem_roll ? -2*FRAME_BUFFER_SZ : FRAME_BUFFER_SZ;

        setFrameBufferRegs(ptr + SCREENXY(0,0), BUFFER_WIDTH - SCREEN_WIDTH);

#if ROLL_PTR
    // printf("ptr=%p // %p %p %p %p\n", ptr,bufmem, bufmem+FRAME_BUFFER_SZ, bufmem+2*FRAME_BUFFER_SZ, bufmem+3*FRAME_BUFFER_SZ);

        // need to copy parts of previous screen?
        if(copy_previous)
        {
            --copy_previous;
            if(copy_all)    blitRect(ptr+dlt, ptr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            else            blitRect(ptr+dlt, ptr, PANEL_LEFT, PANEL_TOP, PANEL_WIDTH, PANEL_HEIGHT);
        }

        // advance ptr
        pal_surface->pixels = (ptr += dlt);
#endif
    }
    doPalette();
}

int vampire_Flip(const SDL_Surface* surf)
{
    chkSignals();

    if(last_was_pal) {
        last_was_pal = 0;
        doFlip();
    } else {
        return SDL_Flip(surf);
    }
}

int vampire_BlitSurface(SDL_Surface *src, SDL_Rect *srcRect,
                        SDL_Surface *dst, SDL_Rect *dstRect)
{
    if(ac68080_saga && src==pal_surface) {
        last_was_pal = 255;
        if(!srcRect || srcRect->h>=SCREEN_HEIGHT) {
            copy_all = 255;
            copy_previous = 3;
        } else if(srcRect->y + srcRect->h > PANEL_Y // something drawn in panel
            && !(srcRect->w==288 && srcRect->h==60) // ignore descpane
        ) {
            copy_all = 0;
            copy_previous = 3;
        }
        return 0;
    } else return SDL_BlitSurface(src, srcRect, dst, dstRect);
}

#define min(a,b) ((a)<=(b)?(a):(b))

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
        d += dst-

	return 0;

}


#endif

