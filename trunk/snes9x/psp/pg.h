// primitive graphics

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "psp.h"

/*
// Use the Snes9x BUILD_PIXEL function, which will take the selected
// pixel format into account.
#define RGB(r,g,b) BUILD_PIXEL(r,g,b) | 0x8000
*/
#define RGB(r,g,b) ((((b>>3) & 0x1F)<<10)|(((g>>3) & 0x1F)<<5)|(((r>>3) & 0x1F)<<0)|0x8000)


extern u32 new_pad, now_pad, old_pad;
extern SceCtrlData paddata;

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define PIXELSIZE     1        // in short
#define LINESIZE      512      // in short
#define FRAMESIZE     0x44000  // in byte

#define CMAX_X 60
#define CMAX_Y 34
#define CMAX2_X 30
#define CMAX2_Y 17
#define CMAX4_X 15
#define CMAX4_Y 8

void pgInit(void);
void pgWaitV(void);
void pgWaitVn(unsigned long count);
void pgScreenFrame(long mode,long frame);
void pgScreenSync(void);
void pgScreenFlip(void);
void pgScreenFlipV(void);
void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgFillvram(unsigned long color);

void pgPrintBG(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrintWF(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgFillBoxWF(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);


void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d);

void pgBitBltFit(unsigned short *d, int Height);
void pgBitBltFull(unsigned long *d, int Height);
void pgBitBltFullFit(unsigned short *d, int Height);

void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag);
void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);
void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);
void mh_print(int x,int y,const unsigned char *str,int col);
unsigned char *pgGetVramAddr(unsigned long x,unsigned long y);
void pgRenderTex(char *tex, int width, int height, int x, int y, int xscale, int yscale, int xres, int yres);

// add by J
#define D_text_all_MAX  3200
// 1行の文字数
#define D_text_MAX      256

//      ダイアログの外枠カラー
#define D_dialog_out_color RGB( 255,   5,   5 )
//      ダイアログの内側カラー
#define D_dialog_in_color  RGB(  50,  50, 115 )
//      ダイアログのタイトル外枠カラー
#define D_dialog_out_color1 RGB( 128,   5,   5 )
//      ダイアログのタイトル内側カラー
#define D_dialog_in_color1  RGB(  50,  50, 180 )
//      文字カラー
#define D_dialog_string_shadow 0x0000
#define D_dialog_message_color 0xffff
//      タイトルカラー
#define D_dialog_title_color RGB(  255, 255,   0 )

void message_dialog( int dialog_pos_x, int dialog_pos_y, const char *title, const char *message );
int getStringLineCount( const char *string );
int getStringLine( char *line_string, const char *string, int *search_cnt, int line_string_max_length );

void pgMain(void);

void readpad(void);

void pgcPuthex2(const unsigned long s);
void pgcPuthex8(const unsigned long s);

static inline void __memcpy4a(unsigned long *d, unsigned long *s, unsigned long c)
{
	unsigned long wk,counter;

	asm volatile (
		"		.set	push"				"\n"
		"		.set	noreorder"			"\n"

		"		move	%1,%4 "				"\n"
		"1:		lw		%0,0(%3) "			"\n"
		"		addiu	%1,%1,-1 "			"\n"
		"		addiu	%3,%3,4 "			"\n"
		"		sw		%0,0(%2) "			"\n"
		"		bnez	%1,1b "				"\n"
		"		addiu	%2,%2,4 "			"\n"

		"		.set	pop"				"\n"

			:	"=&r" (wk),		// %0
				"=&r" (counter)	// %1
			:	"r" (d),		// %2
				"r" (s),		// %3
				"r" (c)			// %4
	);
}

#define __USE_MIPS32R2__

static inline void cpy2x(unsigned long *d, unsigned long cc)
{
#ifdef __USE_MIPS32R2__
	unsigned long wk0;
	asm volatile (
		"		.set	push"				"\n"
		"		.set	noreorder"			"\n"

		"		.set	mips32r2"			"\n"
		
		"		srl		%0,%2,16"			"\n"
		"		ins 	%2,%2,16,16"		"\n"
		"		sw		%2,0(%1)"			"\n"
		"		ins 	%0,%0,16,16"		"\n"
		"		sw		%0,4(%1)"			"\n"
		
		"		.set	pop"				"\n"
		
			:	"=&r" (wk0)		// %0
			:	"r" (d),		// %1
				"r" (cc)		// %2
	);
#else
	*d      = (cc&0x0000ffff)|(cc<<16);
	*(d+1)  = (cc>>16)|(cc&0xffff0000);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

