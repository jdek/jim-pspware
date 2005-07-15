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
#include <psppower.h>
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

extern "C" {
#include "rle_codec.c"

#define RLE_MAGIC     "RLE!"
#define RLE_MAGIC_LEN strlen (RLE_MAGIC)

// Reserve 360 Kb for freeze buffers, increase if needed...
#define MAX_FREEZE_SIZE 368640 
//#define MAX_FREEZE_SIZE 409600 /* 400 Kb */

bool8 bGUIMode = FALSE;
};


#define gettimeofday sceKernelLibcGettimeofday


static struct timeval	s_tvStart;
static int				s_iFrame;
static int				s_iFramebufferState = 0; // 0 = clean, 1 = front/back buffer dirty, 2 = front & back dirty
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
void refresh_state_list (void);

void clear_framebuffer    (void);
void S9xMarkScreenDirty   (void);
void S9xMarkScreenDirtyEx (void);

extern "C" void init_blit_backend (void);
extern "C" void init_pg (void);

extern "C" void draw_menu (void);

PSPSETTINGS PSP_Settings;

#define UPPER_THRESHOLD		0xcf
#define LOWER_THRESHOLD		0x2f
#define PSP_CYCLE_DIV_MAX	30
volatile bool8			g_bSleep = false;
volatile bool8			g_bROMLoaded = false;

//
volatile bool8			g_bLoop = true;
int						g_thread = -1;
int				g_iMainMenuSel = 0;
char				g_szMainMenuMsg[256];
static uint8			SoundBuffer[MAX_BUFFER_SIZE];

//#define FRAMESIZE				0x44000			//in byte

#define MAX_SOUND_SAMPLE		128
#define SOUND_SAMPLE			1024
//#define SOUND_SAMPLE			2048

#define FIXED_POINT				0x10000
#define FIXED_POINT_SHIFT		16
#define FIXED_POINT_REMAINDER	0xffff

#define timercmp(a, b, CMP)	(((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))
//
// Allow debug info to be toggled on/off in release builds
//
#ifdef RELEASE
#define DEBUG_TEST PSP_Settings.bShowDebugInfo
#else
#define DEBUG_TEST true
#endif

#define BEGIN_DEBUG_CODE \
    if (DEBUG_TEST) {

#define END_DEBUG_CODE \
    }

#define BEGIN_RELEASE_CODE \
    if (! DEBUG_TEST) {

#define END_RELEASE_CODE \
    }

#define ELSE_DEBUG_CODE   END_RELEASE_CODE else BEGIN_DEBUG_CODE
#define ELSE_RELEASE_CODE END_DEBUG_CODE   else BEGIN_RELEASE_CODE

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
	BEGIN_DEBUG_CODE
	static int	sy = 1;
	
	if ( sy >= CMAX_Y ) {
		pgFillBoxWF(256, 0, 479, 271, 0);
		sy = 1;
	}
	pgPrintWF( SNES_WIDTH / 8, sy, 0xffff, message );
	sy++;

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
	
	END_DEBUG_CODE
}

void debug_int( const char* message, int value )
{
	BEGIN_DEBUG_CODE
	
	strcpy( String, message );
	format_int( &String[strlen( String )], value );

	debug_log( String );

	END_DEBUG_CODE
}

void debug_hex( int value )
{
	BEGIN_DEBUG_CODE
	
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

	END_DEBUG_CODE
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

void S9xLoadSDD1Data (void)
{
    Settings.SDD1Pack = FALSE;

    Memory.FreeSDD1Data ();

    if (strncmp (Memory.ROMName, "Star Ocean",            10) == 0 ||
        strncmp (Memory.ROMName, "STREET FIGHTER ALPHA2", 21) == 0)
    {
        Settings.SDD1Pack = TRUE;
    }
}

// add by J
//      ダイアログのタイトル内側カラー
//#define D_dialog_in_color1  RGB(  50,  50, 180 )
int PSP_KEY_CONFIG_X = 0x1000; // △
int PSP_KEY_CONFIG_Y = 0x8000; // □
int PSP_KEY_CONFIG_A = 0x2000; // ○
int PSP_KEY_CONFIG_B = 0x4000; // ×
int PSP_KEY_CONFIG_L = 0x0100; // Ｌ
int PSP_KEY_CONFIG_R = 0x0200; // Ｒ
int PSP_KEY_CONFIG[6];
void key_config_init()
{
	PSP_KEY_CONFIG[0] = 0x1000; // △
	PSP_KEY_CONFIG[1] = 0x8000; // □
	PSP_KEY_CONFIG[2] = 0x2000; // ○
	PSP_KEY_CONFIG[3] = 0x4000; // ×
	PSP_KEY_CONFIG[4] = 0x0100; // Ｌ
	PSP_KEY_CONFIG[5] = 0x0200; // Ｒ
	return; // voidだからいらないんだけどね。
}
void key_config_disp( int cur_pos )
{
	int loop_cnt;
	char dialog_text_all[ D_text_all_MAX ];
	char key_con_text[6][64];
	char key[6][4];

	for( loop_cnt = 0; loop_cnt < 6; loop_cnt++ ) {
		switch( PSP_KEY_CONFIG[ loop_cnt ] ) {
			case 0x1000: // △
				strcpy( key[ loop_cnt ] , "△\0" );
			break;
			case 0x2000: // ○
				strcpy( key[ loop_cnt ] , "○\0" );
			break;
			case 0x4000: // ×
				strcpy( key[ loop_cnt ] , "×\0" );
			break;
			case 0x8000: // □
				strcpy( key[ loop_cnt ] , "□\0" );
			break;
			case 0x0100: // Ｌ
				strcpy( key[ loop_cnt ] , "Ｌ\0" );
			break;
			case 0x0200: // Ｒ
				strcpy( key[ loop_cnt ] , "Ｒ\0" );
			break;
		}
	}
	strcpy( dialog_text_all, "  SELECTボタンで終了します。\n\0" );
	strcat( dialog_text_all, "\n\0"               ); // 改行
	strcpy( key_con_text[0], "       ● X --- \0" );
	strcat( key_con_text[0], key[0] );
	strcat( key_con_text[0], "\n\0" );
	strcpy( key_con_text[1], "   Y ● ------- \0" );
	strcat( key_con_text[1], key[1] );
	strcat( key_con_text[1], "\n\0" );
	strcpy( key_con_text[2], "         ● A - \0" );
	strcat( key_con_text[2], key[2] );
	strcat( key_con_text[2], "\n\0" );
	strcpy( key_con_text[3], "     B ● ----- \0" );
	strcat( key_con_text[3], key[3] );
	strcat( key_con_text[3], "\n\0" );
	strcpy( key_con_text[4], "   Ｌボタン --- \0" );
	strcat( key_con_text[4], key[4] );
	strcat( key_con_text[4], "\n\0" );
	strcpy( key_con_text[5], "   Ｒボタン --- \0" );
	strcat( key_con_text[5], key[5] );
	strcat( key_con_text[5], "\n\0" );
	key_con_text[cur_pos][1] = '>';
	for ( loop_cnt = 0; loop_cnt < 6; loop_cnt++ ) {
		strcat( dialog_text_all,key_con_text[loop_cnt] );
	}
	strcat( dialog_text_all, "\n\0"               ); // 改行
	// ダイアログ表示
	message_dialog( 34, 75, " 　【　　Key Config　　】　 \0", dialog_text_all );

	return; // voidだからいらないんだけどね。
}
void key_config()
{
	// ダイアログ用メッセージ文字列
	int sel = 0;
	// 無限ループ気持ち悪い
	while( 1 ){
		readpad();
		// SELECTボタン押した
		if ( new_pad & PSP_CTRL_SELECT ) {
			break;
		}else
		if ( new_pad & PSP_CTRL_DOWN ) {
			sel++;
			if ( sel > 5 ) sel = 0;
		}else
		if ( new_pad & PSP_CTRL_UP ) {
			sel--;
			if ( sel < 0 ) sel = 5;
		}else
		if ( new_pad & PSP_CTRL_SQUARE ) {
			PSP_KEY_CONFIG[sel] = PSP_CTRL_SQUARE;
		}else
		if ( new_pad & PSP_CTRL_TRIANGLE ) {
			PSP_KEY_CONFIG[sel] = PSP_CTRL_TRIANGLE;
		}else
		if ( new_pad & PSP_CTRL_CIRCLE ) {
			PSP_KEY_CONFIG[sel] = PSP_CTRL_CIRCLE;
		}else
		if ( new_pad & PSP_CTRL_CROSS ) {
			PSP_KEY_CONFIG[sel] = PSP_CTRL_CROSS;
		}else
		if ( new_pad & PSP_CTRL_LTRIGGER ) {
			PSP_KEY_CONFIG[sel] = PSP_CTRL_LTRIGGER;
		}else
		if ( new_pad & PSP_CTRL_RTRIGGER ) {
			PSP_KEY_CONFIG[sel] = PSP_CTRL_RTRIGGER;
		}
		
		// Update any text on the main menu...
		draw_menu ();
		
		key_config_disp( sel );
	}
	return; // voidだからいらないんだけどね。
}

#define AddConfigValue(value_string) {                                  \
  strcat( dialog_text_all, value_string );                              \
  strcat( dialog_text_all, "\n"         );                              \
}

#define AddConfigOption(idx,name) {                                     \
  if (cur_pos == idx)                                                   \
    strcat( dialog_text_all, selected_line );                           \
  else                                                                  \
    strcat( dialog_text_all, unselected_line );                         \
                                                                        \
  strcat( dialog_text_all, name );                                      \
  strcat( dialog_text_all, ":  ");                                      \
}

#define AddBooleanConfigOption(idx,name,true_string,false_string,val) { \
  AddConfigOption (idx, name);                                          \
                                                                        \
  if (val)                                                              \
    AddConfigValue (true_string)                                        \
  else                                                                  \
    AddConfigValue (false_string)                                       \
}

