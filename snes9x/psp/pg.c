// primitive graphics

#include "3d.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspge.h>

#include "pg.h"

#include "font.c"
#include "fontNaga10.c"

#include "port.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//variables
char *pg_vramtop = NULL;// = (char *)0x4000000;

long pg_screenmode;
long pg_showframe;
long pg_drawframe;
unsigned long pgc_csr_x[2], pgc_csr_y[2];
unsigned long pgc_fgcolor[2], pgc_bgcolor[2];
char pgc_fgdraw[2], pgc_bgdraw[2];
char pgc_mag[2];

extern bool8 bGUIMode;

void pgWaitVn(unsigned long count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV(void)
{
	sceDisplayWaitVblankStart();
}


unsigned char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	if ((! PSP_Settings.bUseGUBlit) || bGUIMode) {
		return (unsigned char *)pg_vramtop+(pg_drawframe?FRAMESIZE:0)+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
//	return pg_vramtop+(pg_drawframe?FRAMESIZE:0)+x*PIXELSIZE*2+y*LINESIZE*2;//+0x40000000;	//変わらないらしい
	} else {
		return (unsigned char *)pg_vramtop+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
	}
}


void pgInit(void)
{
	if (PSP_Settings.bUseGUBlit && (! bGUIMode)) {
		sceGuInit();

		S9xSceGUInit2 ();

		pg_vramtop = (char *) (0x40000000 | sceGeEdramGetAddr());
		sceDisplaySetFrameBuf(pg_vramtop,LINESIZE,1,1);
	} else {
		S9xSceGUDeinit ();
	}
	
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);
	pgScreenFrame(0,0);

	if ((! PSP_Settings.bUseGUBlit) || bGUIMode)
		pg_vramtop = (char *)0x4000000;

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

void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX2_X && y<CMAX2_Y) {
		pgPutChar(x*16,y*16,color,0,*str,1,0,2);
		str++;
		x++;
		if (x>=CMAX2_X) {
			x=0;
			y++;
		}
	}
}


void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX4_X && y<CMAX4_Y) {
		pgPutChar(x*32,y*32,color,0,*str,1,0,4);
		str++;
		x++;
		if (x>=CMAX4_X) {
			x=0;
			y++;
		}
	}
}

void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for(i=x1; i<=x2; i++){
		((unsigned short *)vptr0)[i*PIXELSIZE + y1*LINESIZE] = color;
		((unsigned short *)vptr0)[i*PIXELSIZE + y2*LINESIZE] = color;
	}
	for(i=y1; i<=y2; i++){
		((unsigned short *)vptr0)[x1*PIXELSIZE + i*LINESIZE] = color;
		((unsigned short *)vptr0)[x2*PIXELSIZE + i*LINESIZE] = color;
	}
}

void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = color;
		}
	}
}

void pgFillBoxWF(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = color;
			((unsigned short *)(vptr0 + (pg_drawframe?-1:1)*FRAMESIZE))[j*PIXELSIZE + i*LINESIZE] = color;
		}
	}
}

void pgFillvram(unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<FRAMESIZE/2; i++) {
		*(unsigned short *)vptr0=color;
		vptr0+=PIXELSIZE*2;
	}
}

void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;

	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=PIXELSIZE*2;
				}
				dd++;
			}
			vptr0+=LINESIZE*2;
		}
		d+=w;
	}

}

//Parallel blend
static inline unsigned long PBlend(unsigned long c0, unsigned long c1)
{
	return (c0 & c1) + (((c0 ^ c1) & 0x7bde7bde) >> 1);
}

