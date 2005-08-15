#include <stdlib.h>
#include "font.h"
#include "gfx.h"

unsigned char *fontready;

#define FONTWIDTH 6
#define FONTHEIGHT 13

unsigned char fontdata[96][13]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x04,0x00,0x00,0x00,
0x00,0x0a,0x0a,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x0a,0x0a,0x1f,0x0a,0x1f,0x0a,0x0a,0x00,0x00,0x00,0x00,
0x00,0x04,0x1e,0x05,0x05,0x0e,0x14,0x14,0x0f,0x04,0x00,0x00,0x00,
0x00,0x12,0x15,0x0a,0x08,0x04,0x02,0x0a,0x15,0x09,0x00,0x00,0x00,
0x00,0x02,0x05,0x05,0x02,0x05,0x19,0x09,0x16,0x00,0x00,0x00,0x00,
0x00,0x0c,0x04,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x08,0x04,0x04,0x02,0x02,0x02,0x04,0x04,0x08,0x00,0x00,0x00,
0x00,0x02,0x04,0x04,0x08,0x08,0x08,0x04,0x04,0x02,0x00,0x00,0x00,
0x00,0x00,0x04,0x15,0x1f,0x0e,0x1f,0x15,0x04,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x04,0x04,0x1f,0x04,0x04,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x04,0x02,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x0e,0x04,0x00,0x00,
0x00,0x10,0x10,0x08,0x08,0x04,0x02,0x02,0x01,0x01,0x00,0x00,0x00,
0x00,0x04,0x0a,0x11,0x11,0x11,0x11,0x11,0x0a,0x04,0x00,0x00,0x00,
0x00,0x04,0x06,0x05,0x04,0x04,0x04,0x04,0x04,0x1f,0x00,0x00,0x00,
0x00,0x0e,0x11,0x11,0x10,0x08,0x04,0x02,0x01,0x1f,0x00,0x00,0x00,
0x00,0x1f,0x10,0x08,0x04,0x0e,0x10,0x10,0x11,0x0e,0x00,0x00,0x00,
0x00,0x08,0x08,0x0c,0x0a,0x0a,0x09,0x1f,0x08,0x08,0x00,0x00,0x00,
0x00,0x1f,0x01,0x01,0x0d,0x13,0x10,0x10,0x11,0x0e,0x00,0x00,0x00,
0x00,0x0e,0x11,0x01,0x01,0x0f,0x11,0x11,0x11,0x0e,0x00,0x00,0x00,
0x00,0x1f,0x10,0x08,0x08,0x04,0x04,0x02,0x02,0x02,0x00,0x00,0x00,
0x00,0x0e,0x11,0x11,0x11,0x0e,0x11,0x11,0x11,0x0e,0x00,0x00,0x00,
0x00,0x0e,0x11,0x11,0x11,0x1e,0x10,0x10,0x11,0x0e,0x00,0x00,0x00,
0x00,0x00,0x00,0x04,0x0e,0x04,0x00,0x00,0x04,0x0e,0x04,0x00,0x00,
0x00,0x00,0x00,0x04,0x0e,0x04,0x00,0x00,0x0c,0x04,0x02,0x00,0x00,
0x00,0x10,0x08,0x04,0x02,0x01,0x02,0x04,0x08,0x10,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1f,0x00,0x00,0x1f,0x00,0x00,0x00,0x00,0x00,
0x00,0x01,0x02,0x04,0x08,0x10,0x08,0x04,0x02,0x01,0x00,0x00,0x00,
0x00,0x0e,0x11,0x11,0x10,0x08,0x04,0x04,0x00,0x04,0x00,0x00,0x00,
0x00,0x0e,0x11,0x11,0x19,0x15,0x15,0x0d,0x01,0x1e,0x00,0x00,0x00,
0x00,0x04,0x0a,0x11,0x11,0x11,0x1f,0x11,0x11,0x11,0x00,0x00,0x00,
0x00,0x0f,0x12,0x12,0x12,0x0e,0x12,0x12,0x12,0x0f,0x00,0x00,0x00,
0x00,0x0e,0x11,0x01,0x01,0x01,0x01,0x01,0x11,0x0e,0x00,0x00,0x00,
0x00,0x0f,0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x0f,0x00,0x00,0x00,
0x00,0x1f,0x01,0x01,0x01,0x0f,0x01,0x01,0x01,0x1f,0x00,0x00,0x00,
0x00,0x1f,0x01,0x01,0x01,0x0f,0x01,0x01,0x01,0x01,0x00,0x00,0x00,
0x00,0x0e,0x11,0x01,0x01,0x01,0x19,0x11,0x11,0x0e,0x00,0x00,0x00,
0x00,0x11,0x11,0x11,0x11,0x1f,0x11,0x11,0x11,0x11,0x00,0x00,0x00,
0x00,0x0e,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x0e,0x00,0x00,0x00,
0x00,0x1c,0x08,0x08,0x08,0x08,0x08,0x08,0x09,0x06,0x00,0x00,0x00,
0x00,0x11,0x11,0x09,0x05,0x03,0x05,0x09,0x11,0x11,0x00,0x00,0x00,
0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x1f,0x00,0x00,0x00,
0x00,0x11,0x11,0x1b,0x15,0x15,0x11,0x11,0x11,0x11,0x00,0x00,0x00,
0x00,0x11,0x13,0x13,0x15,0x15,0x19,0x19,0x11,0x11,0x00,0x00,0x00,
0x00,0x0e,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x0e,0x00,0x00,0x00,
0x00,0x0f,0x11,0x11,0x11,0x0f,0x01,0x01,0x01,0x01,0x00,0x00,0x00,
0x00,0x0e,0x11,0x11,0x11,0x11,0x11,0x11,0x15,0x0e,0x10,0x00,0x00,
0x00,0x0f,0x11,0x11,0x11,0x0f,0x05,0x09,0x11,0x11,0x00,0x00,0x00,
0x00,0x0e,0x11,0x01,0x01,0x0e,0x10,0x10,0x11,0x0e,0x00,0x00,0x00,
0x00,0x1f,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,
0x00,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x0e,0x00,0x00,0x00,
0x00,0x11,0x11,0x11,0x11,0x0a,0x0a,0x0a,0x04,0x04,0x00,0x00,0x00,
0x00,0x11,0x11,0x11,0x11,0x15,0x15,0x15,0x1b,0x11,0x00,0x00,0x00,
0x00,0x11,0x11,0x0a,0x0a,0x04,0x0a,0x0a,0x11,0x11,0x00,0x00,0x00,
0x00,0x11,0x11,0x0a,0x0a,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,
0x00,0x1f,0x10,0x08,0x08,0x04,0x02,0x02,0x01,0x1f,0x00,0x00,0x00,
0x00,0x0e,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x0e,0x00,0x00,0x00,
0x00,0x01,0x01,0x02,0x02,0x04,0x08,0x08,0x10,0x10,0x00,0x00,0x00,
0x00,0x0e,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x0e,0x00,0x00,0x00,
0x00,0x04,0x0a,0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x00,0x00,
0x00,0x0c,0x08,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0e,0x10,0x1e,0x11,0x11,0x1e,0x00,0x00,0x00,
0x00,0x01,0x01,0x01,0x0f,0x11,0x11,0x11,0x11,0x0f,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0e,0x11,0x01,0x01,0x11,0x0e,0x00,0x00,0x00,
0x00,0x10,0x10,0x10,0x1e,0x11,0x11,0x11,0x11,0x1e,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0e,0x11,0x1f,0x01,0x11,0x0e,0x00,0x00,0x00,
0x00,0x0c,0x12,0x02,0x02,0x0f,0x02,0x02,0x02,0x02,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0e,0x11,0x11,0x11,0x1e,0x10,0x11,0x0e,0x00,
0x00,0x01,0x01,0x01,0x0d,0x13,0x11,0x11,0x11,0x11,0x00,0x00,0x00,
0x00,0x00,0x04,0x00,0x06,0x04,0x04,0x04,0x04,0x0e,0x00,0x00,0x00,
0x00,0x00,0x08,0x00,0x0c,0x08,0x08,0x08,0x08,0x09,0x09,0x06,0x00,
0x00,0x01,0x01,0x01,0x09,0x05,0x03,0x05,0x09,0x11,0x00,0x00,0x00,
0x00,0x06,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x0e,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0b,0x15,0x15,0x15,0x15,0x11,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0d,0x13,0x11,0x11,0x11,0x11,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0e,0x11,0x11,0x11,0x11,0x0e,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0f,0x11,0x11,0x11,0x0f,0x01,0x01,0x01,0x00,
0x00,0x00,0x00,0x00,0x1e,0x11,0x11,0x11,0x1e,0x10,0x10,0x10,0x00,
0x00,0x00,0x00,0x00,0x0d,0x13,0x01,0x01,0x01,0x01,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0e,0x11,0x06,0x08,0x11,0x0e,0x00,0x00,0x00,
0x00,0x00,0x02,0x02,0x0f,0x02,0x02,0x02,0x12,0x0c,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x11,0x11,0x11,0x11,0x19,0x16,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x11,0x11,0x11,0x0a,0x0a,0x04,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x11,0x11,0x15,0x15,0x15,0x0a,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x11,0x0a,0x04,0x04,0x0a,0x11,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x11,0x11,0x11,0x19,0x16,0x10,0x11,0x0e,0x00,
0x00,0x00,0x00,0x00,0x1f,0x08,0x04,0x02,0x01,0x1f,0x00,0x00,0x00,
0x00,0x18,0x04,0x04,0x04,0x03,0x04,0x04,0x04,0x18,0x00,0x00,0x00,
0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,
0x00,0x03,0x04,0x04,0x04,0x18,0x04,0x04,0x04,0x03,0x00,0x00,0x00,
0x00,0x12,0x15,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f};