void state_config_disp (int cur_pos)
{
	char dialog_text_all[ D_text_all_MAX ];

	*dialog_text_all = '\0';

	const char* selected_line   = "  > ";
	const char* unselected_line = "    ";


	AddBooleanConfigOption (0, "Save Thumbnails ",
	                           "ON","OFF",
	                               (PSP_Settings.bSaveThumb));

	
	AddConfigOption        (1, "File Compression");
	
	if (PSP_Settings.iCompression)
		AddConfigValue ("Runlength Encoding (RLE)")
	else
		AddConfigValue ("Uncompressed")

	strcat( dialog_text_all, "\n");

	strcat( dialog_text_all, "  NOTE: Compression will make state saves incompatible\n"
	                         "        with older versions, but will make state saving\n"
				 "        faster and use less Memory Stick space.\n\0" );
	strcat( dialog_text_all, "\n\0"               );

	message_dialog( 34, 75, " 　【　　State Save Config　　】　 \0", dialog_text_all );
}

void state_config (void)
{
	static int sel = 0; // Remember the selection when the dialog's closed...
	
	while (1) {
		readpad ();

		if (new_pad & PSP_CTRL_CROSS) {
			break;
		}
		
		else if (new_pad & PSP_CTRL_DOWN) {
			sel++;
			if (sel > 1)
				sel = 0;
		}
		
		else if (new_pad & PSP_CTRL_UP) {
			sel--;
			if (sel < 0)
				sel = 1;
		}
		
		else if (new_pad & PSP_CTRL_CIRCLE) {
			switch (sel)
			{
				case 0:
					PSP_Settings.bSaveThumb = !PSP_Settings.bSaveThumb;
					break;
				case 1:
					PSP_Settings.iCompression++;
					if (PSP_Settings.iCompression > 1)
						PSP_Settings.iCompression = 0;
					break;
			}
		}

		else if (new_pad & PSP_CTRL_RIGHT) {
			switch (sel)
			{
				case 0:
					PSP_Settings.bSaveThumb = FALSE;
					break;
				case 1:
					PSP_Settings.iCompression++;
					if (PSP_Settings.iCompression > 1)
						PSP_Settings.iCompression = 1;
					break;
			}
		}

		else if (new_pad & PSP_CTRL_LEFT) {
			switch (sel)
			{
				case 0:
					PSP_Settings.bSaveThumb = TRUE;
					break;
				case 1:
					PSP_Settings.iCompression--;
					if (PSP_Settings.iCompression < 0)
						PSP_Settings.iCompression = 0;
					break;
			}
		}

		else {
		}

		// Update any text on the main menu...
		draw_menu ();

		state_config_disp (sel);
	}

	return;
}

namespace DisplayOptions
{
	enum {
		SCR_SIZE,
		BG_COLOR,
		SHOWFPS,
		VSYNC,
		TRANS,
		DEBUG,
		BLIT_BACKEND,
		HIRES,
		BILINEAR,

		NUM_DISPLAY_OPTIONS,
	};
};

enum {
	SCR_SIZE_X1,
	SCR_SIZE_FIT,
	SCR_SIZE_FULL,
	SCR_SIZE_FULLFIT,

	NUM_SCREEN_SIZES,
};


void display_config_disp (int cur_pos)
{
	using namespace DisplayOptions;
	
	char dialog_text_all[ D_text_all_MAX ];

	*dialog_text_all = '\0';

	const char* selected_line   = "  > ";
	const char* unselected_line = "    ";


        AddConfigOption        (SCR_SIZE,         "Screen Size      ")

	if (PSP_Settings.iScreenSize == SCR_SIZE_X1)
		AddConfigValue ("Normal")
	else if (PSP_Settings.iScreenSize == SCR_SIZE_FIT)
		if (PSP_Settings.bUseGUBlit)
			AddConfigValue ("4:3")
		else
			AddConfigValue ("Fit") // Too lazy to write pg 4:3 blit :)
	else if (PSP_Settings.iScreenSize == SCR_SIZE_FULL)
		AddConfigValue ("Width x2")
	else if (PSP_Settings.iScreenSize == SCR_SIZE_FULLFIT)
		AddConfigValue ("Full")
	else
		AddConfigValue ("Unknown")
		
	AddConfigOption        (BG_COLOR,         "Screen Color     ")

	if (PSP_Settings.iBackgroundColor == 0)
		AddConfigValue ("Black")
	else
		AddConfigValue ("White")
	
	AddBooleanConfigOption (SHOWFPS,          "Show Framerate   ",
	                            "ON","OFF",
	                                (PSP_Settings.bShowFPS))

	AddBooleanConfigOption (VSYNC,            "Vertical Sync.   ",
	                            "ON","OFF",
	                                (PSP_Settings.bVSync))

	AddBooleanConfigOption (TRANS,            "Transparency     ",
	                            "ON","OFF",
	                                (PSP_Settings.bTrans))

	AddBooleanConfigOption (DEBUG,            "Show Debug Info  ",
	                            "ON","OFF",
	                                (PSP_Settings.bShowDebugInfo))

	AddBooleanConfigOption (BLIT_BACKEND,     "Bit Blit Backend ",
	                            "sceGu (advanced)","pg (original)",
	                                (PSP_Settings.bUseGUBlit))

	if (PSP_Settings.bUseGUBlit) {
		AddConfigValue ("");
		AddConfigValue ("sceGu Blit Options");
		AddConfigValue ("");
		
		AddBooleanConfigOption (HIRES,            "HiRes Mode       ",
	                            "ON","OFF",
	                                (PSP_Settings.bSupportHiRes))

		AddBooleanConfigOption (BILINEAR,         "Filter Technique ",
	                            "Bilinear","Point",
	                                (PSP_Settings.bBilinearFilter))
	}

	// Add a blank line below the last option...
	AddConfigValue ("");


	message_dialog( 34, 50, " 　【　　Display Config　　】　 \0", dialog_text_all );
}

void display_config (void)
{
	using namespace DisplayOptions;

	static int sel = 0;

	bool f_bOldUseGUBlit = PSP_Settings.bUseGUBlit;

	while (1) {
		readpad ();

		if (new_pad & PSP_CTRL_CROSS) {
			break;
		}

		else if (new_pad & PSP_CTRL_DOWN) {
			sel++;
			if (sel > (NUM_DISPLAY_OPTIONS - 1))
				sel = 0;

			if (! PSP_Settings.bUseGUBlit) {
				// Options after BLIT_BACKEND are specific to GU blit only...
				if (sel > BLIT_BACKEND)
					sel = BLIT_BACKEND;
			}
		}
		
		else if (new_pad & PSP_CTRL_UP) {
			sel--;
			if (sel < 0)
				sel = (NUM_DISPLAY_OPTIONS - 1);
		}

		else if (new_pad & PSP_CTRL_CIRCLE) {
			switch (sel)
			{
				case SCR_SIZE:
					PSP_Settings.iScreenSize++;
					if (PSP_Settings.iScreenSize > (NUM_SCREEN_SIZES - 1)) {
						PSP_Settings.iScreenSize = 0;
					}
					break;

				case BG_COLOR:
					PSP_Settings.iBackgroundColor++;
					if (PSP_Settings.iBackgroundColor > 1) {
						PSP_Settings.iBackgroundColor = 0;
					}
					break;

				case SHOWFPS:
					PSP_Settings.bShowFPS = !PSP_Settings.bShowFPS;
					break;

				case VSYNC:
					PSP_Settings.bVSync = !PSP_Settings.bVSync;
					break;

				case TRANS:
					PSP_Settings.bTrans = !PSP_Settings.bTrans;
					break;

				case DEBUG:
					PSP_Settings.bShowDebugInfo = !PSP_Settings.bShowDebugInfo;
					break;

				case BLIT_BACKEND:
					PSP_Settings.bUseGUBlit = !PSP_Settings.bUseGUBlit;

					init_blit_backend ();
					break;

				case HIRES:
					PSP_Settings.bSupportHiRes = !PSP_Settings.bSupportHiRes;

					init_blit_backend ();
					break;

				case BILINEAR:
					PSP_Settings.bBilinearFilter = !PSP_Settings.bBilinearFilter;
					break;

				default:
					break;
			}
		}

		else if (new_pad & PSP_CTRL_LEFT) {
			switch (sel)
			{
				case SCR_SIZE:
					if (PSP_Settings.iScreenSize < 1) {
						PSP_Settings.iScreenSize = 1;
					}
					PSP_Settings.iScreenSize--;
					break;

				case BG_COLOR:
					if (PSP_Settings.iBackgroundColor < 1) {
						PSP_Settings.iBackgroundColor = 1;
					}
					PSP_Settings.iBackgroundColor--;
					break;

				case SHOWFPS:
					PSP_Settings.bShowFPS = TRUE;
					break;

				case VSYNC:
					PSP_Settings.bVSync = TRUE;
					break;

				case TRANS:
					PSP_Settings.bTrans = TRUE;
					break;

				case DEBUG:
					PSP_Settings.bShowDebugInfo = TRUE;
					break;

				case BLIT_BACKEND:
					PSP_Settings.bUseGUBlit = FALSE;

					init_blit_backend ();
					break;

				case HIRES:
					PSP_Settings.bSupportHiRes = TRUE;

					init_blit_backend ();
					break;

				case BILINEAR:
					PSP_Settings.bBilinearFilter = FALSE;
					break;

				default:
					break;
			}
		}
		
		else if (new_pad & PSP_CTRL_RIGHT) {
			switch (sel)
			{
				case SCR_SIZE:
					PSP_Settings.iScreenSize++;
					if (PSP_Settings.iScreenSize > (NUM_SCREEN_SIZES - 1)) {
						PSP_Settings.iScreenSize = (NUM_SCREEN_SIZES - 1);
					}
					break;

				case BG_COLOR:
					PSP_Settings.iBackgroundColor++;
					if (PSP_Settings.iBackgroundColor > 1) {
						PSP_Settings.iBackgroundColor = 1;
					}
					break;

				case SHOWFPS:
					PSP_Settings.bShowFPS = FALSE;
					break;

				case VSYNC:
					PSP_Settings.bVSync = FALSE;
					break;

				case TRANS:
					PSP_Settings.bTrans = FALSE;
					break;

				case DEBUG:
					PSP_Settings.bShowDebugInfo = FALSE;
					break;

				case BLIT_BACKEND:
					PSP_Settings.bUseGUBlit = TRUE;

					init_blit_backend ();
					break;

				case HIRES:
					PSP_Settings.bSupportHiRes = FALSE;

					init_blit_backend ();
					break;

				case BILINEAR:
					PSP_Settings.bBilinearFilter = TRUE;
					break;

				default:
					break;
			}
		}

		if (! PSP_Settings.bUseGUBlit) {
			// Options after BLIT_BACKEND are specific to GU blit only...
			if (sel > BLIT_BACKEND)
				sel = 0;
		}

		// Update any text on the main menu...
		draw_menu ();

		display_config_disp (sel);
	}
}