//Full
void pgBitBltFull(unsigned long *d, int Height)
{
	unsigned long *vptr0;
	unsigned long *vptr;
	unsigned long *dl;
	int x, y, dy, ddy;
	
	if (Height==239) {
		ddy = 14225;
	} else {
		Height = 224;
		ddy = 21874;
	}
	
	vptr0 = (unsigned long *)pgGetVramAddr(0, 0);
	dy = 0;
	
	for (y = 0; y < Height; y++) {
		vptr = vptr0;
		dl = (unsigned long *)d;
		dl += 2;
		for (x = 8; x < 248; x+=2) {
			cpy2x(vptr, *dl++);
			vptr+=2;
		}
		vptr0 += (LINESIZE/2);
		d += 256;
		vptr = vptr0;
		dl = (unsigned long *)d;
		dl += 2;
		dy += ddy;
		if (dy >= 100000) {
			dy-=100000;
			for (x = 8; x < 248; x+=2) {
				cpy2x(vptr, PBlend(*(dl-256), *dl++));
				vptr+=2;
			}
			vptr0 += (LINESIZE/2);
		}
	}
}

void pgBitBltFit(unsigned short *d, int Height)
{
	unsigned short *vptr0;
	unsigned short *vptr;
	unsigned short *dl;
	unsigned short rgb, rgb2;
	int x, y, dx, dy, ddy;
	
	if (Height==239) {
		ddy = 14225;
		pgFillBox( 84, 0, 93, 271, 0 );
		pgFillBox( 386, 0, 395, 271, 0 );
		vptr0 = (unsigned short *)pgGetVramAddr(94, 0);
	} else {
		Height = 224;
		ddy = 21874;
		vptr0 = (unsigned short *)pgGetVramAddr(84, 0);
	}
	
	dy = 0;
	
	for (y = 0; y < Height; y++) {
		vptr = vptr0;
		dx = 0;
		dl = (unsigned short *)d;
		for (x = 0; x < 256; x++) {
			*vptr = *dl++;
			vptr++;
			dx += ddy;
			if (dx >= 100000) {
				dx-=100000;
				*vptr = (*(dl-1) & *dl) + (((*(dl-1) ^ *dl) & 0x7bde ) >> 1);
				vptr++;
			}
		}
		vptr0 += LINESIZE;
		d += 512;
		vptr = vptr0;
		dx = 0;
		dl = (unsigned short *)d;
		dy += ddy;
		if (dy >= 100000) {
			dy-=100000;
			for (x = 0; x < 256; x++) {
				*vptr = (*(dl-512) & *dl) + (((*(dl-512) ^ *dl) & 0x7bde ) >> 1);
				dl++;
				vptr++;
				dx += ddy;
				if (dx >= 100000) {
					dx-=100000;
					rgb = (*(dl-1) & *dl) + (((*(dl-1) ^ *dl) & 0x7bde ) >> 1);
					rgb2 = (*(dl-513) & *(dl-512)) + (((*(dl-513) ^ *(dl-512)) & 0x7bde ) >> 1);
					*vptr = (rgb & rgb2) + (((rgb ^ rgb2) & 0x7bde ) >> 1);
					vptr++;
				}
			}
			vptr0 += LINESIZE;
		}
	}
}

void pgBitBltFullFit(unsigned short *d, int Height)
{
	unsigned short *vptr0;
	unsigned short *vptr;
	unsigned short *dl;
	unsigned short rgb, rgb2;
	int x, y, dx, dy, ddy;
	
	if (Height==239) {
		ddy = 14225;
	} else {
		Height = 224;
		ddy = 21874;
	}
	
	vptr0 = (unsigned short *)pgGetVramAddr(0, 0);
	dy = 0;
	
	for (y = 0; y < Height; y++) {
		vptr = vptr0;
		dx = 0;
		dl = (unsigned short *)d;
		for (x = 0; x < 256; x++) {
			*vptr = *dl++;
			vptr++;
			dx += 875;
			if (dx >= 1000) {
				dx-=1000;
				*vptr = (*(dl-1) & *dl) + (((*(dl-1) ^ *dl) & 0x7bde ) >> 1);
				vptr++;
			}
		}
		vptr0 += LINESIZE;
		d += 512;
		vptr = vptr0;
		dx = 0;
		dl = (unsigned short *)d;
		dy += ddy;
		if (dy >= 100000) {
			dy-=100000;
			for (x = 0; x < 256; x++) {
				*vptr = (*(dl-512) & *dl) + (((*(dl-512) ^ *dl) & 0x7bde ) >> 1);
				dl++;
				vptr++;
				dx += 875;
				if (dx >= 1000) {
					dx-=1000;
					rgb = (*(dl-1) & *dl) + (((*(dl-1) ^ *dl) & 0x7bde ) >> 1);
					rgb2 = (*(dl-513) & *(dl-512)) + (((*(dl-513) ^ *(dl-512)) & 0x7bde ) >> 1);
					*vptr = (rgb & rgb2) + (((rgb ^ rgb2) & 0x7bde ) >> 1);
					vptr++;
				}
			}
			vptr0 += LINESIZE;
		}
	}
}

