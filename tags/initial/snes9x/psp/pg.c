// primitive graphics for Hello World sce
// gu blit code from blit.c by chp from ps2dev.org 

#include <pspkernel.h>
#include <pspgu.h>

#include "pg.h"

#include "font.c"
#include "fontNaga10.c"

#define SLICE_SIZE 64 // change this to experiment with different page-cache sizes

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//variables
char *pg_vramtop;
long pg_screenmode;
long pg_showframe;
long pg_drawframe;
unsigned long pgc_csr_x[2], pgc_csr_y[2];
unsigned long pgc_fgcolor[2], pgc_bgcolor[2];
char pgc_fgdraw[2], pgc_bgdraw[2];
char pgc_mag[2];

static unsigned int list[262144] __attribute__((aligned(16)));

void pgWaitVn(unsigned long count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV()
{
	sceDisplayWaitVblankStart();
}


char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return pg_vramtop+(x*PIXELSIZE*2)+(y*LINESIZE*2);
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
/*
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
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
*/
}

struct Vertex
{
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
};



void pgRenderTex(char *tex, int width, int height, int x, int y, int xscale, int yscale)
{
	unsigned int j;
	int slice_scale = ((float)xscale/(float)width)*(float)SLICE_SIZE;
	struct Vertex* vertices;

	sceGuStart(0,list);

	sceGuTexMode(GE_TPSM_5650,0,0,0);
	sceGuTexImage(0,width,height,width,tex);
	sceGuTexFunc(GE_TFX_REPLACE,0);
	sceGuTexFilter(GE_FILTER_LINEAR,GE_FILTER_LINEAR);
	sceGuTexScale(1,1);
	sceGuTexOffset(0,0);
	sceGuTexSync();
	sceGuAmbientColor(0xffffffff);

	// do a striped blit (takes the page-cache into account)

	for (j = 0; j < width; j += SLICE_SIZE, x += slice_scale)
	{
		vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = j; vertices[0].v = 0;
		vertices[0].color = 0;
		vertices[0].x = x; vertices[0].y = y; vertices[0].z = 0;
		vertices[1].u = j+SLICE_SIZE; vertices[1].v = height;
		vertices[1].color = 0;
		vertices[1].x = x+slice_scale; vertices[1].y = y+yscale; vertices[1].z = 0;

		sceGuDrawArray(GU_PRIM_SPRITES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5650,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),2,0,vertices);
	}

	sceGuFinish();
}

void pgScreenFlip()
{
	sceGuSync(0,0);
	sceGuSwapBuffers();
}

void pgScreenFlipV()
{
	pgWaitV();
	sceGuSync(0,0);
	sceGuSwapBuffers();
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

void readpad()
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

void pgMain()
{
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);

	sceGuInit();

	// setup
	sceGuStart(0,list);
	sceGuDrawBuffer(GE_PSM_5650,(void*)0,512);
	sceGuDispBuffer(480,272,(void*)0x88000,512);
	sceGuDepthBuffer((void*)0x110000,512);
	sceGuOffset(0,0);
	sceGuViewport(480/2,272/2,480,272);
	sceGuDepthRange(0xc350,0x2710);
	sceGuScissor(0,0,480,272);
	sceGuEnable(GU_STATE_SCISSOR);
	sceGuFrontFace(GE_FACE_CW);
	sceGuEnable(GU_STATE_TEXTURE);
	sceGuClear(GE_CLEAR_COLOR|GE_CLEAR_DEPTH);
	sceGuFinish();
	sceGuSync(0,0);

	pg_vramtop = (void *) (sceGeEdramGetAddr() | 0x40000000);
	sceDisplaySetFrameBuf(pg_vramtop,LINESIZE,0,1);

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

}

#ifdef __cplusplus
}
#endif /* __cplusplus */