namespace SoundOptions
{
	enum {

		SOUND_OFF,
		SOUND_RATE,
		SOUND_SYNC,
		SOUND_ALT_SAMPLE_DECODE,

		NUM_SOUND_OPTIONS,
	};
};

void sound_config_disp (int cur_pos)
{
	using namespace SoundOptions;

	char dialog_text_all[ D_text_all_MAX ];

	*dialog_text_all = '\0';

	const char* selected_line   = "  > ";
	const char* unselected_line = "    ";


	AddBooleanConfigOption (SOUND_OFF,         "Sound         " ,
	                            "ON", "OFF",
	                                (! PSP_Settings.bSoundOff))

	AddConfigOption        (SOUND_RATE,        "Sound Rate    ")

	if (PSP_Settings.iSoundRate == 2)
		AddConfigValue ("44kHz")
	else if (PSP_Settings.iSoundRate == 1)
		AddConfigValue ("22kHz")
	else
		AddConfigValue ("11kHz")

	AddConfigOption (SOUND_SYNC,               "Sound Sync.   ")

	if (PSP_Settings.iSoundSync == 2)
		AddConfigValue ("2")
	else if (PSP_Settings.iSoundSync == 1)
		AddConfigValue ("1")
	else
		AddConfigValue ("No")

	AddConfigOption (SOUND_ALT_SAMPLE_DECODE,  "Sample Decode ")

	switch (PSP_Settings.iAltSampleDecode)
	{
		case 1:
		case 2:
		case 3:
			char szAltDecode [32];
			sprintf (szAltDecode, "Alternate %d", PSP_Settings.iAltSampleDecode);
			AddConfigValue (szAltDecode);
			break;
		case 0:
		default:
			AddConfigValue ("Default    ");
			break;
	}

	// Add a blank line below the last option...
	AddConfigValue ("");


	message_dialog( 34, 75, " 　【　　Sound Config　　】　 \0", dialog_text_all );
}

void sound_config (void)
{
	using namespace SoundOptions;

	static int sel = 0;

	bool f_bOldSoundOff        = PSP_Settings.bSoundOff;
	int  f_iOldSoundRate       = PSP_Settings.iSoundRate;
	int  f_iOldSoundSync       = PSP_Settings.iSoundSync;
	int  f_iOldAltSampleDecode = PSP_Settings.iAltSampleDecode;

	while (1) {
		readpad ();

		if (new_pad & PSP_CTRL_CROSS) {
			break;
		}

		else if (new_pad & PSP_CTRL_DOWN) {
			sel++;
			if (sel > (NUM_SOUND_OPTIONS - 1))
				sel = 0;
		}
		
		else if (new_pad & PSP_CTRL_UP) {
			sel--;
			if (sel < 0)
				sel = (NUM_SOUND_OPTIONS - 1);
		}

		else if (new_pad & PSP_CTRL_CIRCLE) {
			switch (sel)
			{
				case SOUND_OFF:
					PSP_Settings.bSoundOff = !PSP_Settings.bSoundOff;
					break;

				case SOUND_RATE:
					PSP_Settings.iSoundRate++;
					if (PSP_Settings.iSoundRate > 2) {
						PSP_Settings.iSoundRate = 0;
					}
					break;

				case SOUND_SYNC:
					PSP_Settings.iSoundSync++;
					if (PSP_Settings.iSoundSync > 2) {
						PSP_Settings.iSoundSync = 0;
					}
					break;

				case SOUND_ALT_SAMPLE_DECODE:
					PSP_Settings.iAltSampleDecode++;
					if (PSP_Settings.iAltSampleDecode > 3) {
						PSP_Settings.iAltSampleDecode = 0;
					}
					break;

				default:
					break;
			}
		}

		else if (new_pad & PSP_CTRL_LEFT) {
			switch (sel)
			{
				case SOUND_OFF:
					PSP_Settings.bSoundOff = FALSE;
					break;

				case SOUND_RATE:
					if (PSP_Settings.iSoundRate < 1) {
						PSP_Settings.iSoundRate = 1;
					}
					PSP_Settings.iSoundRate--;
					break;

				case SOUND_SYNC:
					if (PSP_Settings.iSoundSync < 1) {
						PSP_Settings.iSoundSync = 1;
					}
					PSP_Settings.iSoundSync--;
					break;

				case SOUND_ALT_SAMPLE_DECODE:
					if (PSP_Settings.iAltSampleDecode < 1) {
						PSP_Settings.iAltSampleDecode = 1;
					}
					PSP_Settings.iAltSampleDecode--;
					break;

				default:
					break;
			}
		}
		
		else if (new_pad & PSP_CTRL_RIGHT) {
			switch (sel)
			{
				case SOUND_OFF:
					PSP_Settings.bSoundOff = TRUE;
					break;

				case SOUND_RATE:
					PSP_Settings.iSoundRate++;
					if (PSP_Settings.iSoundRate > 2) {
						PSP_Settings.iSoundRate = 2;
					}
					break;

				case SOUND_SYNC:
					PSP_Settings.iSoundSync++;
					if (PSP_Settings.iSoundSync > 2) {
						PSP_Settings.iSoundSync = 2;
					}
					break;

				case SOUND_ALT_SAMPLE_DECODE:
					PSP_Settings.iAltSampleDecode++;
					if (PSP_Settings.iAltSampleDecode > 3) {
						PSP_Settings.iAltSampleDecode = 3;
					}
					break;

				default:
					break;
			}
		}

		// Update any text on the main menu...
		draw_menu ();

		sound_config_disp (sel);
	}

	if ((PSP_Settings.iSoundRate       != f_iOldSoundRate)       ||
	    (PSP_Settings.bSoundOff        != f_bOldSoundOff)        ||
	    (PSP_Settings.iSoundSync       != f_iOldSoundSync)) {
		if (! f_bOldSoundOff) {
			S9xSetSoundMute (TRUE);
			
			if (g_thread != -1) {
				Settings.ThreadSound = FALSE;
				sceKernelWaitThreadEnd (g_thread, NULL);
				sceKernelDeleteThread  (g_thread);
				
				g_thread = -1;
			}
		}

		if (PSP_Settings.iSoundRate == 0)
			S9xSetPlaybackRate (11025);
		else if (PSP_Settings.iSoundRate == 1)
			S9xSetPlaybackRate (22050);
		else
			S9xSetPlaybackRate (44100);

		Settings.SoundSync = PSP_Settings.iSoundSync;

		if (! PSP_Settings.bSoundOff) {
			Settings.ThreadSound = TRUE;
			void InitTimer (void);
			InitTimer ();
		}
	}

	if (f_iOldAltSampleDecode != PSP_Settings.iAltSampleDecode)
		Settings.AltSampleDecode = PSP_Settings.iAltSampleDecode;
}