void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

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
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
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

void pgPutCharWF(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

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
						if (drawfg) *(unsigned short *)(vptr + (pg_drawframe?-1:1)*FRAMESIZE)=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
						if (drawbg) *(unsigned short *)(vptr + (pg_drawframe?-1:1)*FRAMESIZE)=bgcolor;
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

void pgPrintBG(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,1,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

void pgPrintWF(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutCharWF(x*8,y*8,color,0,*str,1,1,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

void pgScreenFrame(long mode,long frame)
{
	if (PSP_Settings.bUseGUBlit && (! bGUIMode))
		return;

	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
		pg_drawframe=frame;
		sceDisplaySetFrameBuf((char *)pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf((char *)pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
}

struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};

void pgScreenSync(void)
{
	sceGuSync(0,0);
}

void pgScreenFlip()
{
	if ((! PSP_Settings.bUseGUBlit) || bGUIMode) {
		pg_showframe=(pg_showframe?0:1);
		pg_drawframe=(pg_drawframe?0:1);
		sceDisplaySetFrameBuf((char *)pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,0);
	} else {
		sceGuSync(0,0);
		sceGuSwapBuffers();
	}
}


void pgScreenFlipV(void)
{
	if ((! PSP_Settings.bUseGUBlit) || bGUIMode) {
		pgWaitV();
		pgScreenFlip();
	} else {
		sceGuSync(0,0);
		pgWaitV();
		sceGuSwapBuffers();
	}
}

// by kwn
void Draw_Char_Hankaku(int x,int y,const unsigned char c,int col) {
	unsigned short *vr;
	unsigned char  *fnt;
	unsigned char  pt;
	unsigned char ch;
	int x1,y1;

	ch = c;

	// mapping
	if (ch<0x20)
		ch = 0;
	else if (ch<0x80)
		ch -= 0x20;
	else if (ch<0xa0)
		ch = 0;
	else
		ch -= 0x40;

	fnt = (unsigned char *)&hankaku_font10[ch*10];

	// draw
	vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<10;y1++) {
		pt = *fnt++;
		for(x1=0;x1<5;x1++) {
			if (pt & 1)
				*vr = col;
			vr++;
			pt = pt >> 1;
		}
		vr += LINESIZE-5;
	}
}

// by kwn
void Draw_Char_Zenkaku(int x,int y,const unsigned char u,unsigned char d,int col) {
	// ELISA100.FNTに存在しない文字
	const unsigned short font404[] = {
		0xA2AF, 11,
		0xA2C2, 8,
		0xA2D1, 11,
		0xA2EB, 7,
		0xA2FA, 4,
		0xA3A1, 15,
		0xA3BA, 7,
		0xA3DB, 6,
		0xA3FB, 4,
		0xA4F4, 11,
		0xA5F7, 8,
		0xA6B9, 8,
		0xA6D9, 38,
		0xA7C2, 15,
		0xA7F2, 13,
		0xA8C1, 720,
		0xCFD4, 43,
		0xF4A5, 1030,
		0,0
	};
	unsigned short *vr;
	unsigned short *fnt;
	unsigned short pt;
	int x1,y1;

	unsigned long n;
	unsigned short code;
	int i, j;

	// SJISコードの生成
	code = u;
	code = (code<<8) + d;

	// SJISからEUCに変換
	if(code >= 0xE000) code-=0x4000;
	code = ((((code>>8)&0xFF)-0x81)<<9) + (code&0x00FF);
	if((code & 0x00FF) >= 0x80) code--;
	if((code & 0x00FF) >= 0x9E) code+=0x62;
	else code-=0x40;
	code += 0x2121 + 0x8080;

	// EUCから恵梨沙フォントの番号を生成
	n = (((code>>8)&0xFF)-0xA1)*(0xFF-0xA1)
		+ (code&0xFF)-0xA1;
	j=0;
	while(font404[j]) {
		if(code >= font404[j]) {
			if(code <= font404[j]+font404[j+1]-1) {
				n = -1;
				break;
			} else {
				n-=font404[j+1];
			}
		}
		j+=2;
	}
	fnt = (unsigned short *)&zenkaku_font10[n*10];

	// draw
	vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<10;y1++) {
		pt = *fnt++;
		for(x1=0;x1<10;x1++) {
			if (pt & 1)
				*vr = col;
			vr++;
			pt = pt >> 1;
		}
		vr += LINESIZE-10;
	}
}

// by kwn
void mh_print(int x,int y,const unsigned char *str,int col) {
	unsigned char ch = 0,bef = 0;

	while(*str != 0) {
		ch = *str++;
		if (bef!=0) {
			Draw_Char_Zenkaku(x,y,bef,ch,col);
			x+=10;
			bef=0;
		} else {
			if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
				bef = ch;
			} else {
				Draw_Char_Hankaku(x,y,ch,col);
				x+=5;
			}
		}
	}
}

u32 new_pad;
u32 old_pad;
u32 now_pad;
SceCtrlData paddata;

void readpad(void)
{
	static int n=0;
	SceCtrlData paddata;

	sceCtrlReadBufferPositive (&paddata, 1);
	// kmg
	// Analog pad state
	if (paddata.Ly == 0xff) paddata.Buttons=PSP_CTRL_DOWN;  // DOWN
	if (paddata.Ly == 0x00) paddata.Buttons=PSP_CTRL_UP;    // UP
	if (paddata.Lx == 0x00) paddata.Buttons=PSP_CTRL_LEFT;  // LEFT
	if (paddata.Lx == 0xff) paddata.Buttons=PSP_CTRL_RIGHT; // RIGHT

	now_pad = paddata.Buttons;
	new_pad = now_pad & ~old_pad;
	if(old_pad==now_pad){
		n++;
		if(n>=25){
			new_pad=now_pad;
			n = 20;
		}
	}else{
		n=0;
		old_pad = now_pad;
	}
}

/******************************************************************************/


void pgcLocate(unsigned long x, unsigned long y)
{
	if (x>=CMAX_X) x=0;
	if (y>=CMAX_Y) y=0;
	pgc_csr_x[pg_drawframe?1:0]=x;
	pgc_csr_y[pg_drawframe?1:0]=y;
}


void pgcColor(unsigned long fg, unsigned long bg)
{
	pgc_fgcolor[pg_drawframe?1:0]=fg;
	pgc_bgcolor[pg_drawframe?1:0]=bg;
}


void pgcDraw(char drawfg, char drawbg)
{
	pgc_fgdraw[pg_drawframe?1:0]=drawfg;
	pgc_bgdraw[pg_drawframe?1:0]=drawbg;
}


void pgcSetmag(char mag)
{
	pgc_mag[pg_drawframe?1:0]=mag;
}

void pgcCls()
{
	pgFillvram(pgc_bgcolor[pg_drawframe]);
	pgcLocate(0,0);
}

void pgcPutchar_nocontrol(const char ch)
{
	pgPutChar(pgc_csr_x[pg_drawframe]*8, pgc_csr_y[pg_drawframe]*8, pgc_fgcolor[pg_drawframe], pgc_bgcolor[pg_drawframe], ch, pgc_fgdraw[pg_drawframe], pgc_bgdraw[pg_drawframe], pgc_mag[pg_drawframe]);
	pgc_csr_x[pg_drawframe]+=pgc_mag[pg_drawframe];
	if (pgc_csr_x[pg_drawframe]>CMAX_X-pgc_mag[pg_drawframe]) {
		pgc_csr_x[pg_drawframe]=0;
		pgc_csr_y[pg_drawframe]+=pgc_mag[pg_drawframe];
		if (pgc_csr_y[pg_drawframe]>CMAX_Y-pgc_mag[pg_drawframe]) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-pgc_mag[pg_drawframe];
//			pgMoverect(0,pgc_mag[pg_drawframe]*8,SCREEN_WIDTH,SCREEN_HEIGHT-pgc_mag[pg_drawframe]*8,0,0);
		}
	}
}

void pgcPutchar(const char ch)
{
	if (ch==0x0d) {
		pgc_csr_x[pg_drawframe]=0;
		return;
	}
	if (ch==0x0a) {
		if ((++pgc_csr_y[pg_drawframe])>=CMAX_Y) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-1;
//			pgMoverect(0,8,SCREEN_WIDTH,SCREEN_HEIGHT-8,0,0);
		}
		return;
	}
	pgcPutchar_nocontrol(ch);
}

