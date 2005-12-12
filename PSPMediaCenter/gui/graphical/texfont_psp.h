
typedef struct {
  unsigned int width;
  unsigned int height;
  unsigned int pixelmode;
  unsigned int *imageptr;
  unsigned int *clutptr;
  // these below only used for the actual font
  unsigned int charrows;
  unsigned int charwidth;
  unsigned int charheight;
  unsigned int originx;
  unsigned int originy;
} font_def;

typedef struct {
  unsigned int x1,x2,y1,y2;
} font_rect;

typedef struct {
	unsigned short u, v;
	unsigned short color;
	short x, y, z;
} font_vertex;


void texfont_init();
void texfont_setimages(int mode,unsigned int width,unsigned int height, unsigned int pixelmode, unsigned int *imageptr, unsigned int *clutptr);
void texfont_setmetric(unsigned int charwidth, unsigned int charheight, unsigned int charrows);
void texfont_setorigin(unsigned int originx,unsigned int originy);
void texfont_setcurtex(font_def *fontptr);
void texfont_selectbgtex();
void texfont_drawrect(font_rect *pos, font_rect *tex);
void texfont_selectfonttex();
void texfont_drawchar(unsigned int x, unsigned int y, char *charptr);
void texfont_drawstring(unsigned int x, unsigned int y, char *stringptr);