uint32 S9xReadJoypad( int which1 )
{
	SceCtrlData	ctl;
	uint32		ret;

	if ( which1 ){
		return 0;
	}

// mod by J
	sceCtrlReadBufferPositive( &ctl, 1 );
	ret = ((ctl.Buttons & PSP_CTRL_UP)       << 7) |	// ↑
		  ((ctl.Buttons & PSP_CTRL_DOWN)     << 4) |	// ↓
		  ((ctl.Buttons & PSP_CTRL_LEFT)     << 2) |	// ←
		  ((ctl.Buttons & PSP_CTRL_RIGHT)    << 3) |	// →
		  ((ctl.Buttons & PSP_CTRL_START)    << 9) |	// ＳＴＡＲＴ
		  ((ctl.Buttons & PSP_CTRL_SELECT)   << 13);	// ＳＥＬＥＣＴ
// 最適化してくで
	// Xボタン
	if ( PSP_KEY_CONFIG[0] & ctl.Buttons ) ret += 0x0040;
	// Yボタン
	if ( PSP_KEY_CONFIG[1] & ctl.Buttons ) ret += 0x4000;
	// Aボタン
	if ( PSP_KEY_CONFIG[2] & ctl.Buttons ) ret += 0x0080;
	// Bボタン
	if ( PSP_KEY_CONFIG[3] & ctl.Buttons ) ret += 0x8000;
	// Lボタン
	if ( PSP_KEY_CONFIG[4] & ctl.Buttons ) ret += 0x0020;
	// Rボタン
	if ( PSP_KEY_CONFIG[5] & ctl.Buttons ) ret += 0x0010;

/* パズルみたい
		  ((ctl.Buttons & PSP_CTRL_CIRCLE)   >> 6) |	// ○ -> Ａ
		  ((ctl.Buttons & PSP_CTRL_CROSS)    << 1) |	// × -> Ｂ
		  ((ctl.Buttons & PSP_CTRL_SQUARE)   >> 1) |	// □ -> Ｙ
		  ((ctl.Buttons & PSP_CTRL_TRIANGLE) >> 6) |	// △ -> Ｘ
		  ((ctl.Buttons & PSP_CTRL_LTRIGGER) >> 3) |	// Ｌ
		  ((ctl.Buttons & PSP_CTRL_RTRIGGER) >> 5) |	// Ｒ
*/
/*
	    ● X		　△
	Y ●　● A		□　○
	  B ●			　×

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
// add by J
	if ( ( ctl.Buttons & PSP_CTRL_SELECT ) && ( ctl.Buttons & PSP_CTRL_LTRIGGER ) && ( ctl.Buttons & PSP_CTRL_RTRIGGER ) ) {
		s_iAnalog = ANALOG_LEFT;
	}
	if ( ( ret & 0x0f00 ) == 0 ) {
		ret &= ~(SNES_RIGHT_MASK | SNES_LEFT_MASK | SNES_DOWN_MASK | SNES_UP_MASK);
		if (ctl.Ly > UPPER_THRESHOLD) ret |= SNES_DOWN_MASK;
		if (ctl.Ly < LOWER_THRESHOLD) ret |= SNES_UP_MASK;
		if (ctl.Lx < LOWER_THRESHOLD) ret |= SNES_LEFT_MASK;
		if (ctl.Lx > UPPER_THRESHOLD) ret |= SNES_RIGHT_MASK;
	}
/*
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
*/
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

void S9xShutdown (void)
{
	if (g_thread != -1) {
		Settings.ThreadSound = FALSE;
		sceKernelWaitThreadEnd (g_thread, NULL);
		sceKernelDeleteThread  (g_thread);
	}

	S9xCloseSoundDevice ();
}

// S9x may call this if it encounters a fatal error... Don't actually
// exit, just stop running the ROM.
void S9xExit (void)
{
	Settings.Paused = TRUE;

	save_config ();

	Memory.SaveSRAM (S9xGetSRAMFilename ());
	Memory.Deinit   ();

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
	g_thread = sceKernelCreateThread( "sound thread", (SceKernelThreadEntry)S9xProcessSound, 0x8, 0x40000, THREAD_ATTR_USER, 0 );
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
	BEGIN_DEBUG_CODE

	S9xProcessEvents( FALSE );

	IPPU.FrameSkip = 0;
	IPPU.SkippedFrames = 0;
	IPPU.RenderThisFrame = TRUE;

	return;

	ELSE_RELEASE_CODE
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

	END_RELEASE_CODE
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

const char *S9xGetSRAMFilename (void)
{
	return S9xGetFilename ("srm");
}

bool8 S9xInitUpdate (void)
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
	
	BEGIN_DEBUG_CODE
		dest    = (uint32*)pgGetVramAddr( 0, 0 );
	ELSE_RELEASE_CODE
		dest    = (uint32*)pgGetVramAddr( (SCREEN_WIDTH - SNES_WIDTH) >> 1, (SCREEN_HEIGHT - height) >> 1 );
	END_RELEASE_CODE

	while ( height-- ){
		x = 16;
		while ( x-- ){
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

static uint16 __attribute__((aligned(16))) GFX_Screen[512 * 478];
static uint16	GFX_SubScreen[512 * 478];
static uint16	GFX_ZBuffer[512 * 478];
static uint16	GFX_SubZBuffer[512 * 478];


enum {
	SCR_X1,
	SCR_FIT,
	SCR_FULL,
	SCR_FULLFIT,
};

static char FPSbuf[6];

extern int S9xDisplayGetMaxCharsX (void);
extern void S9xDisplayStringEx (const char *string, int x, int y);

#ifdef OPTI
bool8 S9xDeinitUpdate (int Width, int Height)
#else
bool8 S9xDeinitUpdate (int Width, int Height, bool8 sixteen_bit)
#endif // OPTI
{
//	S9xPutImage( Width, Height );
// mod by y

	if (PSP_Settings.bUseGUBlit) {
		// Dirty framebuffer clear for sceGu blit
		if (s_iFramebufferState > 0) {
			s_iFramebufferState = 0;
			clear_framebuffer ();
		}

		// Special consideration for fullscreen modes and FPS drawing
		if (PSP_Settings.bShowFPS && PSP_Settings.iScreenSize >= SCR_SIZE_FULL) {
			Settings.DisplayColor = RGB (255, 255, 255);
			int max_chars = S9xDisplayGetMaxCharsX ();
			S9xDisplayStringEx (FPSbuf, max_chars - 5, 0);
		}

		const int tex_res = (PSP_Settings.bSupportHiRes ? 512 : 256);

		// If you don't call this, Gu will run into cache problems with
		// reading pixel data...
		sceKernelDcacheWritebackAll ();
		
		switch(PSP_Settings.iScreenSize) {
			case SCR_FIT:
			{
				// 60 pixels on either side (4:3)...
				const int x_scale = (PSP_Settings.bSupportHiRes ? 720 : 360);
				const int y_scale = (PSP_Settings.bSupportHiRes ? 624 : 312);
				pgRenderTex((char *)GFX.Screen,tex_res,tex_res,60,0,x_scale,y_scale);
			} break;
			case SCR_FULL:
			case SCR_FULLFIT:
			{
				const int x_scale = (PSP_Settings.bSupportHiRes ? 960 : 480);
				const int y_scale = (PSP_Settings.bSupportHiRes ? 624 : 312);
				pgRenderTex((char *)GFX.Screen,tex_res,tex_res,0,0,x_scale,y_scale);
			} break;
			case SCR_X1:
			default:
				const int xy_scale = (PSP_Settings.bSupportHiRes ? 512 : 256);
				pgRenderTex((char *)GFX.Screen,tex_res,tex_res,(480/2)-(256/2),(272/2)-(224/2),xy_scale,xy_scale);
				break;
		}
	} else {
		BEGIN_RELEASE_CODE
	
		switch(PSP_Settings.iScreenSize) {
			default:
			case SCR_SIZE_X1:
				break;
			case SCR_SIZE_FIT:
				pgBitBltFit((unsigned short *)GFX.Screen, Height);
				break;
			case SCR_SIZE_FULL:
				pgBitBltFull((unsigned long *)GFX.Screen, Height);
				break;
			case SCR_SIZE_FULLFIT:
				pgBitBltFullFit((unsigned short *)GFX.Screen, Height);
				break;
		}

		END_RELEASE_CODE
	}

	if (PSP_Settings.bShowFPS) {
		struct timeval	now;
		unsigned int	diff;
	
		s_iFrame++;
		gettimeofday (&now, 0);
	
		diff  = (now.tv_sec - s_tvStart.tv_sec) * 1000000 + now.tv_usec - s_tvStart.tv_usec;
		diff /= 1000000;
	
		if ( diff ){
			FPSbuf [0] = ((s_iFrame / diff) / 10) + '0';
			FPSbuf [1] = ((s_iFrame / diff) % 10) + '0';
			FPSbuf [2] = 'F';
			FPSbuf [3] = 'P';
			FPSbuf [4] = 'S';
			FPSbuf [5] = '\0';
	 
			s_tvStart = now;
			s_iFrame  = 0;
		}

		// Don't use this method of drawing the FPS if the screen mode
		// is FULL and sceGu blitting is enabled...
		if ((! PSP_Settings.bUseGUBlit) || PSP_Settings.iScreenSize < SCR_SIZE_FULL)
			pgPrintBG (CMAX_X - 6, 0, 0xffff, FPSbuf);
	}

	if (PSP_Settings.bVSync) {
		if (PSP_Settings.bUseGUBlit) {
			pgWaitV      ();
			pgScreenSync ();
		} else {		
			pgScreenFlipV ();
		}
	} else {
		if (PSP_Settings.bUseGUBlit)
		  ; // pgScreenSync ();
		else
			pgScreenFlip();
	}

	if (! PSP_Settings.bUseGUBlit) {
		BEGIN_RELEASE_CODE
	
			if (PSP_Settings.iScreenSize == 0)
				GFX.Screen = (uint8*)pgGetVramAddr( (SCREEN_WIDTH - SNES_WIDTH) >> 1, (SCREEN_HEIGHT - SNES_HEIGHT) >> 1 );
			else
				GFX.Screen = (uint8*)GFX_Screen;

		ELSE_DEBUG_CODE

			GFX.Screen = (uint8*)pgGetVramAddr( 0, 0 );

		END_DEBUG_CODE
		}
		
// FPS move --> before Flip
// analog_menu move --> S9xProcessEvents() by y

	// Dirty framebuffer clear for pg
	if (! PSP_Settings.bUseGUBlit) {
		if (s_iFramebufferState > 0) {
			s_iFramebufferState--;
			clear_framebuffer ();
		}
	}

	return TRUE;
}

// mod by y
void S9xInitDisplay( int argc, char** argv )
{
	PSP_Settings.bTrans = true;
	Settings.Transparency = TRUE;
#ifndef OPTI
	Settings.SixteenBit   = TRUE;
#endif // OPTI

	memset( GFX_Screen,	0, sizeof(GFX_Screen) );
	memset( GFX_SubScreen,  0, sizeof(GFX_SubScreen) );
	memset( GFX_ZBuffer,    0, sizeof(GFX_ZBuffer) );
	memset( GFX_SubZBuffer, 0, sizeof(GFX_SubZBuffer) );

//	GFX.Pitch      = IMAGE_WIDTH * 2;
	if (PSP_Settings.bUseGUBlit) {
		GFX.Pitch             = (PSP_Settings.bSupportHiRes ? 512 : 256) * 2;
		GFX.Screen            = (uint8*)GFX_Screen;
		Settings.SupportHiRes = PSP_Settings.bSupportHiRes;
	} else {
		GFX.Pitch             = 512 * 2;
		Settings.SupportHiRes = FALSE;

		BEGIN_RELEASE_CODE
			GFX.Screen     = (uint8*)pgGetVramAddr( (SCREEN_WIDTH - SNES_WIDTH) >> 1, (SCREEN_HEIGHT - SNES_HEIGHT) >> 1 );
		ELSE_DEBUG_CODE
			GFX.Screen     = (uint8*)pgGetVramAddr( 0, 0 );
		END_DEBUG_CODE
	}

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
		if (PSP_Settings.iBackgroundColor < 0 || PSP_Settings.iBackgroundColor > 1)
			PSP_Settings.iBackgroundColor = 0;
		if (PSP_Settings.iCompression < 0 || PSP_Settings.iCompression > 1)
			PSP_Settings.iCompression = 0;
		if (PSP_Settings.iSoundSync < 0 || PSP_Settings.iSoundSync > 2)
			PSP_Settings.iSoundSync = 0;
		if (!PSP_Settings.bShowDebugInfo)
			PSP_Settings.bShowDebugInfo = false;
		if (!PSP_Settings.bUseGUBlit)
			PSP_Settings.bUseGUBlit = false;
		if (!PSP_Settings.bSupportHiRes)
			PSP_Settings.bSupportHiRes = false;
		if (!PSP_Settings.bBilinearFilter)
			PSP_Settings.bBilinearFilter = false;
		if (PSP_Settings.iAltSampleDecode < 0 || PSP_Settings.iAltSampleDecode > 3)
			PSP_Settings.iAltSampleDecode = 0;
	}
	
	// TODO: Switch to a parsed config file format, and compress it.
	char vercnf[48];
	strcpy(vercnf, "uo_Snes9x for PSP Ver.0.02pd1");
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
		PSP_Settings.iPSP_ClockUp = 2;
		PSP_Settings.iScreenSize = 1;
		PSP_Settings.bAutoSkip = false;
		PSP_Settings.iBackgroundColor = 0;
		PSP_Settings.iCompression = 0;
		PSP_Settings.iSoundSync = 0;
		PSP_Settings.bShowDebugInfo = false;
		PSP_Settings.bUseGUBlit = true;
		PSP_Settings.bSupportHiRes = false;
		PSP_Settings.bBilinearFilter = true;
		PSP_Settings.iAltSampleDecode = 0;
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
	
	if (! PSP_Settings.bUseGUBlit) {
		BEGIN_RELEASE_CODE
	
		if (PSP_Settings.iScreenSize == 0) {
			GFX.Screen = (uint8*)pgGetVramAddr( (SCREEN_WIDTH - SNES_WIDTH) >> 1, (SCREEN_HEIGHT - SNES_HEIGHT) >> 1 );
		} else {
			GFX.Screen = (uint8*)GFX_Screen;
		}
	
		END_RELEASE_CODE
	} else {
		GFX.Screen = (uint8*)GFX_Screen;
	}
	
	if (PSP_Settings.iPSP_ClockUp == 2) scePowerSetClockFrequency(333,333,166);
	else if (PSP_Settings.iPSP_ClockUp == 1) scePowerSetClockFrequency(266,266,133);
	else scePowerSetClockFrequency(222,222,111);
	
	Settings.SoundSync = PSP_Settings.iSoundSync;

	Settings.AltSampleDecode = PSP_Settings.iAltSampleDecode;

	g_bSleep = false;

	init_blit_backend ();
}

void ustoa(unsigned short val, char *s);
// RINのソースから頂きました。
// みらきち様・kwn様 J が使用させて頂く事をお許し下さい。
// by kwn
// 壁紙保存用
unsigned short WALL_BITMAP[480*272];
int WALL_BITMAP_FLG = false;
int load_wall_bmp()
{
	int ret_int = -1;
	// sceIoOpenの戻り値
	int i_sceIoOpen_ret = -1;
	unsigned char *wall_bmp;
	unsigned char *vptr;
	static unsigned char wall_buf[ 480 * 272 * 3 + 0x36 ];
	char           wall_file_name[ _MAX_PATH ];
 	unsigned short x,y,yy,r,g,b,data;

	char drive [ _MAX_DRIVE + 1 ];
	char dir   [ _MAX_DIR   + 1 ];
	char fname [ _MAX_FNAME + 1 ];
	char ext   [ _MAX_EXT   + 1 ];

	_splitpath ( Memory.ROMFilename, drive, dir, fname, ext );
	strcpy( wall_file_name, drive );
	strcat( wall_file_name, dir );
	strcat( wall_file_name, "/" );
//	strcat( wall_file_name, fname );
	strcat( wall_file_name, "wall.bmp\0" );

	i_sceIoOpen_ret = sceIoOpen( wall_file_name, PSP_O_RDONLY, 0777 );
	if( i_sceIoOpen_ret >= 0 ) {
		ret_int = sceIoRead( i_sceIoOpen_ret, wall_buf, 480 * 272 * 3 + 0x36 );
	}
	sceIoClose( i_sceIoOpen_ret );
	// 正常に読込めたら
	if ( ret_int >= 0 ) {
		wall_bmp = wall_buf + 0x36;
		vptr = (unsigned char*)WALL_BITMAP;
		for( y = 0; y < 272; y++ ) {
			for( x = 0; x < 480; x++ ) {
				yy = 271 - y;
				r = *( wall_bmp + ( yy * 480 + x ) * 3 + 2 );
				g = *( wall_bmp + ( yy * 480 + x ) * 3 + 1 );
				b = *( wall_bmp + ( yy * 480 + x ) * 3     );
				data = ( ( ( b & 0xf8 ) << 7 ) | ( ( g & 0xf8 ) << 2 ) | ( r >> 3 ) );
				*(unsigned short *)vptr = data;
				vptr += 2;
			}
		}
		WALL_BITMAP_FLG = true;
	}else {
		WALL_BITMAP_FLG = false;
	}
	return ret_int;
}

#define D_dialog_pos_x1 34
#define D_dialog_pos_y1 58

int   S9xFreezeGame_PSP   (const char* filename);
bool8 S9xUnfreezeGame_PSP (const char* filename);


//     SRAM日付取得用構造体
struct SceIoDirent sram_file_data[ 1 ];
int sram_save_check()
{
	int  ret_int = false;
	//   
	int  i_sceIoDopen_ret = -1;
	//   sramファイルのフルパス保存用変数
	char save_sram_file_name[ _MAX_PATH ];
	//   ダイアログテキストの保存用変数
	char dialog_text_all[ D_text_all_MAX ];
	//   日付・日時取得用
	char get_date_data[20];
	//   
	int dir_files_cnt = 0;
	
	char drive [_MAX_DRIVE + 1];
	char dir   [_MAX_DIR + 1];
	char fname [_MAX_FNAME + 1];
	char ext   [_MAX_EXT + 1];
	_splitpath (Memory.ROMFilename, drive, dir, fname, ext);
	// SAVEフォルダに保存
	strcpy( save_sram_file_name, PBPPath );
	strcat( save_sram_file_name, "SAVE/\0" );

	strcpy( dialog_text_all, fname );
	strcat( dialog_text_all, ".srm\0" );
	i_sceIoDopen_ret = sceIoDopen( save_sram_file_name );
	dir_files_cnt = 0;
	while( 1 ) {
		if( sceIoDread( i_sceIoDopen_ret, &sram_file_data[ 0 ] ) <= 0 ) {
			dir_files_cnt = -1;
			break;
		}else {
			if ( strcasecmp ( dialog_text_all, sram_file_data[ 0 ].d_name ) == 0 ) {
				dir_files_cnt = 1;
				break;
			}
		}
	}
	sceIoClose( i_sceIoDopen_ret );
	if ( dir_files_cnt == 1 ) {
		//   ループカウント
		int  loop_cnt;
		//   タイトル文字列
		char title_string[ D_text_MAX ];

		// ダイアログタイトル文字列作成
		strcpy( title_string, " 　【　　SRAM  Save　　】　 \0" );

		// メッセージ作成
		strcat( dialog_text_all, " - \0" );
		ustoa ( sram_file_data[ 0 ].d_stat.st_mtime.year , get_date_data );
		strcat( dialog_text_all, get_date_data );
		strcat( dialog_text_all, "/00/00 00:00:00\0" );

		// 月
		if( sram_file_data[ 0 ].d_stat.st_mtime.month  > 10 ) 
			dialog_text_all[ strlen( dialog_text_all ) -14] = sram_file_data[ 0 ].d_stat.st_mtime.month / 10 + '0';
			dialog_text_all[ strlen( dialog_text_all ) -13] = sram_file_data[ 0 ].d_stat.st_mtime.month % 10 + '0';
		// 日
		if( sram_file_data[ 0 ].d_stat.st_mtime.day  > 10 ) 
			dialog_text_all[ strlen( dialog_text_all ) -11] = sram_file_data[ 0 ].d_stat.st_mtime.day / 10 + '0';
			dialog_text_all[ strlen( dialog_text_all ) -10] = sram_file_data[ 0 ].d_stat.st_mtime.day % 10 + '0';
		// 時間
		if( sram_file_data[ 0 ].d_stat.st_mtime.hour > 10 ) 
			dialog_text_all[ strlen( dialog_text_all ) -8] = sram_file_data[ 0 ].d_stat.st_mtime.hour / 10 + '0';
			dialog_text_all[ strlen( dialog_text_all ) -7] = sram_file_data[ 0 ].d_stat.st_mtime.hour % 10 + '0';
		// 分
		if( sram_file_data[ 0 ].d_stat.st_mtime.minute  > 10 ) 
			dialog_text_all[ strlen( dialog_text_all ) -5] = sram_file_data[ 0 ].d_stat.st_mtime.minute / 10 + '0';
			dialog_text_all[ strlen( dialog_text_all ) -4] = sram_file_data[ 0 ].d_stat.st_mtime.minute % 10 + '0';
		// 秒
		if( sram_file_data[ 0 ].d_stat.st_mtime.second  > 10 ) 
			dialog_text_all[ strlen( dialog_text_all ) -2] = sram_file_data[ 0 ].d_stat.st_mtime.second / 10 + '0';
			dialog_text_all[ strlen( dialog_text_all ) -1] = sram_file_data[ 0 ].d_stat.st_mtime.second  % 10 + '0';

		// メッセージ作成
		strcat( dialog_text_all, "\n\n\0"                           ); // 改行x2
		strcat( dialog_text_all, "セーブデータを上書きします。\n\0" );
		strcat( dialog_text_all, "      よろしいですか？\n\0"       );
		strcat( dialog_text_all, "\n\0"                             ); // 改行
		strcat( dialog_text_all, "    はい □ ／ いいえ ×\n\0"     );
		strcat( dialog_text_all, "\n\0"                             ); // 改行

		// ダイアログ表示
		message_dialog( D_dialog_pos_x1, D_dialog_pos_y1, title_string, dialog_text_all );

		// 無限ループ気持ち悪い
		while( 1 ){
			readpad();
			// ×ボタン押した
			if ( new_pad & PSP_CTRL_CROSS ) {
				ret_int = false;
				break;
			// □ボタン押す
			} else if ( new_pad & PSP_CTRL_SQUARE ){
				ret_int = true;
				break;
			}
		}
	}else {
		ret_int = false;
	}

	return ret_int;
}

#define D_dialog_pos_x0 34
#define D_dialog_pos_y0 74
// ステートセーブチェックルーチン
int state_save_check()
{
	int ret_int = false;
	// ダイアログテキストの保存用変数
	char dialog_text_all[ D_text_all_MAX ];

	// 選択しているセーブスロットにセーブデータが無ければ保存
	if ( save_slots[PSP_Settings.iSaveSlot].flag == false ) {
		ret_int = true;
	}else {
	// 選択しているセーブスロットにセーブデータが有ればダイアログを開く
		strcpy( dialog_text_all, save_slots[ PSP_Settings.iSaveSlot ].date ); // セーブデータ情報表示
		strcat( dialog_text_all, "\n\n\0"                           );        // 改行x2
		strcat( dialog_text_all, "セーブデータを上書きします。\n\0" );
		strcat( dialog_text_all, "      よろしいですか？\n\0"       );
		strcat( dialog_text_all, "\n\0"                             );        // 改行
		strcat( dialog_text_all, "    はい □ ／ いいえ ×\n\0"     );
		strcat( dialog_text_all, "\n\0"                             );        // 改行
		// ダイアログ表示
		message_dialog( D_dialog_pos_x0, D_dialog_pos_y0, " 　【　　State Save　　】　 \0", dialog_text_all );
		// 無限ループ気持ち悪い
		while( 1 ){
			readpad();
			// ×ボタン押した
			if ( new_pad & PSP_CTRL_CROSS ) {
				break;
			// □ボタン押す
			} else if ( new_pad & PSP_CTRL_SQUARE ){
				ret_int = true;
				break;
			}
		}
	}
	return ret_int;
}

#define D_dialog_pos_x2 34
#define D_dialog_pos_y2 90
// ステートデリートチェックルーチン
int state_delete_check()
{
	int ret_int = false;
	// ダイアログテキストの保存用変数
	char dialog_text_all[ D_text_all_MAX ];

	// 選択しているセーブスロットにセーブデータが無ければ削除
	if ( save_slots[PSP_Settings.iSaveSlot].flag == false ) {
		ret_int = true;
	}else {
	// 選択しているセーブスロットにセーブデータが有ればダイアログを開く
		strcpy( dialog_text_all, save_slots[ PSP_Settings.iSaveSlot ].date ); // セーブデータ情報表示
		strcat( dialog_text_all, "\n\n\0"                           );        // 改行x2
		strcat( dialog_text_all, "セーブデータを削除します。\n\0"   );
		strcat( dialog_text_all, "     よろしいですか？\n\0"        );
		strcat( dialog_text_all, "\n\0"                             );        // 改行
		strcat( dialog_text_all, "   はい □ ／ いいえ ×\n\0"      );
		strcat( dialog_text_all, "\n\0"                             );        // 改行
		// ダイアログ表示
		message_dialog( D_dialog_pos_x2, D_dialog_pos_y2, " 　【　　State Delete　】　 \0", dialog_text_all );
		// 無限ループ気持ち悪い
		while( 1 ){
			readpad();
			// ×ボタン押した
			if ( new_pad & PSP_CTRL_CROSS ) {
				break;
			// □ボタン押す
			} else if ( new_pad & PSP_CTRL_SQUARE ){
				ret_int = true;
				break;
			}
		}
	}
	return ret_int;
}


enum {
	SRAM_SAVE,
		
	STATE_SLOT,
	STATE_SAVE,
	STATE_LOAD,
	STATE_DEL,
	STATE_COMPRESS,
	STATE_CONFIG,
	STATE_REFRESH,

	DISPLAY_CONFIG,
	SOUND_CONFIG,
	KEY_CONFIG,

	FRAME_SKIP,
	AUTO_SKIP,
	HBLANK_CYCLE,
	APU_CYCLE,
	PSP_CLOCKUP,

	LOAD_ROM,
	RESET,
	EXIT_HOME,
	CONTINUE,
};

void draw_menu (void)
{
	char* msg = g_szMainMenuMsg;
	int&  sel = g_iMainMenuSel;

	char tmp[256];
	uint8 tmp_color;

	int x,y;
	
//  add by J
	// 充電電圧
	long BatteryVolt_ret;
	// アダプタ・バッテリー判定
	long BatteryCharging_ret;

	menu_frame((unsigned char *)msg, (unsigned char *)"○：OK  ×：Continue  SELECT+START：Exit to PSP Menu");
		
	mh_print(33, 33, (unsigned char*)Memory.ROMFilename, RGB(95,95,125));
		
	x = 2;
	y = 6;
		
	pgPrint(x,y++,0xffff,"  SRAM Save");
		
	y++;
		
	strcpy(tmp,"  Save Slot     : 0");
	tmp[strlen(tmp)-1] = PSP_Settings.iSaveSlot + '0';
	pgPrint(x,y++,0xffff,tmp);
		
	pgPrint(x,y++,0xffff,"  State Save");
	pgPrint(x,y++,0xffff,"  State Load");
	pgPrint(x,y++,0xffff,"  State Delete");
		
	if (save_slots [PSP_Settings.iSaveSlot].compression == 1)
		pgPrint(x,y++,0xffff,"  State Decompress");
	else
		pgPrint(x,y++,0xffff,"  State Compress");

	pgPrint(x,y++,0xffff,"  State Config");

	pgPrint(x,y++,0xffff,"  Refresh List");

	y++;
		
	pgPrint(x,y++,0xffff,"  Display Config");

	pgPrint(x,y++,0xffff,"  Sound Config");

	pgPrint(x,y++,0xffff,"  Key Config");

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
	if (sel >= STATE_SLOT) {
		y++;
	}
	if (sel >= DISPLAY_CONFIG){
		y++;
	}
	if (sel >= FRAME_SKIP){
		y++;
	}
	if (sel >= LOAD_ROM){
		y++;
	}
		
	pgPutChar((x+1)*8,y*8,0xffff,0,127,1,0,1);
		
	if (save_slots[PSP_Settings.iSaveSlot].flag && save_slots[PSP_Settings.iSaveSlot].thumbflag) {
		pgDrawFrame(327,129,456,242,RGB(85,85,95));
		pgBitBlt(328,130,128,112,1,save_slots[PSP_Settings.iSaveSlot].thumbnail);
	}

	int  size_of_states = 0;
	char state_list_txt  [64];
	char slot_descriptor [64];

	for(y=0; y<=SAVE_SLOT_MAX; y++){
		size_of_states += save_slots [y].size;
			
		if (save_slots [y].flag)
			sprintf (slot_descriptor, "%s - %3d Kb", save_slots [y].date, save_slots [y].size / 1024);
		else
			sprintf (slot_descriptor, "%s", save_slots [y].date);
				
		if (y==PSP_Settings.iSaveSlot) {
			pgPrint(22,y+7,0xffff,slot_descriptor);
		} else {
			pgPrint(22,y+7,RGB(105,105,115),slot_descriptor);
		}
	}
		
	sprintf (state_list_txt, "State Save List             %1.2f Mb", ((float)size_of_states / 1024.0f / 1024.0f));
	pgPrint (22,6, RGB (105,105,115), state_list_txt);

// add by J
	// このAPI遅いんじゃないかな？
	BatteryVolt_ret     = scePowerGetBatteryVolt(); // バッテリー充電電圧(5000で5Vかな)
	BatteryCharging_ret = scePowerIsBatteryCharging(); // バッテリーなし:- 充電中:1 アダプターなし:0
	char message[256];
	char BatteryVolt_ret_text[16];
	char BatteryCharge_percent[16];
	bool BatteryFull = false;
	if ( ( BatteryVolt_ret > 999 ) && ( BatteryVolt_ret < 9999 )) {
		sprintf (BatteryVolt_ret_text, "(%1.3fV)", (float)BatteryVolt_ret / 1000);
	}else {
		strcpy( BatteryVolt_ret_text, "(No Battery or Bad Voltage)" );
	}
	// Assume 4.13 volts indicates a full battery... (*** Is this valid? -Andon)
	if (BatteryVolt_ret < 4130) {
		sprintf (BatteryCharge_percent, "%d%%", (int)(((float)BatteryVolt_ret / 4130.0f) * 100.0f), BatteryVolt_ret_text);
	} else {
		BatteryFull = true;
		strcpy (BatteryCharge_percent, "Full");
	}
	// Running on AC power...
	if ( BatteryCharging_ret <  0 ) {
		mh_print(0, 0, (unsigned char*)"[Running on AC Power]", RGB(0,255,0));
	}
	// Battery is being used...
	else if ( BatteryCharging_ret == 0 ) {
		sprintf  (message, "[Battery: %s %s]", BatteryCharge_percent, BatteryVolt_ret_text);
		mh_print (0, 0, (unsigned char*)message, BatteryFull ? RGB (0,0,255) :
		                                                       RGB (255,0,0));
	}
	// Battery is being charged...
	else {
		long BatteryTemp_ret = scePowerGetBatteryTemp();
		// If the battery temp. is > 38C (100F), display the temp.
		if ((BatteryTemp_ret > 38) && (BatteryTemp_ret < 100)) {
			sprintf (message, "[Charging: %s %s - %dｰ F]", BatteryCharge_percent, BatteryVolt_ret_text, (int)((9.0f/5.0f) * (float)BatteryTemp_ret) + 32);
			mh_print(0, 0, (unsigned char*)message, RGB(255,0,0));
		} else {
			sprintf( message, "[Charging: %s %s]", BatteryCharge_percent, BatteryVolt_ret_text );
			mh_print(0, 0, (unsigned char*)message, RGB(0,0,255));
		}
	}
}

void close_menu (void)
{
	S9xMarkScreenDirtyEx ();

/*
	BEGIN_RELEASE_CODE
	if (PSP_Settings.iScreenSize == 0) {
		GFX.Screen = (uint8*)pgGetVramAddr( (SCREEN_WIDTH - SNES_WIDTH) >> 1, (SCREEN_HEIGHT - SNES_HEIGHT) >> 1 );
	} else {
		GFX.Screen = (uint8*)GFX_Screen;
	}
	END_RELEASE_CODE
*/

	if      (PSP_Settings.iPSP_ClockUp == 2) scePowerSetClockFrequency (333, 333, 166);
	else if (PSP_Settings.iPSP_ClockUp == 1) scePowerSetClockFrequency (266, 266, 133);
	else                                     scePowerSetClockFrequency (222, 222, 111);
	
	g_bSleep = false;

	// Restore GU blitting, if used
	bGUIMode = FALSE;
	init_pg ();
}

void open_menu (void)
{
	// Do this before screwing with the blit backend...
	get_screenshot((unsigned char *)GFX.Screen);

	// While the menu is open, temporarily disable GU blitting...
	bGUIMode = TRUE;
	init_pg ();

	char* msg = g_szMainMenuMsg;
	int&  sel = g_iMainMenuSel;
	
	scePowerSetClockFrequency (222, 222, 111);
	
	// mod by a - Only do this when a new state is written or a manual refresh is performed
	//            Doing this here would make opening the menu take longer...
	///get_slotdate(S9xGetFilename("sv0"));
	///get_thumbs(S9xGetFilename("tn0"));

	bool f_bExit = false;
	
	old_pad = PSP_CTRL_LEFT;
	
	msg[0]=0;

	for(;;){
		readpad();

		if (new_pad & PSP_CTRL_CIRCLE){
			if (sel == SRAM_SAVE){
				// mod by J
				if ( sram_save_check() == true ) {
					save_config();
					if ( Memory.SaveSRAM( S9xGetFilename("srm") ) ) {
							strcpy(msg, "SRAM Saved Successfully.");
					} else {
							strcpy(msg, "SRAM Save Failed or Not Found SRAM.");
					}
				}else {
					strcpy(msg, "SRAM Save Cancel.");
				}
			} else if (sel == STATE_SLOT){
				PSP_Settings.iSaveSlot++;
				if (PSP_Settings.iSaveSlot > SAVE_SLOT_MAX) {
					PSP_Settings.iSaveSlot = 0;
				}
			} else if (sel == STATE_SAVE){
				// mod by J
				if ( state_save_check() == true ) {
					// Speed the clock up to save time saving states
					scePowerSetClockFrequency (333, 333, 166);

					pgFillBox( 129, 104, 351, 168, 0 );
					mh_print(195, 132, (unsigned char*)"Now State Saving...", RGB(255,205,0));
					pgScreenFlipV();
					save_config();
					Memory.SaveSRAM( S9xGetFilename("srm") );
					
					int S9xFreezeGame_PSP (const char* filename);
					if ( S9xFreezeGame_PSP (S9xGetFilename("sv0") ) ) {
							if (PSP_Settings.bSaveThumb) {
								save_thumb(S9xGetFilename("tn0"));
							} else {
								delete_file(S9xGetFilename("tn0"));
							}
							strcpy(msg, "State Saved Successfully.");
					} else {
							strcpy(msg, "State Save Failed.");
					}
					
					refresh_state_list ();
					
					S9xSetSoundMute( TRUE );

					scePowerSetClockFrequency (222, 222, 111);
				}else {
					strcpy(msg, "State Save Cancel.");
				}
			} else if (sel == STATE_LOAD){
				scePowerSetClockFrequency (333, 333, 166);

				if ( S9xUnfreezeGame_PSP ( S9xGetFilename("sv0") ) ) {
					Memory.LoadSRAM( S9xGetFilename("srm") );
					S9xSetInfoString( "State Loaded." );
					break;
				} else {
					strcpy(msg, "State Load Failed.");
				}
			} else if (sel == STATE_DEL){
				// mod by J
				if ( state_delete_check() == true ) {
					// ステートセーブデータ削除
					delete_file(S9xGetFilename("sv0"));
					delete_file(S9xGetFilename("tn0"));
					strcpy(msg, "State Delete Success.");
				}else {
					strcpy(msg, "State Delete Cancel.");
				}

				refresh_state_list ();
			} else if (sel == STATE_COMPRESS) {
				scePowerSetClockFrequency (333, 333, 166);

				bool success     = false;
				int  compression = save_slots [PSP_Settings.iSaveSlot].compression;

				SceIoStat original_stat;
				sceIoGetstat (S9xGetFilename ("sv0"), &original_stat);

				STREAM state = NULL;

				if (S9xOpenSnapshotFile (S9xGetFilename ("sv0"), TRUE, &state))
				{
					unsigned int dest_len = 0;
					static char  dest_buf [MAX_FREEZE_SIZE];

					REVERT_STREAM (state, 0, SEEK_END);
					unsigned int state_len = FIND_STREAM (state);
					REVERT_STREAM (state, 0, SEEK_SET);

					static char state_buf [MAX_FREEZE_SIZE];
					READ_STREAM   (state_buf, state_len, state);

					S9xCloseSnapshotFile (state);

					if (S9xOpenSnapshotFile (S9xGetFilename ("sv0"), FALSE, &state)) {
						pgFillBox (129, 104, 351, 168, 0);

						// Decompress
						if (compression == 1) {
							mh_print      (195, 132, (unsigned char*)"Now Decompressing...", RGB (255,205,0));
							pgScreenFlipV ();

							// Skip the RLE header
							dest_len = rle_decode ((state_buf + RLE_MAGIC_LEN), (state_len - RLE_MAGIC_LEN), dest_buf, MAX_FREEZE_SIZE);
						}

						// Compress
						else {
							mh_print      (195, 132, (unsigned char*)"Now Compressing...", RGB (255,205,0));
							pgScreenFlipV ();

							dest_len = rle_encode (state_buf, state_len, dest_buf, MAX_FREEZE_SIZE);

							// Write the RLE header first
							WRITE_STREAM (RLE_MAGIC, RLE_MAGIC_LEN, state);
						}

						WRITE_STREAM (dest_buf, dest_len, state);

						S9xCloseSnapshotFile (state);

						// Store the original modification time
						SceIoStat new_stat;
						sceIoGetstat (S9xGetFilename ("sv0"), &new_stat);

						new_stat.st_mtime = original_stat.st_mtime;

						sceIoChstat (S9xGetFilename ("sv0"), &new_stat, 0xffffffff /* TODO: What's the proper bitmask?! */);
						

						success = true;
					}
				}

				if (compression == 1) {
					if (success)
						strcpy (msg, "State Decompressed Successfully.");
					else
						strcpy (msg, "State Decompression Failed.");
				} else {
					if (success)
						strcpy (msg, "State Compressed Successfully.");
					else
						strcpy (msg, "State Compression Failed.");
				}

				refresh_state_list ();
				scePowerSetClockFrequency (222, 222, 111);
			} else if (sel == STATE_REFRESH) {
				refresh_state_list ();
			} else if (sel == STATE_CONFIG) {
				state_config ();
			} else if (sel == DISPLAY_CONFIG) {
				display_config ();
			} else if (sel == SOUND_CONFIG) {
				sound_config ();
			} else if (sel == FRAME_SKIP){
				PSP_Settings.iSkipFrames++;
				if ( PSP_Settings.iSkipFrames > 10 ){
					PSP_Settings.iSkipFrames = 0;
				}
			} else if (sel == AUTO_SKIP){
				PSP_Settings.bAutoSkip = !PSP_Settings.bAutoSkip;
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
			} else if (sel == KEY_CONFIG){
				// mod by J
				key_config();
			} else if (sel == PSP_CLOCKUP){
				PSP_Settings.iPSP_ClockUp++;
				if (PSP_Settings.iPSP_ClockUp > 2) {
					PSP_Settings.iPSP_ClockUp = 0;
				}
			} else if (sel == LOAD_ROM){
				// Speed the clock up to save time loading ROMs (especially compressed ones)
				scePowerSetClockFrequency (333, 333, 166);

				msg[0]=0;
				FilerMsg[0]=0;
				if (getFilePath(RomPath)){
					save_config();
					Memory.SaveSRAM( S9xGetFilename("srm") );
					if ( Memory.LoadROM(RomPath) ){
						Memory.LoadSRAM( S9xGetFilename("srm") );
						load_config();
						// add by J
						load_wall_bmp();
						Settings.Paused = FALSE;

						refresh_state_list ();

						close_menu ();

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
			if (sel >= STATE_SLOT && sel <= STATE_REFRESH){
				if (PSP_Settings.iSaveSlot < 1) {
					PSP_Settings.iSaveSlot = SAVE_SLOT_MAX + 1;
				}
				PSP_Settings.iSaveSlot--;
			} else if (sel == FRAME_SKIP){
				if ( PSP_Settings.iSkipFrames > 0 ){
					PSP_Settings.iSkipFrames--;
				}
			} else if (sel == AUTO_SKIP){
				PSP_Settings.bAutoSkip = true;
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
			} else if (sel == PSP_CLOCKUP){
				if (PSP_Settings.iPSP_ClockUp < 1) {
					PSP_Settings.iPSP_ClockUp = 1;
				}
				PSP_Settings.iPSP_ClockUp--;
			}
		} else if (new_pad & PSP_CTRL_RIGHT){
			if (sel >= STATE_SLOT && sel <= STATE_REFRESH) {
				PSP_Settings.iSaveSlot++;
				if (PSP_Settings.iSaveSlot > SAVE_SLOT_MAX) {
					PSP_Settings.iSaveSlot = 0;
				}
			} else if (sel == FRAME_SKIP){
				if ( PSP_Settings.iSkipFrames < 10 ){
					PSP_Settings.iSkipFrames++;
				}
			} else if (sel == AUTO_SKIP){
				PSP_Settings.bAutoSkip = false;
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
			} else if (sel == PSP_CLOCKUP){
				PSP_Settings.iPSP_ClockUp++;
				if (PSP_Settings.iPSP_ClockUp > 2) {
					PSP_Settings.iPSP_ClockUp = 2;
				}
			}
		} else if ((now_pad & PSP_CTRL_SELECT) && (now_pad & PSP_CTRL_START)) {
			f_bExit = true;
			break;
		}
		// If both, the L and R triggers are down, don't do anything...
		else if (new_pad & PSP_CTRL_LTRIGGER && (! (new_pad & PSP_CTRL_RTRIGGER))){
			if (sel>LOAD_ROM) {
				sel=LOAD_ROM;
			} else if (sel>FRAME_SKIP){
				sel=FRAME_SKIP;
			} else if (sel>DISPLAY_CONFIG){
				sel=DISPLAY_CONFIG;
			} else if (sel>STATE_SLOT){
				sel=STATE_SLOT;
			} else if (sel>SRAM_SAVE){
				sel=SRAM_SAVE;
			}
		}
		// If both, the L and R triggers are down, don't do anything...
		else if (new_pad & PSP_CTRL_RTRIGGER && (! (new_pad & PSP_CTRL_LTRIGGER))){
			if (sel<STATE_SLOT) {
				sel=STATE_SLOT;
			} else if (sel<DISPLAY_CONFIG) {
				sel=DISPLAY_CONFIG;
			} else if (sel<FRAME_SKIP) {
				sel=FRAME_SKIP;
			} else if (sel<LOAD_ROM){
				sel=LOAD_ROM;
			} else if (sel<CONTINUE){
				sel=CONTINUE;
			}
		}
		
		draw_menu     ();
		pgScreenFlipV ();
	}
	
	if (f_bExit) {
		pgFillBox( 129, 104, 351, 168, 0 );
		mh_print(210, 132, (unsigned char*)"Good bye ...", 0xffff);
		pgScreenFlipV();

		// Shutdown Snes9x
		S9xExit     ();
		S9xShutdown ();

		g_bLoop = false;

		// Exit game
		sceKernelExitGame ();
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

	close_menu ();
}

void S9xProcessEvents( bool8 block )
{
	if ( s_iAnalog == ANALOG_NONE ) {
	return;
	}
	
	if ( s_iAnalog == ANALOG_LEFT ){
		S9xSetSoundMute( TRUE );
	
		open_menu();

		pgWaitVn (8);

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

void init_pg (void)
{
	pgMain ();
	pgInit ();
	
	pgScreenFrame (2, 0);
}

void init_blit_backend (void)
{
	if (PSP_Settings.bUseGUBlit) {
		S9xSetRenderPixelFormat (BGR555);
	} else {
		S9xSetRenderPixelFormat (BGR555);
	}

	init_pg ();

	S9xInitDisplay (0, 0);

	S9xGraphicsDeinit ();
	S9xGraphicsInit   ();

	S9xMarkScreenDirty ();
}

int main(int argc, char **argv)
{
//	debug_log( argv );

	pspDebugInstallErrorHandler (NULL);
	pspDebugScreenInit          ();

	PSP_Settings.bUseGUBlit = FALSE; // Disable this by default

// mod by y
	char *p, savedir[_MAX_PATH];
//	pgMain();
	strcpy(PBPPath, argv[0]);
	p = strrchr(PBPPath, '/');
	*++p = 0;

	SetupCallbacks();

//	pgInit();

//	pgScreenFrame(2,0);
	init_blit_backend ();

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

//	debug_log( "Snes9x for PSP Ver.0.02" );

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
Settings.SoundBufferSize = MAX_SOUND_SAMPLE;/*SOUND_SAMPLE;*/
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

// add by J
	key_config_init ();

	// Speed the clock up to save time loading ROMs (especially compressed ones)
	scePowerSetClockFrequency (333, 333, 166);

// mod by y
	strcpy(LastPath,PBPPath);
	FilerMsg[0]=0;
	for(;;){
		while(!getFilePath(RomPath))
			;
		if ( !Memory.LoadROM(RomPath) ){
			continue;
		} else {
			refresh_state_list ();

			Settings.Paused = FALSE;
		}
		break;
	}
// mod by J
	load_wall_bmp ();

	S9xMarkScreenDirty ();
	Memory.LoadSRAM( S9xGetFilename( "srm" ) );
//	S9xLoadCheatFile (S9xGetFilename ("cht"));

	g_bROMLoaded = true;
	g_bSleep     = false;
	
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

	S9xMarkScreenDirty ();

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

void S9xMarkScreenDirtyEx (void)
{
	// Mark front/back buffers dirty (requires a FULL update)
	s_iFramebufferState = 2;
}

void S9xMarkScreenDirty (void)
{
	// Don't clean the screen if debug info is enabled...
	BEGIN_DEBUG_CODE
		return;
	END_DEBUG_CODE

	S9xMarkScreenDirtyEx ();
}

void clear_framebuffer (void)
{
	if (WALL_BITMAP_FLG == true) {
		pgFillvram (0x0000);
		pgBitBlt   (0, 0, 480, 272, 1, WALL_BITMAP);
	}
	else if (PSP_Settings.iBackgroundColor > 0) {
		pgFillvram (0xffff);
	}
	else {
		pgFillvram (0x0000);
	}
}

// This is a special form of S9xFreezeGame that's optimized for the PSP...
//
// Instead of writing each memory block/struct to the Memory Stick individually,
// they're copied to a data buffer and written to the Memory Stick when finished.
int S9xFreezeGame_PSP (const char* filename)
{
	static char freeze_data [MAX_FREEZE_SIZE];
	int         freeze_bytes = S9xFreezeGameToBuffer (freeze_data);

	if (freeze_bytes > 0) {
		STREAM stream = NULL;

		if (S9xOpenSnapshotFile (filename, FALSE, &stream))
		{
			int size = 0;
			
			if (PSP_Settings.iCompression == 1) {
				unsigned int rle_length;
				static char  rle_data [MAX_FREEZE_SIZE];
				
				rle_length = rle_encode (freeze_data, freeze_bytes, rle_data, MAX_FREEZE_SIZE);

				WRITE_STREAM (RLE_MAGIC, RLE_MAGIC_LEN, stream);
				WRITE_STREAM (rle_data,  rle_length,    stream);

				size = rle_length;
			} else {
				WRITE_STREAM (freeze_data, freeze_bytes, stream);

				size = freeze_bytes;
			}

			S9xCloseSnapshotFile (stream);

			return size;
		} else {
			return 0;
		}
	}

	return 0;
}

extern "C" {
bool8 S9xIsFreezeGameRLE (void* data)
{
    char header [4];
    memcpy (header, data, RLE_MAGIC_LEN);

    if (strncmp (header, RLE_MAGIC, RLE_MAGIC_LEN) == 0)
      return (TRUE);

    return (FALSE);
}
};

// This is a special form of S9xUnfreezeGame that's specialized for the PSP...
//
// Instead of reading each memory block/struct to the Memory Stick individually,
// they're read into one large data buffer... And optionally runlength decoded.
bool8 S9xUnfreezeGame_PSP (const char* filename)
{
    static char freeze_data [MAX_FREEZE_SIZE];

    STREAM snapshot = NULL;
    if (S9xOpenSnapshotFile (filename, TRUE, &snapshot))
    {
		REVERT_STREAM (snapshot, 0, SEEK_END);
		int len = FIND_STREAM (snapshot);
		REVERT_STREAM (snapshot, 0, SEEK_SET);
		
		READ_STREAM (freeze_data, len, snapshot);

		extern bool8 S9xUnfreezeGameFromBuffer (const void* buffer, unsigned int buffer_size);

		bool8 result;

		if (S9xIsFreezeGameRLE ((void *)freeze_data)) {
			unsigned int rle_len;
			static char  rle_freeze_data [MAX_FREEZE_SIZE];
			rle_len = rle_decode (freeze_data + RLE_MAGIC_LEN, len - RLE_MAGIC_LEN, rle_freeze_data, MAX_FREEZE_SIZE);
			
			result = S9xUnfreezeGameFromBuffer (rle_freeze_data, rle_len);
		} else {
			result = S9xUnfreezeGameFromBuffer (freeze_data, len);
		}

		S9xCloseSnapshotFile (snapshot);

		return result;
    }

    return (FALSE);
}


void refresh_state_list (void)
{
	get_slotdate (S9xGetFilename ("sv0"));
	get_thumbs   (S9xGetFilename ("tn0"));
}


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

int atoi (const char *s)
{
  int neg = 0, ret = 1;

  for (;ret;++s) {
    switch(*s) {
      case ' ':
      case '\t':
        continue;
      case '-':
        ++neg;
      case '+':
        ++s;
      default:
        ret = 0;
    }
    break;
  }

  while ((*s >= '0') && (*s <= '9'))
    ret = ((ret * 10) + (int)(*s++ - '0'));

  return ((neg == 0) ? ret : -ret);
}
}

// Call this to clear the execute bit on a particular file...
//
//  sceIoOpen requires 0777 for creating files, so new files are
//  always flagged executable.
void clear_execute_bit (const char* filename)
{
	SceIoStat filestat;
	sceIoGetstat (filename, &filestat);

	filestat.st_attr &= ~FIO_SO_IXOTH;
	filestat.st_mode &= ~(FIO_S_IXUSR | FIO_S_IXGRP | FIO_S_IXOTH);

	sceIoChstat (filename, &filestat, 0xffffffff);
}

#include "font.h"

extern void DisplayChar (uint8 *Screen, uint8 c);

int S9xDisplayGetMaxCharsX (void)
{
	return IPPU.RenderedScreenWidth / (font_width - 1);
}

void S9xDisplayStringEx (const char *string, int x, int y)
{
    uint8 *Screen = GFX.Screen +
		    (font_height * y) * GFX.Pitch2 + 
#ifdef OPTI
		    (((font_width - 1) * sizeof (uint16)) * x);
#else
		    ((Settings.SixteenBit ? (font_width - 1) * sizeof (uint16) : 
		  (font_width - 1) * x);
#endif

    int len = strlen (string);
    int max_chars = IPPU.RenderedScreenWidth / (font_width - 1);
    int char_count = 0;
    int i;

    for (i = 0; i < len; i++, char_count++)
    {
	if (char_count >= max_chars || string [i] < 32)
	{
#ifdef OPTI
	    Screen -= (font_width - 1) * sizeof (uint16) * max_chars;
#else
	    Screen -= Settings.SixteenBit ? 
			(font_width - 1) * sizeof (uint16) * max_chars :
			(font_width - 1) * max_chars;
#endif // OPTI
	    Screen += font_height * GFX.Pitch;
	    if (Screen >= GFX.Screen + GFX.Pitch * IPPU.RenderedScreenHeight)
		break;
	    char_count -= max_chars;
	}
	if (string [i] < 32)
	    continue;
	DisplayChar (Screen, string [i]);
#ifdef OPTI
	Screen += (font_width - 1) * sizeof (uint16);
#else
	Screen += Settings.SixteenBit ? (font_width - 1) * sizeof (uint16) : 
		  (font_width - 1);
#endif // OPTI
    }
}

/*
WIP
・ちょっぴり速くなった
・サウンドが綺麗になった
・画面のセンタリング
・フレームスキップ(時間同期すると遅いので本当のスキップのみ)
・SRAMセーブを有効
・フレームスキップ時にハングアップする不具合の修正
・画面隅が欠ける不具合の修正

http://www.geocities.jp/pasofami77/chip/sfcchip
*/