void pgcPuthex2(const unsigned long s)
{
	char ch;
	ch=((s>>4)&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
	ch=(s&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
}


void pgcPuthex8(const unsigned long s)
{
	pgcPuthex2(s>>24);
	pgcPuthex2(s>>16);
	pgcPuthex2(s>>8);
	pgcPuthex2(s);
}

/******************************************************************************/

void pgiInit()
{
//	sceCtrlInit(0);
	sceCtrlSetSamplingMode(1);
}

/******************************************************************************/

void pgMain(void)
{
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);

	pgScreenFrame(0,1);
	pgcLocate(0,0);
	pgcColor(0xffff,0x0000);
	pgcDraw(1,1);
	pgcSetmag(1);
	pgScreenFrame(0,0);
	pgcLocate(0,0);
	pgcColor(0xffff,0x0000);
	pgcDraw(1,1);
	pgcSetmag(1);

	pgiInit();

	if (PSP_Settings.bUseGUBlit || bGUIMode)
		sceDisplaySetMode(1,SCREEN_WIDTH,SCREEN_HEIGHT);
}

// add by J
// stringからの1行取り出し
// char       *line_string            : search_char以降のテキスト
// const char *string                 : テキスト全部
// int        *search_cnt             : 読出し位置 -> 次の読出し位置を代入しますのでint型の変数を指定して下さい。(数字直接指定はだめです。)
// int         line_string_max_length : line_string のサイズ
// int         ret_char_cnt           : 1行分のテキスト文字数
// How can I live without you by pという名様
// こんなアルゴリズムでいいのでしょうか。教えてエロいひと
int getStringLine( char *line_string, const char *string, int *search_cnt, int line_string_max_length )
{
	int loop_cnt;
	int ret_char_cnt = 0;
	for ( loop_cnt = *search_cnt; loop_cnt < strlen( string ) ; loop_cnt++ ) {
		if ( string[loop_cnt] != '\n' ) {
			if ( ret_char_cnt < line_string_max_length -1 ) {
				line_string[ ret_char_cnt ] = string[ loop_cnt ];
				ret_char_cnt++;
			}
		}else {
			line_string[ ret_char_cnt ] = '\0';
			break;
		}
	}
	*search_cnt = loop_cnt+1;
	return ret_char_cnt;
}

// const char *string  : テキスト全部
// int         ret_int : 改行の数
int getStringLineCount( const char *string )
{
	int ret_int = 0;
	int loop_cnt;
	for ( loop_cnt = 0; loop_cnt < strlen( string ) ; loop_cnt++ ) {
		if ( string[loop_cnt] == '\n' ) ret_int++;
	}
	return ret_int;
}

// ダイアログ作成/表示ルーチン
// キー判定はこのルーチンを呼んだ後に自前で行なって下さい。
// int         dialog_pos_x : ダイアログ表示位置x座標
// int         dialog_pos_y : ダイアログ表示位置y座標
// const char *title        : ダイアログのタイトル 
// const char *message      : ダイアログに表示するメッセージ(最後に改行コードを一つ追加してね)

void message_dialog( int dialog_pos_x, int dialog_pos_y, const char *title, const char *message )
{
	int loop_cnt;
	//  ダイアログ横幅
	int dialog_whidth = strlen( title ) *5 +20;
	//  ダイアログ縦幅
	int dialog_height = getStringLineCount( message ) *10 +15;
	//  message_text_allの読込みカウンタ
	int message_text_read_cnt =  0;
	// メッセージテキストx・y座標
	int message_text_pos_x    = 10;
	int message_text_pos_y    =  1;
	// タイトル用文字列
	char title_string[ D_text_MAX ];
	int  titel_string_cnt = 0;

	// ダイアログ横幅判定(メッセージ内の文字数によってダイアログの横幅を変えます。)
	while( message_text_read_cnt < strlen( message ) ) {
		int   read_message_text_cnt = 0;
		char  read_message_text[ D_text_MAX ];
		read_message_text_cnt = getStringLine( read_message_text, message, &message_text_read_cnt, D_text_MAX );
		if ( titel_string_cnt < read_message_text_cnt ) titel_string_cnt = read_message_text_cnt;
	}
	// ダイアログタイトル文字列作成(メッセージ内の1行最大文字数より小さかったら' 'を文字数分追加)
	strcpy( title_string, title );
	for ( loop_cnt = strlen( title ); loop_cnt < titel_string_cnt; loop_cnt++ ) {
		if ( strlen( title_string ) < D_text_MAX-1 ) {
			strcat( title_string, " \0" );
		}else {
			strcat( title_string, "\0" );
			break;
		}
	}
	// title_stringの文字数でダイアログの横幅を決定
	dialog_whidth = strlen( title_string ) *5 +20;

	// 影？描画
	pgFillBox( dialog_pos_x +2, dialog_pos_y  +7, dialog_pos_x + dialog_whidth  +2, dialog_pos_y + dialog_height +2, D_dialog_string_shadow );

	// 外枠描画
	pgFillBox( dialog_pos_x    , dialog_pos_y +5, dialog_pos_x + dialog_whidth    , dialog_pos_y + dialog_height   , D_dialog_out_color     );

	// 内側描画
	pgFillBox( dialog_pos_x  +2, dialog_pos_y +7, dialog_pos_x + dialog_whidth  -2, dialog_pos_y + dialog_height -2, D_dialog_in_color      );

	// タイトル枠描画
	pgFillBox( dialog_pos_x +11, dialog_pos_y +1, dialog_pos_x + dialog_whidth  -9, dialog_pos_y +14               , D_dialog_string_shadow );
	pgFillBox( dialog_pos_x +10, dialog_pos_y   , dialog_pos_x + dialog_whidth -10, dialog_pos_y +13               , D_dialog_out_color     );
	pgFillBox( dialog_pos_x +11, dialog_pos_y +1, dialog_pos_x + dialog_whidth -11, dialog_pos_y +12               , D_dialog_in_color1     );
	// タイトル描画
	mh_print ( dialog_pos_x + message_text_pos_x +1, dialog_pos_y + message_text_pos_y +2, (unsigned char*)title_string, D_dialog_string_shadow );
	mh_print ( dialog_pos_x + message_text_pos_x   , dialog_pos_y + message_text_pos_y +1, (unsigned char*)title_string, D_dialog_title_color   );

	// メッセージ
	message_text_pos_y += 18;
	message_text_read_cnt =  0;
	while( message_text_read_cnt < strlen( message ) ) {
		char  read_message_text[ D_text_MAX ];
		int   read_message_text_cnt = 0;
		// 行読込み
		read_message_text_cnt = getStringLine( read_message_text, message, &message_text_read_cnt, D_text_MAX );
		if ( read_message_text_cnt > 0 ) {
			// メッセージ描画
			mh_print ( dialog_pos_x + message_text_pos_x +1, dialog_pos_y + message_text_pos_y +1, (unsigned char*)read_message_text, D_dialog_string_shadow );
			mh_print ( dialog_pos_x + message_text_pos_x   , dialog_pos_y + message_text_pos_y   , (unsigned char*)read_message_text, D_dialog_message_color );
		}
		message_text_pos_y += 10;
	}

	// ダイアログ表示
	if (! PSP_Settings.bUseGUBlit || bGUIMode)
		pgScreenFlipV();

	return; // voidだからいらないんだけどね
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

