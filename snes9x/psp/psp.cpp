/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 
  (c) Copyright 1996 - 2002 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2001 - 2004 John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2004 Brad Jorsch (anomie@users.sourceforge.net),
                            funkyass (funkyass@spam.shaw.ca),
                            Joel Yliluoma (http://iki.fi/bisqwit/)
                            Kris Bleakley (codeviolation@hotmail.com),
                            Matthew Kendora,
                            Nach (n-a-c-h@users.sourceforge.net),
                            Peter Bortas (peter@bortas.org) and
                            zones (kasumitokoduck@yahoo.com)

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and Nach

  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2004 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman, neviksti (neviksti@hotmail.com),
                            Kris Bleakley, Andreas Naive

  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2004 zsKnight, pagefault (pagefault@zsnes.com) and
                            Kris Bleakley
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003 Brad Jorsch with research by
                     Andreas Naive and John Weidman
 
  S-RTC C emulator code
  (c) Copyright 2001 John Weidman
  
  ST010 C++ emulator code
  (c) Copyright 2003 Feather, Kris Bleakley, John Weidman and Matthew Kendora

  Super FX x86 assembler emulator code 
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault 

  Super FX C emulator code 
  (c) Copyright 1997 - 1999 Ivar, Gary Henderson and John Weidman


  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004 Marcus Comstedt (marcus@mc.pp.se) 

 
  Specific ports contains the works of other authors. See headers in
  individual files.
 
  Snes9x homepage: http://www.snes9x.com
 
  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.
 
  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.
 
  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.
 
  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.
 
  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/
#include "psp.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <pspaudio.h>
#include <pspdebug.h>
#include <sys/time.h>
#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "cpuexec.h"
#include "ppu.h"
#include "snapshot.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "soundux.h"
#include "spc700.h"
#include "spc7110.h"

//#include "font.c"
// add by y
#include "pg.h"
#include "filer.h"

#define RELEASE

/* Define the module info section */
PSP_MODULE_INFO("snes9x", 0x1000, 1, 1); 
PSP_MAIN_THREAD_ATTR(0);

#define gettimeofday sceKernelLibcGettimeofday

static struct timeval	s_tvStart;
static int				s_iFrame;
// static int				s_iFlip = 0;
// add by y
char					PBPPath[_MAX_PATH];
char					RomPath[_MAX_PATH];
#define ANALOG_NONE		0
#define ANALOG_LEFT		1
#define ANALOG_RIGHT	2
#define ANALOG_UP		3
#define ANALOG_DOWN		4
static int 				s_iAnalog = ANALOG_NONE;

extern "C" { void save_config(void); };

typedef struct
{
	char	vercnf[48];
	int		iSaveSlot;
	int		iSkipFrames;
	bool8	bShowFPS;
	bool8	bVSync;
	bool8	bSoundOff;
	int		iSoundRate;
	bool8	bTrans;
	int		iHBlankCycleDiv;
	int		iAPUTimerCycleDiv;
	bool8	bSwapAnalog;
	bool8	bSaveThumb;
	int		iPSP_ClockUp;
	int		iScreenSize;
	bool8	bAutoSkip;
} PSPSETTINGS;
PSPSETTINGS PSP_Settings;

#define UPPER_THRESHOLD		0xcf
#define LOWER_THRESHOLD		0x2f
#define PSP_CYCLE_DIV_MAX	30
volatile bool8			g_bSleep = false;
volatile bool8			g_bROMLoaded = false;

//
volatile bool8			g_bLoop = true;
int						g_thread = -1;
static uint8			SoundBuffer[MAX_BUFFER_SIZE];

//#define FRAMESIZE				0x44000			//in byte

#define SOUND_SAMPLE			1024
//#define SOUND_SAMPLE			2048

#define FIXED_POINT				0x10000
#define FIXED_POINT_SHIFT		16
#define FIXED_POINT_REMAINDER	0xffff

#define timercmp(a, b, CMP)	(((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))

static volatile bool8 block_signal = FALSE;
static volatile bool8 block_generate_sound = FALSE;
static volatile bool8 pending_signal = FALSE;

void S9xCloseSoundDevice();

#include "psp2.cpp"

extern "C" {
void *S9xProcessSound (void *);

/*	-->pg by y
#define	PIXELSIZE	1				//in short
#define	LINESIZE	512				//in short
#define CMAX_X 60
#define CMAX_Y 38

char *pg_vramtop=(char *)0x04000000;

char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return pg_vramtop+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
}

void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

	if (ch>255) return;
	cfont=font+ch*8;
	vptr0=(unsigned char*)pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((*cfont&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
						if (drawfg) *(unsigned short *)(vptr + FRAMESIZE)=color;
					} else {
//						if (drawbg) *(unsigned short *)vptr=bgcolor;
						*(unsigned short *)vptr=bgcolor;
						*(unsigned short *)(vptr + FRAMESIZE)=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}

void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,0,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}
*/

int format_int( char* buf, int value )
{
	char*	org;
	int		div;
	int		val;
	bool	bFirst;
	int		i;

	org    = buf;
	bFirst = true;
	div    = 1000000000;
	for ( i = 0; i < 10; i++ ){
		val = (unsigned)value / div;

		if ( !bFirst || val ){
			*buf++ = val + '0';
			bFirst = false;
		}

		value %= div;
		div   /= 10;
	}

	if ( bFirst ){
		*buf++ = '0';
	}
	*buf = 0;

	return strlen( org );
}

// mod by y
void debug_log( const char* message )
{
#ifndef RELEASE
	static int	sy = 1;

	if ( sy >= CMAX_Y ) {
		pgFillBoxWF(256, 0, 479, 271, 0);
		sy = 1;
	}
	pgPrintWF( SNES_WIDTH / 8, sy, 0xffff, message );
	sy++;

/*
	pgPrint( SNES_WIDTH / 8, sy, 0xffff, message );
	sy++;

	if ( sy >= CMAX_Y ){
		int 	x, y;
		uint16*	dest;

		dest = (uint16*)pgGetVramAddr( SNES_WIDTH, 0 );

		for ( y = 0; y < SCREEN_HEIGHT; y++ ){
			for ( x = 0; x < (SCREEN_WIDTH - SNES_WIDTH); x++ ){
				*dest++ = 0;
			}
			dest += (512 - (SCREEN_WIDTH - SNES_WIDTH));
		}

		sy = 1;
	}
*/
#endif // RELEASE
}

void debug_int( const char* message, int value )
{
#ifndef RELEASE
	strcpy( String, message );
	format_int( &String[strlen( String )], value );

	debug_log( String );
#endif // RELEASE
}

void debug_hex( int value )
{
#ifndef RELEASE
	int		shift;
	int		val;
	int		i;

	shift = 28;
	for ( i = 0; i < 8; i++ ){
		val = (value >> shift) & 0x0f;
		if ( val < 10 ){
			String[i] = val + '0';
		} else {
			String[i] = val - 10 + 'A';
		}
		shift -= 4;
	}
	String[i] = 0;

	debug_log( String );
#endif // RELEASE
}

static struct timeval s_analyze;
void StartAnalyze()
{
	gettimeofday( &s_analyze, 0 );
}

void StopAnalyze()
{
	struct timeval now;
	int		diff;

	gettimeofday( &now, 0 );

	diff  = (now.tv_sec - s_analyze.tv_sec) * 1000000 + now.tv_usec - s_analyze.tv_usec;

	debug_int( "time:", diff );
}
};

//
// C++ Language
//
#ifndef OPTI
void JustifierButtons(uint32&)
{
}

bool JustifierOffscreen()
{
	return false;
}
#endif // OPTI

void S9xInitCheatData()
{
}

void S9xApplyCheat( uint32 which1 )
{
}

void S9xApplyCheats()
{
}

bool8 S9xLoadCheatFile( const char *filename )
{
	return FALSE;
}

void S9xAutoSaveSRAM()
{
//	Memory.SaveSRAM (S9xGetFilename("srm"));
}

bool8 S9xOpenSoundDevice( int mode, bool8 stereo, int buffer_size )
{
	so.mute_sound  = TRUE;
/*
int		pos;

pos = format_int( buf, buffer_size );
strcat( &buf[pos], "BufSize" );

debug_log( buf );
*/
	if ( buffer_size <= 0 ){
		return FALSE;
	}

#ifndef OPTI
	so.sound_switch = 255;
	so.stereo       = TRUE;
	so.sixteen_bit  = TRUE;
#endif // OPTI
	so.buffer_size  = buffer_size;
	so.encoded      = FALSE;

	// Initialize channel and allocate buffer
	so.sound_fd = sceAudioChReserve( -1, buffer_size, 0 );
	if ( so.sound_fd < 0 ){
		return FALSE;
	}

#ifdef OPTI
	so.buffer_size *= 2;
	so.buffer_size *= 2;
#else
	if ( so.stereo ){
		so.buffer_size *= 2;
	}
	if ( so.sixteen_bit ){
		so.buffer_size *= 2;
	}
#endif // OPTI
	if ( so.buffer_size > MAX_BUFFER_SIZE ){
		so.buffer_size = MAX_BUFFER_SIZE;
	}

	S9xSetPlaybackRate( 44100 );

	so.mute_sound  = FALSE;

	return TRUE;
}

void S9xCloseSoundDevice()
{
	if ( so.sound_fd >= 0 ){
		sceAudioChRelease( so.sound_fd );
		so.sound_fd = -1;
	}
}

// add by y
bool8 S9xOpenSnapshotFile( const char *fname, bool8 read_only, STREAM *file)
{
    char filename [_MAX_PATH + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char fn [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];

    _splitpath( fname, drive, dir, fn, ext);
    _makepath( filename, drive, dir, fn, ext);

    if (read_only)
    {
	if ((*file = OPEN_STREAM (filename, "rb")))
	    return (TRUE);
    }
    else
    {
	if ((*file = OPEN_STREAM (filename, "wb")))
	    return (TRUE);
        FILE *fs = fopen (filename, "rb");
        if (fs)
        {
            sprintf (String, "Freeze file exists but is read only.");
            fclose (fs);
            S9xMessage (S9X_ERROR, S9X_FREEZE_FILE_NOT_FOUND, String);
        }
        else
        {
            sprintf (String, "Cannot create freeze file. Directory is read-only or does not exist.");
            S9xMessage (S9X_ERROR, S9X_FREEZE_FILE_NOT_FOUND, String);
        }
    }
    return (FALSE);
}

void S9xCloseSnapshotFile( STREAM file)
{
    CLOSE_STREAM (file);
}

//
// C Language
//
extern "C" {
void S9xMessage( int type, int number, const char* message )
{
	debug_log( message );
	S9xSetInfoString( message );
}

void S9xMovieUpdate ()
{
}


bool8 S9xReadSuperScopePosition (int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

bool8 S9xReadMousePosition (int which, int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

void S9xLoadSDD1Data()
{
	Settings.SDD1Pack=FALSE;
    Memory.FreeSDD1Data ();

    if (strncmp (Memory.ROMName, "Star Ocean", 10) == 0)
	{
		Settings.SDD1Pack=TRUE;
	}
    else if(strncmp(Memory.ROMName, "STREET FIGHTER ALPHA2", 21)==0)
	{
		Settings.SDD1Pack=TRUE;
	}
}

uint32 S9xReadJoypad( int which1 )
{
	SceCtrlData	ctl;
	uint32		ret;

	if ( which1 ){
		return 0;
	}

	sceCtrlReadBufferPositive( &ctl, 1 );

	ret = ((ctl.Buttons & PSP_CTRL_UP)       << 7) |	// ª
		  ((ctl.Buttons & PSP_CTRL_DOWN)     << 4) |	// «
		  ((ctl.Buttons & PSP_CTRL_LEFT)     << 2) |	// ©
		  ((ctl.Buttons & PSP_CTRL_RIGHT)    << 3) |	// ¨
		  ((ctl.Buttons & PSP_CTRL_CIRCLE)   >> 6) |	// › -> ‚`
		  ((ctl.Buttons & PSP_CTRL_CROSS)    << 1) |	// ~ -> ‚a
		  ((ctl.Buttons & PSP_CTRL_SQUARE)   >> 1) |	//   -> ‚x
		  ((ctl.Buttons & PSP_CTRL_TRIANGLE) >> 6) |	// ¢ -> ‚w
		  ((ctl.Buttons & PSP_CTRL_LTRIGGER) >> 3) |	// ‚k
		  ((ctl.Buttons & PSP_CTRL_RTRIGGER) >> 5) |	// ‚q
		  ((ctl.Buttons & PSP_CTRL_START)    << 9) |	// ‚r‚s‚`‚q‚s
		  ((ctl.Buttons & PSP_CTRL_SELECT)   << 13);	// ‚r‚d‚k‚d‚b‚s
/*
	    œ X		@¢
	Y œ@œ A		 @›
	  B œ			@~

SNES_TR			0x0010
SNES_TL			0x0020
SNES_X			0x0040
SNES_A			0x0080
SNES_RIGHT		0x0100
SNES_LEFT		0x0200
SNES_DOWN		0x0400
SNES_UP			0x0800
SNES_START		0x1000
SNES_SELECT		0x2000
SNES_Y			0x4000
SNES_B			0x8000

PSP_SQUARE		0x8000
PSP_TRIANGLE	0x1000
PSP_CIRCLE		0x2000
PSP_CROSS		0x4000
PSP_UP			0x0010
PSP_DOWN		0x0040
PSP_LEFT		0x0080
PSP_RIGHT		0x0020
PSP_START		0x0008
PSP_SELECT		0x0001
PSP_LTRIGGER	0x0100
PSP_RTRIGGER	0x0200
*/
// add by y
	if (PSP_Settings.bSwapAnalog) {
		ret &= ~(SNES_RIGHT_MASK | SNES_LEFT_MASK | SNES_DOWN_MASK | SNES_UP_MASK);
		if (ctl.Ly > UPPER_THRESHOLD) ret |= SNES_DOWN_MASK;
		if (ctl.Ly < LOWER_THRESHOLD) ret |= SNES_UP_MASK;
		if (ctl.Lx < LOWER_THRESHOLD) ret |= SNES_LEFT_MASK;
		if (ctl.Lx > UPPER_THRESHOLD) ret |= SNES_RIGHT_MASK;
		s_iAnalog = ANALOG_NONE;
		if ( ctl.Buttons & PSP_CTRL_LEFT ){
			s_iAnalog = ANALOG_LEFT;
		} else if ( ctl.Buttons & PSP_CTRL_RIGHT ){
			s_iAnalog = ANALOG_RIGHT;
		} else if ( ctl.Buttons & PSP_CTRL_UP ){
			s_iAnalog = ANALOG_UP;
		} else if ( ctl.Buttons & PSP_CTRL_DOWN ){
			s_iAnalog = ANALOG_DOWN;
		}
	} else {
		s_iAnalog = ANALOG_NONE;
		if ( ctl.Lx == 0x00 ){
			s_iAnalog = ANALOG_LEFT;
		} else if ( ctl.Lx == 0xff ){
			s_iAnalog = ANALOG_RIGHT;
		} else if ( ctl.Ly == 0x00 ){
			s_iAnalog = ANALOG_UP;
		} else if ( ctl.Ly == 0xff ){
			s_iAnalog = ANALOG_DOWN;
		}
	}
	return ret | 0x80000000;
}

void S9xSetPalette()
{
}

bool8 S9xSPCDump( const char *filename )
{
	return FALSE;
}

#define MAXVOLUME	0x8000

void S9xGenerateSound()
{
#ifndef PSP
    /* Linux and Sun versions */
    
#ifdef OPTI
    int bytes_so_far = so.samples_mixed_so_far << 1;
#else
    int bytes_so_far = so.sixteen_bit ? (so.samples_mixed_so_far << 1) :
				        so.samples_mixed_so_far;
#endif // OPTI
	if (Settings.SoundSync == 2)
 	{
 	// Assumes sound is signal driven
//	while (so.samples_mixed_so_far >= so.buffer_size && !so.mute_sound)
//	    pause ();
    }
    else
    if (bytes_so_far >= so.buffer_size)
	return;

    if (Settings.ThreadSound)
    {
//	if (block_generate_sound || pthread_mutex_trylock (&mutex))
	if (block_generate_sound)
	    return;
    }

    block_signal = TRUE;

    so.err_counter += so.err_rate;
    if (so.err_counter >= FIXED_POINT)
    {
        int sample_count = so.err_counter >> FIXED_POINT_SHIFT;
		int byte_offset;
		int byte_count;

        so.err_counter &= FIXED_POINT_REMAINDER;
#ifndef OPTI
	if (so.stereo)
#endif // OPTI
	    sample_count <<= 1;
		byte_offset = bytes_so_far + so.play_position;
	    
	do
	{
	    int sc = sample_count;
	    byte_count = sample_count;
#ifndef OPTI
	    if (so.sixteen_bit)
#endif // OPTI
		byte_count <<= 1;
	    
	    if ((byte_offset & SOUND_BUFFER_SIZE_MASK) + byte_count > SOUND_BUFFER_SIZE)
	    {
			sc = SOUND_BUFFER_SIZE - (byte_offset & SOUND_BUFFER_SIZE_MASK);
			byte_count = sc;
#ifndef OPTI
		if (so.sixteen_bit)
#endif // OPTI
		    sc >>= 1;
	    }
	    if (bytes_so_far + byte_count > so.buffer_size)
	    {
			byte_count = so.buffer_size - bytes_so_far;
			if (byte_count == 0)
		    	break;
			sc = byte_count;
#ifndef OPTI
		if (so.sixteen_bit)
#endif // OPTI
		    sc >>= 1;
	    }
	    S9xMixSamplesO (SoundBuffer, sc,
			    byte_offset & SOUND_BUFFER_SIZE_MASK);
	    so.samples_mixed_so_far += sc;
	    sample_count -= sc;
#ifdef OPTI
	    bytes_so_far = so.samples_mixed_so_far << 1;
#else
	    bytes_so_far = so.sixteen_bit ? (so.samples_mixed_so_far << 1) :
	 	           so.samples_mixed_so_far;
#endif // OPTI
	    byte_offset += byte_count;
	} while (sample_count > 0);
    }
    block_signal = FALSE;

	if (Settings.ThreadSound)
		;
//	pthread_mutex_unlock (&mutex);
    else
    if (pending_signal)
	{
		S9xProcessSound (NULL);
		pending_signal = FALSE;
	}
#endif // PSP
}

void S9xExit()
{
//	Memory.SaveSRAM (S9xGetSRAMFilename ());
	Memory.Deinit();

//	exit (0);
}

void CopyAudio( char* buf, int len )
{
	static int	pos  = 0;
	static char	tmp[SOUND_SAMPLE * 2 * 2];
	int		i;

	for ( i = 0; i < len; i++ ){
		tmp[pos++] = buf[i];
		if ( pos >= (SOUND_SAMPLE * 2 * 2) ){
			sceAudioOutputPannedBlocking( so.sound_fd, MAXVOLUME, MAXVOLUME, (char*)tmp );
			pos = 0;
		}
	}
}

// mod by y
void *S9xProcessSound (void *)
{
//debug_log( "Thread start!" );
    /* Linux and Sun versions */
    
    /* If threads in use, this is to loop indefinitely */
    /* If not, this will be called by timer */
    
    do
    {
//		sceDisplayWaitVblankStart();

    /* Number of samples to generate now */
    int sample_count = so.buffer_size;
    unsigned byte_offset;
    
#ifndef OPTI
    if (so.sixteen_bit)
#endif // OPTI
    {
        /* to prevent running out of buffer space,
         * create less samples
         */
		sample_count >>= 1;
    }

//	if (Settings.ThreadSound)
//		;
//	pthread_mutex_lock (&mutex);
//	else
#ifndef PSP
    if (!Settings.ThreadSound) {
		if (block_signal)
		{
			pending_signal = TRUE;
			return (NULL);
		}
	}
	block_generate_sound = TRUE;
#endif // PSP
    /* If we need more audio samples */
    if (so.samples_mixed_so_far < sample_count)
    {
	/* Where to put the samples to */
#ifdef OPTI
		byte_offset = so.play_position + (so.samples_mixed_so_far << 1);
#else
		byte_offset = so.play_position + 
		      (so.sixteen_bit ? (so.samples_mixed_so_far << 1)
				      : so.samples_mixed_so_far);
#endif // OPTI
//printf ("%d:", sample_count - so.samples_mixed_so_far); fflush (stdout);
#ifdef PSP
		S9xMixSamplesO (SoundBuffer, sample_count - so.samples_mixed_so_far,
			byte_offset & SOUND_BUFFER_SIZE_MASK);
#else
		if (Settings.SoundSync == 2)
		{
//			memset (SoundBuffer + (byte_offset & SOUND_BUFFER_SIZE_MASK), 0,
//			    sample_count - so.samples_mixed_so_far);
		} else {
		    /* Mix the missing samples */
		    S9xMixSamplesO (SoundBuffer, sample_count - so.samples_mixed_so_far,
				    byte_offset & SOUND_BUFFER_SIZE_MASK);
		}
#endif // PSP
		so.samples_mixed_so_far = 0;
    } else {
		so.samples_mixed_so_far -= sample_count;
    }
    
//    if (!so.mute_sound)
    {
#ifdef OPTI
		unsigned bytes_to_write = so.buffer_size;
//		bytes_to_write <<= 1;
#else
		unsigned bytes_to_write = sample_count;
		if(so.sixteen_bit) bytes_to_write <<= 1;
#endif // OPTI

		byte_offset = so.play_position;
		so.play_position = (so.play_position + so.buffer_size) & SOUND_BUFFER_SIZE_MASK;
//	so.play_position += bytes_to_write;
//	so.play_position &= SOUND_BUFFER_SIZE_MASK; /* wrap to beginning */

#ifndef PSP
	if (Settings.ThreadSound)
//	    pthread_mutex_unlock (&mutex);
		block_generate_sound = FALSE;
#endif // PSP

	/* Feed the samples to the soundcard until nothing is left */
		for(;;)
		{
		    int I = bytes_to_write;
		    if (byte_offset + I > SOUND_BUFFER_SIZE)
		    {
		        I = SOUND_BUFFER_SIZE - byte_offset;
		    }
		    if(I == 0) break;
	    
//            I = write (so.sound_fd, (char *) Buf + byte_offset, I);
#if 0
			CopyAudio( (char*)SoundBuffer + byte_offset, I );
#else
			sceAudioOutputPannedBlocking( so.sound_fd, MAXVOLUME, MAXVOLUME, (char*)SoundBuffer + byte_offset );
//debug_log( "sceAudio_2" );
#endif
            if (I > 0)
            {
                bytes_to_write -= I;
                byte_offset += I;
                byte_offset &= SOUND_BUFFER_SIZE_MASK; /* wrap */
            }
            /* give up if an unrecoverable error happened */
//            if(I < 0 && errno != EINTR) break;
		}
	/* All data sent. */
    }
//    so.samples_mixed_so_far -= sample_count;
    } while (Settings.ThreadSound);
//debug_log( "Thread end" );

    return (NULL);
}

void InitTimer()
{
	debug_log( "Create Thread" );
	g_thread = sceKernelCreateThread( "sound thread", (void *)S9xProcessSound, 0x8, 0x40000, THREAD_ATTR_USER, 0 );
	if ( g_thread < 0 ){
		debug_log( "Thread failed" );
		return;
	}

	sceKernelStartThread( g_thread, 0, 0 );

	debug_log( "Thread ok" );
}

//mod by y
void S9xSyncSpeed()
{
#ifndef RELEASE
	S9xProcessEvents( FALSE );

	IPPU.FrameSkip = 0;
	IPPU.SkippedFrames = 0;
	IPPU.RenderThisFrame = TRUE;

	return;
#else
	S9xProcessEvents( FALSE );
	if (!PSP_Settings.bAutoSkip || Settings.SkipFrames == 0) {
		if (++IPPU.FrameSkip >= Settings.SkipFrames) {
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		} else {
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}
		return;
	} else {
		static struct timeval next1 = {0, 0};
		struct timeval now;
		
		while (gettimeofday(&now, 0) < 0) ;
		if (next1.tv_sec == 0) {
			next1 = now;
			next1.tv_usec++;
		}
		if (timercmp(&next1, &now, >)) {
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
			next1 = now;
		} else {
			if (++IPPU.FrameSkip >= Settings.SkipFrames) {
				IPPU.FrameSkip = 0;
				IPPU.SkippedFrames = 0;
				IPPU.RenderThisFrame = TRUE;
				next1 = now;
			} else {
				IPPU.SkippedFrames++;
				IPPU.RenderThisFrame = FALSE;
			}
		}
		next1.tv_usec += Settings.FrameTime * (IPPU.FrameSkip + 1) ;
		if (next1.tv_usec >= 1000000) {
			next1.tv_sec += next1.tv_usec / 1000000;
			next1.tv_usec %= 1000000;
		}
		return;
	}
/*
	static struct timeval next1 = { 0, 0 };
	struct timeval now;

	CHECK_SOUND(); S9xProcessEvents( FALSE );

	sceKernelLibcGettimeofday( &now, 0 );
	if ( next1.tv_sec == 0 ){
		next1 = now;
		++next1.tv_usec;
	}
*/

#if 1
/*
	unsigned limit = Settings.SkipFrames;

	IPPU.RenderThisFrame = ++IPPU.SkippedFrames >= limit;
	if ( IPPU.RenderThisFrame ){
		IPPU.SkippedFrames = 0;
	}
*/
#else
#if 0
	if ( timercmp( &next1, &now, >= ) ){
 		if ( IPPU.SkippedFrames == 0 ){
			while ( timercmp( &next1, &now, > ) ){
				sceKernelLibcGettimeofday( &now, 0 );
			}
		}
		IPPU.RenderThisFrame = TRUE;
		IPPU.SkippedFrames = 0;
	} else {
		if ( IPPU.SkippedFrames < Settings.AutoMaxSkipFrames ){
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		} else {
			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
			next1 = now;
		}
	}
#else
	unsigned limit = Settings.SkipFrames == AUTO_FRAMERATE ? (timercmp( &next1, &now, < ) ? Settings.AutoMaxSkipFrames : 1) : Settings.SkipFrames;

	IPPU.RenderThisFrame = ++IPPU.SkippedFrames >= limit;
	if ( IPPU.RenderThisFrame ){
		IPPU.SkippedFrames = 0;
	} else {
		if ( timercmp( &next1, &now, < ) ){
			unsigned int lag;
			lag = (now.tv_sec - next1.tv_sec) * 1000000 + now.tv_usec - next1.tv_usec;
			if ( lag >= 1000000 ){
				next1 = now;
			}
		}
	}

	while ( timercmp( &next1, &now, > ) ){
/*
		unsigned timeleft;

		timeleft = (next1.tv_sec - now.tv_sec) * 1000000 + next1.tv_usec - now.tv_usec;
		usleep( timeleft );

		CHECK_SOUND(); S9xProcessEvents( FALSE );
*/
		gettimeofday( &now, 0 );
	}
#endif
	next1.tv_usec += Settings.FrameTime;
	if ( next1.tv_usec >= 1000000 ){
		next1.tv_sec += next1.tv_usec / 1000000;
		next1.tv_usec %= 1000000;
	}
#endif

#endif // RELEASE
}


const char *S9xGetFilename( const char *e )
{
	static char filename [_MAX_PATH + 1];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	char fname [_MAX_FNAME + 1];
	char ext [_MAX_EXT + 1];

	_splitpath (Memory.ROMFilename, drive, dir, fname, ext);
// add by y
	if (strcasecmp (e, "srm") == 0) {
		strcpy(filename,PBPPath);
		strcat(filename,"SAVE/");
		strcat(filename,fname);
		strcat(filename,".srm");
	} else if (strcasecmp (e, "sv0") == 0) {
		strcpy(filename,PBPPath);
		strcat(filename,"SAVE/");
		strcat(filename,fname);
		strcat(filename,".sv0");
		filename[strlen(filename)-1] = PSP_Settings.iSaveSlot + '0';
	} else if (strcasecmp (e, "tn0") == 0) {
		strcpy(filename,PBPPath);
		strcat(filename,"SAVE/");
		strcat(filename,fname);
		strcat(filename,".tn0");
		filename[strlen(filename)-1] = PSP_Settings.iSaveSlot + '0';
	} else if (strcasecmp (e, "cfg") == 0) {
		strcpy(filename,PBPPath);
		strcat(filename,"SAVE/");
		strcat(filename,fname);
		strcat(filename,"_psp.cfg");
	} else {
		_makepath (filename, drive, dir, fname, e);
	}
	return (filename);
}

bool8 S9xInitUpdate()
{
	return TRUE;
}

#if 0
void S9xPutImage( int width, int height )
{
	uint32*	dest;
	uint32* pBuffer;
	int		x;

	pBuffer = (uint32*)GFX.Screen;
#ifdef RELEASE
	dest    = (uint32*)pgGetVramAddr( (SCREEN_WIDTH - SNES_WIDTH) >> 1, (SCREEN_HEIGHT - height) >> 1 );
#else
	dest    = (uint32*)pgGetVramAddr( 0, 0 );
#endif // RELEASE

	while ( height-- ){
		x = 16;
		while ( x-- ){
			uint32 data;
			*dest++ = *pBuffer++;
			*dest++ = *pBuffer++;
			*dest++ = *pBuffer++;
			*dest++ = *pBuffer++;
			*dest++ = *pBuffer++;
			*dest++ = *pBuffer++;
			*dest++ = *pBuffer++;
			*dest++ = *pBuffer++;
		}
		dest += (512 - SNES_WIDTH) / 2;
	}
}
void S9xPutImage( int width, int height )
{
	int 	x, y;
	uint16*	src;
	uint16*	dest;

	src  = (uint16*)GFX.Screen;
//	dest = (uint16*)(VRAM_ADDR + 0x40000000);
	dest = (uint16*)pgGetVramAddr( 0, 0 );

//	pspDisplayWaitVblankStart();

	for ( y = 0; y < SNES_HEIGHT_EXTENDED; y++ ){
		for ( x = 0; x < SNES_WIDTH; x++ ){
			*dest++ = *src++;
		}
		dest += (512 - SNES_WIDTH);
	}
}
#endif

enum {
	SCR_X1,
	SCR_FIT,
	SCR_FULL,
	SCR_FULLFIT,
};

static char FPSbuf[6];
#ifdef OPTI
bool8 S9xDeinitUpdate (int Width, int Height)
#else
bool8 S9xDeinitUpdate (int Width, int Height, bool8 sixteen_bit)
#endif // OPTI
{
//	S9xPutImage( Width, Height );
// mod by y
	switch(PSP_Settings.iScreenSize) {
		case SCR_FIT:
		case SCR_FULL:
		case SCR_FULLFIT:
			pgRenderTex((char *)GFX.Screen,256,256,0,0,480,315);
			break;
		case SCR_X1:
		default:
			pgRenderTex((char *)GFX.Screen,256,256,(480/2)-(256/2),(272/2)-(224/2),256,256);
	}
	if (PSP_Settings.bShowFPS) {
		struct timeval	now;
		unsigned int	diff;
//		char buf[128];
	
		s_iFrame++;
		gettimeofday( &now, 0 );
	
		diff  = (now.tv_sec - s_tvStart.tv_sec) * 1000000 + now.tv_usec - s_tvStart.tv_usec;
		diff /= 1000000;
	
		if ( diff ){
			FPSbuf[0] = ((s_iFrame / diff) / 10) + '0';
			FPSbuf[1] = ((s_iFrame / diff) % 10) + '0';
			FPSbuf[2] = 'F';
			FPSbuf[3] = 'P';
			FPSbuf[4] = 'S';
			FPSbuf[5] = '\0';
	
			s_tvStart = now;
			s_iFrame  = 0;
		}
		pgPrintBG( CMAX_X - 6, 0, 0xffff, FPSbuf );
	}
	if (PSP_Settings.bVSync) {
		pgScreenFlipV();
	} else {
		pgScreenFlip();
	}

	return TRUE;
}

static uint8 __attribute__((aligned(16))) GFX_Screen[256 * 256 * 2];
static uint8	GFX_SubScreen[256 * 256 * 2];
static uint8	GFX_ZBuffer[256 * 256 * 2];
static uint8	GFX_SubZBuffer[256 * 256 * 2];

// mod by y
void S9xInitDisplay( int argc, char** argv )
{
	PSP_Settings.bTrans = true;
	Settings.Transparency = TRUE;
#ifndef OPTI
	Settings.SixteenBit   = TRUE;
#endif // OPTI
	Settings.SupportHiRes = FALSE; //interpolate;

	memset( GFX_Screen,	0, sizeof(GFX_Screen) );
	memset( GFX_SubScreen,  0, sizeof(GFX_SubScreen) );
	memset( GFX_ZBuffer,    0, sizeof(GFX_ZBuffer) );
	memset( GFX_SubZBuffer, 0, sizeof(GFX_SubZBuffer) );

//	GFX.Pitch      = IMAGE_WIDTH * 2;
	GFX.Pitch      = 256 * 2;
	GFX.Screen     = (uint8*)GFX_Screen;
	GFX.SubScreen  = (uint8*)GFX_SubScreen;
	GFX.ZBuffer    = (uint8*)GFX_ZBuffer;
	GFX.SubZBuffer = (uint8*)GFX_SubZBuffer;
}

void S9xInitInputDevices()
{
}

// add by y
void fix_H_Max(void)
{
	//CPU timing hacks
	// A Couple of HDMA related hacks - Lantus
	if ((strcmp(Memory.ROMName, "SFX SUPERBUTOUDEN2")==0) ||
	    (strcmp(Memory.ROMName, "ALIEN vs. PREDATOR")==0) ||
		(strcmp(Memory.ROMName, "STONE PROTECTORS")==0) ||
	    (strcmp(Memory.ROMName, "SUPER BATTLETANK 2")==0))
		Settings.H_Max = (Settings.H_Max * 130) / 100;
	if(strcmp(Memory.ROMName, "HOME IMPROVEMENT")==0)
		Settings.H_Max = (Settings.H_Max * 200) / 100;
    if (strcmp (Memory.ROMId, "ASRJ") == 0 && Settings.CyclesPercentage == 100)
		// Street Racer
		Settings.H_Max = (Settings.H_Max * 95) / 100;
	// Power Rangers Fight
    if (strncmp (Memory.ROMId, "A3R", 3) == 0 ||
        // Clock Tower
		strncmp (Memory.ROMId, "AJE", 3) == 0)
		Settings.H_Max = (Settings.H_Max * 103) / 100;
    if (strncmp (Memory.ROMId, "A3M", 3) == 0 && Settings.CyclesPercentage == 100)
		// Mortal Kombat 3. Fixes cut off speech sample
		Settings.H_Max = (Settings.H_Max * 110) / 100;
    if (strcmp (Memory.ROMName, "\x0bd\x0da\x0b2\x0d4\x0b0\x0bd\x0de") == 0 &&
		Settings.CyclesPercentage == 100)
		Settings.H_Max = (Settings.H_Max * 101) / 100;
    // Start Trek: Deep Sleep 9
    if (strncmp (Memory.ROMId, "A9D", 3) == 0 && Settings.CyclesPercentage == 100)
		Settings.H_Max = (Settings.H_Max * 110) / 100;
}

void save_config(void)
{
	char CFGPath [_MAX_PATH + 1];
	strcpy(CFGPath, S9xGetFilename("cfg"));
	FILE *fd;
	if ((fd = fopen (CFGPath, "wb"))) {
		fwrite (&PSP_Settings, sizeof(PSP_Settings), 1, fd);
		fclose (fd);
	}
}

void load_config(void)
{
	if (!PSP_Settings.bSoundOff) {
		S9xSetSoundMute( TRUE );
		if ( g_thread !=-1 ){
			Settings.ThreadSound = FALSE;
			sceKernelWaitThreadEnd( g_thread, NULL );
			sceKernelDeleteThread( g_thread );
			g_thread = -1;
		}
	}
	
	memset( &PSP_Settings, 0, sizeof( PSP_Settings ) );
	
	char CFGPath [_MAX_PATH + 1];
	strcpy(CFGPath, S9xGetFilename("cfg"));
	FILE *fd;
	if ((fd = fopen (CFGPath, "rb"))) {
		fread (&PSP_Settings, sizeof(PSP_Settings), 1, fd);
		fclose (fd);
		
		if (PSP_Settings.iSaveSlot < 0 || PSP_Settings.iSaveSlot > SAVE_SLOT_MAX)
			PSP_Settings.iSaveSlot = 0;
		if (PSP_Settings.iSkipFrames < 0 || PSP_Settings.iSkipFrames > 10)
			PSP_Settings.iSkipFrames = 0;
		if (!PSP_Settings.bShowFPS)
			PSP_Settings.bShowFPS = false;
		if (!PSP_Settings.bVSync)
			PSP_Settings.bVSync = false;
		if (!PSP_Settings.bSoundOff)
			PSP_Settings.bSoundOff = false;
		if (PSP_Settings.iSoundRate < 0 || PSP_Settings.iSoundRate > 2)
			PSP_Settings.iSoundRate = 2;
		if (!PSP_Settings.bTrans)
			PSP_Settings.bTrans = false;
		if (PSP_Settings.iHBlankCycleDiv < 0 || PSP_Settings.iHBlankCycleDiv > PSP_CYCLE_DIV_MAX)
			PSP_Settings.iHBlankCycleDiv = 10;
		if (PSP_Settings.iAPUTimerCycleDiv < 0 || PSP_Settings.iAPUTimerCycleDiv > PSP_CYCLE_DIV_MAX)
			PSP_Settings.iAPUTimerCycleDiv = 10;
		if (!PSP_Settings.bSwapAnalog)
			PSP_Settings.bSwapAnalog = false;
		if (!PSP_Settings.bSaveThumb)
			PSP_Settings.bSaveThumb = false;
		if (PSP_Settings.iPSP_ClockUp < 0 || PSP_Settings.iPSP_ClockUp > 2)
			PSP_Settings.iPSP_ClockUp = 0;
		if (PSP_Settings.iScreenSize < 0 || PSP_Settings.iScreenSize > 3)
			PSP_Settings.iScreenSize = 0;
		if (!PSP_Settings.bAutoSkip)
			PSP_Settings.bAutoSkip = false;
	}
	
	char vercnf[48];
	strcpy(vercnf, "uo_Snes9x for PSP Ver.0.02y11");
	if (strcmp(PSP_Settings.vercnf, vercnf) != 0) {
		strcpy(PSP_Settings.vercnf, vercnf);
		PSP_Settings.iSaveSlot = 0;
		PSP_Settings.iSkipFrames = 0;
		PSP_Settings.bShowFPS = false;
		PSP_Settings.bVSync = false;
		PSP_Settings.bSoundOff = false;
		PSP_Settings.iSoundRate = 2;
		PSP_Settings.bTrans = true;
		PSP_Settings.iHBlankCycleDiv = 10;
		PSP_Settings.iAPUTimerCycleDiv = 10;
		PSP_Settings.bSwapAnalog = false;
		PSP_Settings.bSaveThumb = true;
		PSP_Settings.iPSP_ClockUp = 0;
		PSP_Settings.iScreenSize = 0;
		PSP_Settings.bAutoSkip = false;
	}
	
	if (PSP_Settings.iSoundRate==0) S9xSetPlaybackRate( 11025 );
	else if (PSP_Settings.iSoundRate==1) S9xSetPlaybackRate( 22050 );
	else S9xSetPlaybackRate( 44100 );
	if (!PSP_Settings.bSoundOff) {
		Settings.ThreadSound = TRUE;
		InitTimer();
	}
	Settings.H_Max = (long)((SNES_CYCLES_PER_SCANLINE * 1000) / ( PSP_Settings.iHBlankCycleDiv * 100));
	Settings.HBlankStart = ((uint32)Settings.H_Max << 8) / SNES_HCOUNTER_MAX;
	fix_H_Max();
	Settings.PSP_APUTimerCycle = (uint32)((SNES_APUTIMER2_CYCLEx10000 * 1000) / (PSP_Settings.iAPUTimerCycleDiv * 100));
	if (PSP_Settings.bTrans){
		Settings.Transparency = TRUE;
	} else {
		Settings.Transparency = FALSE;
	}
	Settings.SkipFrames = PSP_Settings.iSkipFrames;
	if (PSP_Settings.iPSP_ClockUp == 2) scePowerSetClockFrequency(333,333,166);
	else if (PSP_Settings.iPSP_ClockUp == 1) scePowerSetClockFrequency(266,266,133);
	else scePowerSetClockFrequency(222,222,111);
	g_bSleep = false;
}

void open_menu(void)
{
	enum {
		SRAM_SAVE,
		STATE_SLOT,
		STATE_SAVE,
		STATE_LOAD,
		STATE_DEL,
		SAVE_THUMB,

		FRAME_SKIP,
		AUTO_SKIP,
		SCR_SIZE,
		SHOWFPS,
		VSYNC,
		SOUNDOFF,
		SOUNDRATE,
		TRANS,
		HBLANK_CYCLE,
		APU_CYCLE,
		SWAP_ANALOG,
		PSP_CLOCKUP,

		LOAD_ROM,
		RESET,
		EXIT_HOME,
		CONTINUE,
	};
	char msg[256], tmp[256];
	static int sel=0;
	int x, y;
	int ret;
	
	get_screenshot((unsigned char *)GFX.Screen);
	scePowerSetClockFrequency(222,222,111);
	get_slotdate(S9xGetFilename("sv0"));
	get_thumbs(S9xGetFilename("tn0"));
	
	bool f_bExit = false;
	bool f_bOldSoundOff = PSP_Settings.bSoundOff;
	int f_iOldSoundRate = PSP_Settings.iSoundRate;
	uint8 tmp_color;
	
	old_pad = PSP_CTRL_LEFT;
	
	msg[0]=0;
	readpad();
	for(;;){
		if (new_pad & PSP_CTRL_CIRCLE){
			if (sel == SRAM_SAVE){
				save_config();
				if ( Memory.SaveSRAM( S9xGetFilename("srm") ) ) {
						strcpy(msg, "SRAM Saved Successfully.");
				} else {
						strcpy(msg, "SRAM Save Failed or Not Found SRAM.");
				}
			} else if (sel == STATE_SLOT){
				PSP_Settings.iSaveSlot++;
				if (PSP_Settings.iSaveSlot > SAVE_SLOT_MAX) {
					PSP_Settings.iSaveSlot = 0;
				}
			} else if (sel == STATE_SAVE){
				pgFillBox( 129, 104, 351, 168, 0 );
				mh_print(195, 132, (unsigned char*)"Now State Saving...", RGB(255,205,0));
				save_config();
				Memory.SaveSRAM( S9xGetFilename("srm") );
				if ( S9xFreezeGame( S9xGetFilename("sv0") ) ) {
						if (PSP_Settings.bSaveThumb) {
							save_thumb(S9xGetFilename("tn0"));
						} else {
							delete_file(S9xGetFilename("tn0"));
						}
						strcpy(msg, "State Saved Successfully.");
						get_slotdate(S9xGetFilename("sv0"));
						get_thumbs(S9xGetFilename("tn0"));
				} else {
						strcpy(msg, "State Save Failed.");
				}
				S9xSetSoundMute( TRUE );
			} else if (sel == STATE_LOAD){
				if ( S9xUnfreezeGame( S9xGetFilename("sv0") ) ) {
					Memory.LoadSRAM( S9xGetFilename("srm") );
					S9xSetInfoString( "State Loaded." );
					break;
				} else {
					strcpy(msg, "State Load Failed.");
				}
			} else if (sel == STATE_DEL){
				if (slotflag[PSP_Settings.iSaveSlot]) {
					pgFillBox( 129, 104, 351, 168, 0 );
					strcpy(tmp,"Are you sure to delete .sv0 H");
					tmp[strlen(tmp)-4] = PSP_Settings.iSaveSlot + '0';
					mh_print(165, 122, (unsigned char*)tmp, RGB(255,255,0));
					mh_print(195, 142, (unsigned char*)"›FOK  ~FCancel", RGB(105,105,115));
					for(;;){
						readpad();
						if (new_pad & PSP_CTRL_CIRCLE){
							delete_file(S9xGetFilename("sv0"));
							delete_file(S9xGetFilename("tn0"));
							get_slotdate(S9xGetFilename("sv0"));
							get_thumbs(S9xGetFilename("tn0"));
							break;
						} else if (new_pad & PSP_CTRL_CROSS){
							break;
						}
					}
				}
			} else if (sel == SAVE_THUMB){
				PSP_Settings.bSaveThumb = !PSP_Settings.bSaveThumb;
			} else if (sel == FRAME_SKIP){
				PSP_Settings.iSkipFrames++;
				if ( PSP_Settings.iSkipFrames > 10 ){
					PSP_Settings.iSkipFrames = 0;
				}
			} else if (sel == AUTO_SKIP){
				PSP_Settings.bAutoSkip = !PSP_Settings.bAutoSkip;
			} else if (sel == SCR_SIZE){
				PSP_Settings.iScreenSize++;
				if (PSP_Settings.iScreenSize > 3) {
					PSP_Settings.iScreenSize = 0;
				}
			} else if (sel == SHOWFPS){
				PSP_Settings.bShowFPS = !PSP_Settings.bShowFPS;
			} else if (sel == VSYNC){
				PSP_Settings.bVSync = !PSP_Settings.bVSync;
			} else if (sel == SOUNDOFF){
				PSP_Settings.bSoundOff = !PSP_Settings.bSoundOff;
			} else if (sel == SOUNDRATE){
				PSP_Settings.iSoundRate++;
				if (PSP_Settings.iSoundRate > 2) {
					PSP_Settings.iSoundRate = 0;
				}
			} else if (sel == TRANS){
				PSP_Settings.bTrans = !PSP_Settings.bTrans;
			} else if (sel == HBLANK_CYCLE){
				PSP_Settings.iHBlankCycleDiv++;
				if (PSP_Settings.iHBlankCycleDiv > PSP_CYCLE_DIV_MAX) {
					PSP_Settings.iHBlankCycleDiv = 10;
				}
			} else if (sel == APU_CYCLE){
				PSP_Settings.iAPUTimerCycleDiv++;
				if (PSP_Settings.iAPUTimerCycleDiv > PSP_CYCLE_DIV_MAX) {
					PSP_Settings.iAPUTimerCycleDiv = 10;
				}
			} else if (sel == SWAP_ANALOG){
				PSP_Settings.bSwapAnalog = !PSP_Settings.bSwapAnalog;
			} else if (sel == PSP_CLOCKUP){
				PSP_Settings.iPSP_ClockUp++;
				if (PSP_Settings.iPSP_ClockUp > 2) {
					PSP_Settings.iPSP_ClockUp = 0;
				}
			} else if (sel == LOAD_ROM){
				msg[0]=0;
				FilerMsg[0]=0;
				if (getFilePath(RomPath)){
					save_config();
					Memory.SaveSRAM( S9xGetFilename("srm") );
					if ( Memory.LoadROM(RomPath) ){
						Memory.LoadSRAM( S9xGetFilename("srm") );
						load_config();
						S9xSetInfoString( "ROM image Loaded." );
						return;
					} else {
						strcpy(msg, "Rom image Load Failed.");
					}
				}
			} else if (sel == RESET){
				S9xReset();
				S9xSetInfoString( "Game Reseted." );
				break;
			} else if (sel == EXIT_HOME){
				f_bExit = true;
				break;
			} else if (sel == CONTINUE){
				break;
			}
		} else if (new_pad & PSP_CTRL_CROSS){
			break;
		} else if (new_pad & PSP_CTRL_UP){
			if (sel!=0){
				sel--;
			} else {
				sel=CONTINUE;
			}
		} else if (new_pad & PSP_CTRL_DOWN){
			if (sel!=CONTINUE){
				sel++;
			} else {
				sel=0;
			}
		} else if (new_pad & PSP_CTRL_LEFT){
			if (sel == STATE_SLOT || sel == STATE_SAVE || sel == STATE_LOAD || sel == STATE_DEL){
				if (PSP_Settings.iSaveSlot < 1) {
					PSP_Settings.iSaveSlot = SAVE_SLOT_MAX + 1;
				}
				PSP_Settings.iSaveSlot--;
			} else if (sel == SAVE_THUMB){
				PSP_Settings.bSaveThumb = !PSP_Settings.bSaveThumb;
			} else if (sel == FRAME_SKIP){
				if ( PSP_Settings.iSkipFrames > 0 ){
					PSP_Settings.iSkipFrames--;
				}
			} else if (sel == AUTO_SKIP){
				PSP_Settings.bAutoSkip = true;
			} else if (sel == SCR_SIZE){
				if (PSP_Settings.iScreenSize < 1) {
					PSP_Settings.iScreenSize = 1;
				}
				PSP_Settings.iScreenSize--;
			} else if (sel == SHOWFPS){
				PSP_Settings.bShowFPS = true;
			} else if (sel == VSYNC){
				PSP_Settings.bVSync = true;
			} else if (sel == SOUNDOFF){
				PSP_Settings.bSoundOff = false;
			} else if (sel == SOUNDRATE){
				if (PSP_Settings.iSoundRate < 1) {
					PSP_Settings.iSoundRate = 1;
				}
				PSP_Settings.iSoundRate--;
			} else if (sel == TRANS){
				PSP_Settings.bTrans = true;
			} else if (sel == HBLANK_CYCLE){
				if (PSP_Settings.iHBlankCycleDiv < 11) {
					PSP_Settings.iHBlankCycleDiv = 11;
				}
				PSP_Settings.iHBlankCycleDiv--;
			} else if (sel == APU_CYCLE){
				if (PSP_Settings.iAPUTimerCycleDiv < 11) {
					PSP_Settings.iAPUTimerCycleDiv = 11;
				}
				PSP_Settings.iAPUTimerCycleDiv--;
			} else if (sel == SWAP_ANALOG){
				PSP_Settings.bSwapAnalog = !PSP_Settings.bSwapAnalog;
			} else if (sel == PSP_CLOCKUP){
				if (PSP_Settings.iPSP_ClockUp < 1) {
					PSP_Settings.iPSP_ClockUp = 1;
				}
				PSP_Settings.iPSP_ClockUp--;
			}
		} else if (new_pad & PSP_CTRL_RIGHT){
			if (sel == STATE_SLOT || sel == STATE_SAVE || sel == STATE_LOAD || sel == STATE_DEL){
				PSP_Settings.iSaveSlot++;
				if (PSP_Settings.iSaveSlot > SAVE_SLOT_MAX) {
					PSP_Settings.iSaveSlot = 0;
				}
			} else if (sel == SAVE_THUMB){
				PSP_Settings.bSaveThumb = !PSP_Settings.bSaveThumb;
			} else if (sel == FRAME_SKIP){
				if ( PSP_Settings.iSkipFrames < 10 ){
					PSP_Settings.iSkipFrames++;
				}
			} else if (sel == AUTO_SKIP){
				PSP_Settings.bAutoSkip = false;
			} else if (sel == SCR_SIZE){
				PSP_Settings.iScreenSize++;
				if (PSP_Settings.iScreenSize > 3) {
					PSP_Settings.iScreenSize = 3;
				}
			} else if (sel == SHOWFPS){
				PSP_Settings.bShowFPS = false;
			} else if (sel == VSYNC){
				PSP_Settings.bVSync = false;
			} else if (sel == SOUNDOFF){
				PSP_Settings.bSoundOff = true;
			} else if (sel == SOUNDRATE){
				PSP_Settings.iSoundRate++;
				if (PSP_Settings.iSoundRate > 2) {
					PSP_Settings.iSoundRate = 2;
				}
			} else if (sel == TRANS){
				PSP_Settings.bTrans = false;
			} else if (sel == HBLANK_CYCLE){
				PSP_Settings.iHBlankCycleDiv++;
				if (PSP_Settings.iHBlankCycleDiv > PSP_CYCLE_DIV_MAX) {
					PSP_Settings.iHBlankCycleDiv = PSP_CYCLE_DIV_MAX;
				}
			} else if (sel == APU_CYCLE){
				PSP_Settings.iAPUTimerCycleDiv++;
				if (PSP_Settings.iAPUTimerCycleDiv > PSP_CYCLE_DIV_MAX) {
					PSP_Settings.iAPUTimerCycleDiv = PSP_CYCLE_DIV_MAX;
				}
			} else if (sel == SWAP_ANALOG){
				PSP_Settings.bSwapAnalog = !PSP_Settings.bSwapAnalog;
			} else if (sel == PSP_CLOCKUP){
				PSP_Settings.iPSP_ClockUp++;
				if (PSP_Settings.iPSP_ClockUp > 2) {
					PSP_Settings.iPSP_ClockUp = 2;
				}
			}
		} else if ((now_pad & PSP_CTRL_SELECT) && (now_pad & PSP_CTRL_START)) {
			f_bExit = true;
			break;
		} else if (new_pad & PSP_CTRL_LTRIGGER){
			if (sel>LOAD_ROM) {
				sel=LOAD_ROM;
			} else if (sel>FRAME_SKIP){
				sel=FRAME_SKIP;
			} else if (sel>SRAM_SAVE){
				sel=SRAM_SAVE;
			}
		} else if (new_pad & PSP_CTRL_RTRIGGER){
			if (sel<FRAME_SKIP) {
				sel=FRAME_SKIP;
			} else if (sel<LOAD_ROM){
				sel=LOAD_ROM;
			} else if (sel<CONTINUE){
				sel=CONTINUE;
			}
		}
		
		menu_frame(msg, "›FOK  ~FContinue  SELECT+STARTFExit to PSP Menu");
		
		mh_print(33, 33, (unsigned char*)Memory.ROMFilename, RGB(95,95,125));
		
		x = 2;
		y = 6;
		
		pgPrint(x,y++,0xffff,"  SRAM Save");
		strcpy(tmp,"  Save Slot     : 0");
		tmp[strlen(tmp)-1] = PSP_Settings.iSaveSlot + '0';
		pgPrint(x,y++,0xffff,tmp);
		
		pgPrint(x,y++,0xffff,"  State Save");
		pgPrint(x,y++,0xffff,"  State Load");
		pgPrint(x,y++,0xffff,"  State Delete");
		if (PSP_Settings.bSaveThumb) {
			pgPrint(x,y++,0xffff,"  Save Thumbnail: ON");
		} else {
			pgPrint(x,y++,0xffff,"  Save Thumbnail: OFF");
		}
		y++;
		
		strcpy(tmp,"  FrameSkip     : 00");
		tmp[strlen(tmp)-2] = PSP_Settings.iSkipFrames / 10 + '0';
		tmp[strlen(tmp)-1] = PSP_Settings.iSkipFrames % 10 + '0';
		pgPrint(x,y++,0xffff,tmp);
		
		if (PSP_Settings.bAutoSkip) {
			pgPrint(x,y++,0xffff,"  Auto FrameSkip: ON");
		} else {
			pgPrint(x,y++,0xffff,"  Auto FrameSkip: OFF");
		}
		if (PSP_Settings.iScreenSize==0) {
			pgPrint(x,y++,0xffff,"  Screen Size   : Normal");
		} else if (PSP_Settings.iScreenSize==1){
			pgPrint(x,y++,0xffff,"  Screen Size   : Fit");
		} else if (PSP_Settings.iScreenSize==2){
			pgPrint(x,y++,0xffff,"  Screen Size   : Width x2");
		} else if (PSP_Settings.iScreenSize==3){
			pgPrint(x,y++,0xffff,"  Screen Size   : Full");
		}
		if (PSP_Settings.bShowFPS) {
			pgPrint(x,y++,0xffff,"  Show FPS      : ON");
		} else {
			pgPrint(x,y++,0xffff,"  Show FPS      : OFF");
		}
		if (PSP_Settings.bVSync) {
			pgPrint(x,y++,0xffff,"  VSync         : ON");
		} else {
			pgPrint(x,y++,0xffff,"  VSync         : OFF");
		}
		if (PSP_Settings.bSoundOff) {
			pgPrint(x,y++,0xffff,"  Sound         : OFF");
		} else {
			pgPrint(x,y++,0xffff,"  Sound         : ON");
		}
		if (PSP_Settings.iSoundRate==2) {
			pgPrint(x,y++,0xffff,"  Sound Rate    : 44kHz");
		} else if (PSP_Settings.iSoundRate==1){
			pgPrint(x,y++,0xffff,"  Sound Rate    : 22kHz");
		} else {
			pgPrint(x,y++,0xffff,"  Sound Rate    : 11kHz");
		}
		if (PSP_Settings.bTrans) {
			pgPrint(x,y++,0xffff,"  Transparency  : ON");
		} else {
			pgPrint(x,y++,0xffff,"  Transparency  : OFF");
		}
		strcpy(tmp,"x0.0");
		tmp[strlen(tmp)-3] = PSP_Settings.iHBlankCycleDiv / 10 + '0';
		tmp[strlen(tmp)-1] = PSP_Settings.iHBlankCycleDiv % 10 + '0';
		pgPrint(x,y,0xffff,"  Graphic Speed : ");
		tmp_color = 255-((PSP_Settings.iHBlankCycleDiv - 10) * 10);
		pgPrint(x+18,y++,RGB(255,tmp_color,tmp_color),tmp);
		strcpy(tmp,"x0.0");
		tmp[strlen(tmp)-3] = PSP_Settings.iAPUTimerCycleDiv / 10 + '0';
		tmp[strlen(tmp)-1] = PSP_Settings.iAPUTimerCycleDiv % 10 + '0';
		pgPrint(x,y,0xffff,"  Sound Speed   : ");
		tmp_color = 255-((PSP_Settings.iAPUTimerCycleDiv - 10) * 10);
		pgPrint(x+18,y++,RGB(255,tmp_color,tmp_color),tmp);
		if (PSP_Settings.bSwapAnalog) {
			pgPrint(x,y++,0xffff,"  Swap A<->D pad: ON");
		} else {
			pgPrint(x,y++,0xffff,"  Swap A<->D pad: OFF");
		}
		pgPrint(x,y,0xffff,"  PSP Clock     : ");
		if (PSP_Settings.iPSP_ClockUp == 2) pgPrint(x+18,y++,RGB(255,95,95),"333MHz");
		else if (PSP_Settings.iPSP_ClockUp == 1) pgPrint(x+18,y++,RGB(255,255,95),"266MHz");
		else pgPrint(x+18,y++,0xffff,"222MHz");
		y++;
		
		pgPrint(x,y++,0xffff,"  ROM Selector");
		pgPrint(x,y++,0xffff,"  Reset");
		pgPrint(x,y++,0xffff,"  Exit to PSP Menu");
		pgPrint(x,y++,0xffff,"  Continue");
		
		y = sel + 6;
		if (sel >= FRAME_SKIP){
			y++;
		}
		if (sel >= LOAD_ROM){
			y++;
		}
		
		pgPutChar((x+1)*8,y*8,0xffff,0,127,1,0,1);
		
		if (slotflag[PSP_Settings.iSaveSlot] && thumbflag[PSP_Settings.iSaveSlot]) {
			pgDrawFrame(327,129,456,242,RGB(85,85,95));
			pgBitBlt(328,130,128,112,1,slot_thumb[PSP_Settings.iSaveSlot]);
		}
		pgPrint(28,6,RGB(105,105,115),"State Save List");
		for(y=0; y<=4; y++){
			if (y==PSP_Settings.iSaveSlot) {
				pgPrint(28,y+7,0xffff,slotdate[y]);
			} else {
				pgPrint(28,y+7,RGB(105,105,115),slotdate[y]);
			}
		}
		do readpad(); while(!new_pad);
	}
	
	if (f_bExit) {
		pgFillBox( 129, 104, 351, 168, 0 );
		mh_print(210, 132, (unsigned char*)"Good bye ...", 0xffff);
		Settings.Paused = TRUE;
		save_config();
		Memory.SaveSRAM( S9xGetFilename("srm") );
		if ( g_thread !=-1 ){
			Settings.ThreadSound = FALSE;
			sceKernelWaitThreadEnd( g_thread, NULL );
			sceKernelDeleteThread( g_thread );
		}
		S9xCloseSoundDevice();
		g_bLoop = false;
		// Exit game
		sceKernelExitGame();
	}
	
	if ((PSP_Settings.iSoundRate != f_iOldSoundRate) || ( PSP_Settings.bSoundOff != f_bOldSoundOff)) {
		if (!f_bOldSoundOff) {
			S9xSetSoundMute( TRUE );
			if ( g_thread !=-1 ){
				Settings.ThreadSound = FALSE;
				sceKernelWaitThreadEnd( g_thread, NULL );
				sceKernelDeleteThread( g_thread );
				g_thread = -1;
			}
		}
		if (PSP_Settings.iSoundRate==0) S9xSetPlaybackRate( 11025 );
		else if (PSP_Settings.iSoundRate==1) S9xSetPlaybackRate( 22050 );
		else S9xSetPlaybackRate( 44100 );
		if (!PSP_Settings.bSoundOff) {
			Settings.ThreadSound = TRUE;
			InitTimer();
		}
	}
	
	Settings.H_Max = (long)((SNES_CYCLES_PER_SCANLINE * 1000) / ( PSP_Settings.iHBlankCycleDiv * 100));
	Settings.HBlankStart = ((uint32)Settings.H_Max << 8) / SNES_HCOUNTER_MAX;
	fix_H_Max();
	Settings.PSP_APUTimerCycle = (uint32)((SNES_APUTIMER2_CYCLEx10000 * 1000) / (PSP_Settings.iAPUTimerCycleDiv * 100));
	
	if (PSP_Settings.bTrans){
		Settings.Transparency = TRUE;
	} else {
		Settings.Transparency = FALSE;
	}
	Settings.SkipFrames = PSP_Settings.iSkipFrames;
#ifdef RELEASE
	GFX.Screen = (uint8*)GFX_Screen;
#endif // RELEASE
	if (PSP_Settings.iPSP_ClockUp == 2) scePowerSetClockFrequency(333,333,166);
	else if (PSP_Settings.iPSP_ClockUp == 1) scePowerSetClockFrequency(266,266,133);
	else scePowerSetClockFrequency(222,222,111);
	g_bSleep = false;
}

void S9xProcessEvents( bool8 block )
{
	if ( s_iAnalog == ANALOG_NONE ) {
	return;
	}
	
	if ( s_iAnalog == ANALOG_LEFT ){
		S9xSetSoundMute( TRUE );
	
		open_menu();
	
		pgFillvram(0);
		pgWaitVn(8);
		if (!PSP_Settings.bSoundOff) { S9xSetSoundMute( FALSE );}
	} else if ( s_iAnalog == ANALOG_RIGHT ){
		PSP_Settings.bSoundOff = !PSP_Settings.bSoundOff;
		if (PSP_Settings.bSoundOff) {
			S9xSetSoundMute( TRUE );
			if ( g_thread !=-1 ){
				Settings.ThreadSound = FALSE;
				sceKernelWaitThreadEnd( g_thread, NULL );
				sceKernelDeleteThread( g_thread );
			}
			S9xSetInfoString( "Sound: OFF" );
		} else {
			Settings.ThreadSound = TRUE;
			InitTimer();
			S9xSetSoundMute( FALSE );
			S9xSetInfoString( "Sound: ON" );
		}
		pgWaitVn(16);
	} else if ( s_iAnalog == ANALOG_UP ){
		if ( PSP_Settings.iSkipFrames < 10 ){
			PSP_Settings.iSkipFrames++;
			pgWaitVn(8);
		}
	} else if ( s_iAnalog == ANALOG_DOWN ){
		if ( PSP_Settings.iSkipFrames > 0 ){
			PSP_Settings.iSkipFrames--;
			pgWaitVn(8);
		}
	}

	if ( Settings.SkipFrames != PSP_Settings.iSkipFrames ){
		Settings.SkipFrames = PSP_Settings.iSkipFrames;
		strcpy( String, "FrameSkip:" );
		if ( Settings.SkipFrames < 10 ){
			String[10] = (Settings.SkipFrames % 10) + '0';
			String[11] = 0;
		} else {
			String[10] = (Settings.SkipFrames / 10) + '0';
			String[11] = (Settings.SkipFrames % 10) + '0';
			String[12] = 0;
		}
		S9xSetInfoString( String );
	}
	s_iAnalog = ANALOG_NONE;
}

int main(int argc, char **argv)
{
//	debug_log( argv );

	pspDebugInstallErrorHandler(NULL);

// mod by y
	char *p, savedir[_MAX_PATH];
	pgMain();
	strcpy(PBPPath, argv[0]);
	p = strrchr(PBPPath, '/');
	*++p = 0;

	SetupCallbacks();

	pgScreenFrame(2,0);
	sceCtrlSetSamplingCycle(0);

	strcpy(savedir,PBPPath);
	strcat(savedir,"SAVE");
	sceIoMkdir(savedir,0777);

/*
#ifdef RELEASE
	sceCtrlSetAnalogMode( 1 );
#else
	sceCtrlSetAnalogMode( 0 );
#endif // RELEASE
*/
	so.sound_fd = -1;

//	debug_log( "Snse9x for PSP Ver.0.02" );

	memset( &Settings, 0, sizeof( Settings ) );

#ifndef OPTI
	Settings.JoystickEnabled = FALSE;
#endif // OPTI
	Settings.SoundPlaybackRate = 4;
#ifndef OPTI
	Settings.Stereo = TRUE;
#endif // OPTI
	Settings.SoundBufferSize = 0;
	Settings.CyclesPercentage = 100;
	Settings.DisableSoundEcho = FALSE;
	Settings.APUEnabled = Settings.NextAPUEnabled = FALSE;
	Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
	Settings.SkipFrames = AUTO_FRAMERATE;
	Settings.ShutdownMaster = TRUE;
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.FrameTime = Settings.FrameTimeNTSC;
#ifndef OPTI
	Settings.StretchScreenshots = 1;
#endif // OPTI
	Settings.DisableMasterVolume = FALSE;
#ifndef OPTI
	Settings.Mouse = TRUE;
	Settings.SuperScope = TRUE;
	Settings.MultiPlayer5 = TRUE;
#endif // OPTI
	Settings.ControllerOption = SNES_JOYPAD;
	Settings.Transparency = FALSE;
#ifndef OPTI
	Settings.SixteenBit = FALSE;
	Settings.NetPlay = FALSE;
	Settings.ServerName [0] = 0;
	Settings.AutoSaveDelay = 30;
	Settings.ApplyCheats = TRUE;
#endif // OPTI
	Settings.SupportHiRes = FALSE;
	Settings.ThreadSound = FALSE;
	Settings.TurboMode = FALSE;
	Settings.TurboSkipFrames = 40;
#ifndef OPTI
	Settings.StretchScreenshots = 1;
#endif // OPTI
//Settings.DisplayFrameRate = TRUE;
Settings.SkipFrames = 0;
//Settings.SkipFrames = AUTO_FRAMERATE;
//Settings.AutoMaxSkipFrames = 10;
//Settings.SoundBufferSize = 44100 * 2 * 2 / 1000;
Settings.SoundBufferSize = SOUND_SAMPLE;
Settings.SoundSync = FALSE;
Settings.SoundEnvelopeHeightReading = TRUE;
Settings.ThreadSound = TRUE;
Settings.APUEnabled = Settings.NextAPUEnabled = TRUE;
#ifdef PSP
Settings.PSP_APUTimerCycle = SNES_APUTIMER2_CYCLEx10000;
#endif // PSP

#ifndef OPTI
	Settings.Transparency = Settings.ForceTransparency;
	if ( Settings.ForceNoTransparency ){
		Settings.Transparency = FALSE;
	}

	if ( Settings.Transparency ){
//		Settings.SixteenBit = TRUE;
	}
#endif // OPTI

	Settings.HBlankStart = (Settings.H_Max << 8) / SNES_HCOUNTER_MAX;

	if ( !Memory.Init() || !S9xInitAPU() ){
		return -1;
	}

#ifdef OPTI
	S9xInitSound( Settings.SoundPlaybackRate, 1, Settings.SoundBufferSize );
#else
	S9xInitSound( Settings.SoundPlaybackRate, Settings.Stereo, Settings.SoundBufferSize );
#endif // OPTI

	if ( !Settings.APUEnabled ){
		S9xSetSoundMute( TRUE );
	}

	uint32 saved_flags = CPU.Flags;

//	S9xSetRenderPixelFormat( RGB565 );

#if 1
// mod by y
	strcpy(LastPath,PBPPath);
	FilerMsg[0]=0;
	for(;;){
		while(!getFilePath(RomPath))
			;
		if ( !Memory.LoadROM(RomPath) ){
			continue;
		}
		S9xSetInfoString( "ROM image Loaded." );
		break;
	}
	pgFillvram(0);
	Memory.LoadSRAM( S9xGetFilename( "srm" ) );
	g_bROMLoaded = true;
	g_bSleep = false;
#else
	if ( rom_filename ){
		if ( !Memory.LoadROM (rom_filename ) ){
			char dir [_MAX_DIR + 1];
			char drive [_MAX_DRIVE + 1];
			char name [_MAX_FNAME + 1];
			char ext [_MAX_EXT + 1];
			char fname [_MAX_PATH + 1];

			_splitpath (rom_filename, drive, dir, name, ext);
			_makepath (fname, drive, dir, name, ext);

			strcpy (fname, S9xGetROMDirectory ());
			strcat (fname, SLASH_STR);
			strcat (fname, name);
			if ( ext[0] ){
				strcat (fname, ".");
				strcat (fname, ext);
			}
			_splitpath (fname, drive, dir, name, ext);
			_makepath (fname, drive, dir, name, ext);
			if ( !Memory.LoadROM (fname ) ){
				fprintf (stderr, "Error opening: %s\n", rom_filename);
				exit (1);
			}
		}
		Memory.LoadSRAM (S9xGetFilename (".srm"));
		S9xLoadCheatFile (S9xGetFilename (".cht"));
	} else {
		S9xReset();
		Settings.Paused |= 2;
	}
#endif
	CPU.Flags = saved_flags;

	S9xInitInputDevices();

	S9xInitDisplay( 0, 0 );
	if ( !S9xGraphicsInit() ){
		return -1;
	}

//	S9xTextMode();

/*
	if (snapshot_filename)
	{
	int Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
	if (!S9xLoadSnapshot (snapshot_filename))
		exit (1);
	CPU.Flags |= Flags;
	}
*/
//	S9xGraphicsMode();

//	sprintf (String, "\"%s\" %s: %s", Memory.ROMName, TITLE, VERSION);
//	S9xSetTitle( String );

	InitTimer();
	if ( !Settings.APUEnabled ){
		S9xSetSoundMute( FALSE );
	}

	s_iFrame = 0;
	gettimeofday( &s_tvStart, 0 );

	PSP_Settings.bSoundOff = false;
	load_config();

	S9xSetSoundMute( TRUE );

	while ( g_bLoop ){
#if 1
		if ( !Settings.Paused ){
			S9xMainLoop();
		}
#else
		if ( !Settings.Paused ){
			S9xMainLoop();
		}

		if ( Settings.Paused ){
			S9xSetSoundMute( TRUE );
		}

		if ( Settings.Paused ){
			S9xProcessEvents( FALSE );
//			usleep(100000);
		}

		S9xProcessEvents( FALSE );
	
		if ( !Settings.Paused ){
			S9xSetSoundMute( FALSE );
		}
#endif
// add by y
		if (g_bSleep){
			pgWaitVn(180);
			if (PSP_Settings.iPSP_ClockUp == 2) scePowerSetClockFrequency(333,333,166);
			else if (PSP_Settings.iPSP_ClockUp == 1) scePowerSetClockFrequency(266,266,133);
			if (!PSP_Settings.bSoundOff) { S9xSetSoundMute( FALSE );}
			g_bSleep = false;
		}
	}

	return 0;
}


};


#define SLASH_STR "/"
#define SLASH_CHAR '/'

void _makepath( char *path, const char *drive, const char *dir, const char *fname, const char *ext )
{
	if ( drive && *drive ){
		*path       = *drive;
		*(path + 1) = ':';
		*(path + 2) = 0;
	} else {
		*path = 0;
	}
	
	if ( dir && *dir ){
		strcat( path, dir );
		if ( strlen( dir ) != 1 || *dir != '\\' ){
			strcat( path, SLASH_STR );
		}
	}
	
	if ( fname ){
		strcat( path, fname );
	}
	if ( ext && *ext ){
		strcat( path, "." );
		strcat( path, ext );
	}
}

void _splitpath( const char *path, char *drive, char *dir, char *fname, char *ext )
{
	if ( *path && *(path + 1) == ':' ){
		*drive = toupper( *path );
		path += 2;
	} else {
		*drive = 0;
	}

	char*	slash = strrchr( path, SLASH_CHAR );
	if ( !slash ){
		slash = strrchr( path, '/' );
	}
	char*	dot = strrchr( path, '.' );
	if ( dot && slash && dot < slash ){
		dot = NULL;
	}

	if ( !slash ){
		if ( *drive ){
			strcpy( dir, "\\" );
		} else {
			strcpy( dir, "" );
		}
		strcpy( fname, path );
		if ( dot ){
			*(fname + (dot - path)) = 0;
			strcpy( ext, dot + 1 );
		} else {
			strcpy( ext, "" );
		}
	} else {
		if ( *drive && *path != '\\' ){
			strcpy( dir, "\\" );
			strcat( dir, path );
			*(dir + (slash - path) + 1) = 0;
		} else {
			strcpy( dir, path );
			if ( (slash - path) == 0 ){
				*(dir + 1) = 0;
			} else {
				*(dir + (slash - path)) = 0;
			}
		}

		strcpy( fname, slash + 1 );
		if ( dot ){
			*(fname + (dot - slash) - 1) = 0;
			strcpy( ext, dot + 1 );
		} else {
			strcpy( ext, "" );
		}
	}
}

extern "C" {

void strrev(char *s)
{
	char tmp;
	int i;
	int len = strlen(s);
	
	for(i=0; i<len/2; i++){
		tmp = s[i];
		s[i] = s[len-1-i];
		s[len-1-i] = tmp;
	}
}

void itoa(int val, char *s) {
	char *t;
	int mod;

	if(val < 0) {
		*s++ = '-';
		val = -val;
	}
	t = s;

	while(val) {
		mod = val % 10;
		*t++ = (char)mod + '0';
		val /= 10;
	}

	if(s == t)
		*t++ = '0';

	*t = '\0';

	strrev(s);
}

void ustoa(unsigned short val, char *s)
{
	char *t;
	unsigned short mod;
	
	t = s;
	
	while(val) {
		mod = val % 10;
		*t++ = (char)mod + '0';
		val /= 10;
	}

	if(s == t)
		*t++ = '0';

	*t = '\0';

	strrev(s);
}
}
/*
WIP
E‚¿‚å‚Á‚Ò‚è‘¬‚­‚È‚Á‚½
EƒTƒEƒ“ƒh‚ªãY—í‚É‚È‚Á‚½
E‰æ–Ê‚ÌƒZƒ“ƒ^ƒŠƒ“ƒO
EƒtƒŒ[ƒ€ƒXƒLƒbƒv(ŽžŠÔ“¯Šú‚·‚é‚Æ’x‚¢‚Ì‚Å–{“–‚ÌƒXƒLƒbƒv‚Ì‚Ý)
ESRAMƒZ[ƒu‚ð—LŒø
EƒtƒŒ[ƒ€ƒXƒLƒbƒvŽž‚Éƒnƒ“ƒOƒAƒbƒv‚·‚é•s‹ï‡‚ÌC³
E‰æ–Ê‹÷‚ªŒ‡‚¯‚é•s‹ï‡‚ÌC³

http://www.geocities.jp/pasofami77/chip/sfcchip
*/
