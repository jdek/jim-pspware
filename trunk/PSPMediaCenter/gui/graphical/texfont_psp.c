
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <pspgu.h>

#include "texfont_psp.h"

static font_def *currentfontptr = 0;
static font_def *currentbgptr = 0;

void texfont_init()
{ //  for now just allocated the two structures
  currentfontptr = (font_def *)malloc(sizeof(font_def));
  currentbgptr = (font_def *)malloc(sizeof(font_def));
}

void texfont_setimages(int mode,unsigned int width,unsigned int height, unsigned int pixelmode, unsigned int *imageptr, unsigned int *clutptr)
{
  font_def *texptr = 0;
  if (mode == 0) { // font texture
    texptr = currentfontptr;
  } else { // background texture
    texptr = currentbgptr;
  }
  // Now set the values
  if (texptr != 0) {
    texptr->width = width;
    texptr->height = height;
    texptr->pixelmode = pixelmode;
    texptr->imageptr = imageptr;
    texptr->clutptr = clutptr;
    texptr->originx = texptr->originy = 0;
    texptr->charrows = texptr->charwidth = texptr->charheight = 0;
  }
}
void texfont_setmetric(unsigned int charwidth, unsigned int charheight, unsigned int charrows)
{
  currentfontptr->charwidth = charwidth;
  currentfontptr->charheight = charheight;
  currentfontptr->charrows = charrows;
}

void texfont_setorigin(unsigned int originx,unsigned int originy)
{
  currentfontptr->originx = originx;
  currentfontptr->originy = originy;
}

void texfont_setcurtex(font_def *fontptr)
{
  sceGuTexMode(fontptr->pixelmode,0,0,0);
  sceGuTexImage(0,fontptr->width,fontptr->height,fontptr->width,fontptr->imageptr);
  sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGB);
  sceGuTexFilter(GU_NEAREST,GU_NEAREST);
  sceGuTexScale(1.0f/(float)fontptr->width,1.0f/(float)fontptr->height); // scale UVs to 0..1
  sceGuTexOffset(0.0f,0.0f);
  sceGuAmbientColor(0xffffffff);
  sceGuEnable(GU_TEXTURE_2D);
}

void texfont_selectbgtex()
{
  texfont_setcurtex(currentbgptr);
}

void texfont_drawrect(font_rect *pos, font_rect *tex)
{
  font_vertex* vertices;
	vertices = (font_vertex *)sceGuGetMemory(2 * sizeof(font_vertex));

  vertices[0].u = tex->x1; vertices[0].v = tex->y1;
	vertices[0].color = 0;
	vertices[0].x = pos->x1; vertices[0].y = pos->y1; vertices[0].z = 0;

	vertices[1].u = tex->x2; vertices[1].v = tex->y2;
	vertices[1].color = 0;
	vertices[1].x = pos->x2; vertices[1].y = pos->y2; vertices[1].z = 0;

	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
}

void texfont_selectfonttex()
{
  texfont_setcurtex(currentfontptr);
}

void texfont_drawchar(unsigned int x, unsigned int y, char *charptr)
{
  unsigned char ch = *charptr - ' '; // remove the ctrl chars
  font_rect pos,tex;
  // calculate the screen rectangle
  pos.x1 = currentfontptr->originx + (x * currentfontptr->charwidth);
  pos.y1 = currentfontptr->originy + (y * currentfontptr->charheight);
  pos.x2 = pos.x1 + currentfontptr->charwidth - 1;
  pos.y2 = pos.y1 + currentfontptr->charheight - 1;
  // calculate the texture rectangle, for the character
  tex.x1 = (ch % currentfontptr->charrows) * currentfontptr->charwidth;   //  xpos , pixels wise
  tex.y1 = (ch / currentfontptr->charrows) * currentfontptr->charheight;  //  ypos , pixels wise
  tex.x2 = tex.x1 + currentfontptr->charwidth - 1;
  tex.y2 = tex.y1 + currentfontptr->charheight - 1;

  // draw the rectangle
  texfont_drawrect(&pos,&tex);
}

void texfont_drawstring(unsigned int x, unsigned int y, char *stringptr)
{
  char *ptr = stringptr;
  unsigned int x2 = x;
  while (*ptr != 0) {
    texfont_drawchar(x2,y,ptr);
    ptr++;
    x2++;
  }
}