unsigned char *fonts[256];

void initfont(void)
{
unsigned short white,black,red;
int i,j,k;
unsigned char *p;
unsigned short *ps,*ps2;

	white=maprgb(255,255,255);
	black=maprgb(0,0,0);
	red=maprgb(255,0,0);
	fontready=malloc(2*96*6*2*13);
	if(!fontready) return;
	p=&fontdata[0][0];
	ps=(void *)fontready;
	ps2=(void *)(fontready+96*6*2*13);
	for(i=0;i<96*13;++i)
	{
		k=*p++;
		for(j=0;j<6;++j)
		{
			if(k&0x01)
			{
				*ps++=white;
				*ps2++=black;
			}
			else
			{
//				*ps++=black;
//				*ps2++=red;
*ps++=0;
*ps2++=0;
			}
			k>>=1;
		}
	}
	for(i=0;i<256;++i)
	{
		j=i&0x7f;
		if(j<0x20) j=0;
		else j-=0x20;
		if(i&0x80) j+=96;
		fonts[i]=fontready+j*6*13*2;
	}
}
void drawcharxy(unsigned int x,unsigned int y,unsigned char c)
{
unsigned short *ps,*pd;
int i,j,k;

#ifndef PSP
	pd=(void *)(videomem+(y*stride)+(x<<1));
#else
	pd=(void *)(videomem+(y*stride)+x);
#endif
	ps=(void *)fonts[c];

	for(j=0;j<13;++j)
	{
		for(i=0;i<FONTWIDTH;++i)
		{
			if(k=ps[i])
				pd[i]=k;
		}
		pd+=stride;
		ps+=12;
	}
}

void drawprintfxy(unsigned int x,unsigned int y,char *str,...)
{
char tbuff[256],*s,ch;
	vsprintf(tbuff,str,&str+1);
	s=tbuff;
	while(ch=*s++)
	{
		drawcharxy(x,y,ch);
		x+=FONTWIDTH;
	}
}
