/****************************************************
  uo_Snes9x for PSP - Portable Super Nintendo Entertainment System (TM) emulator.

  SceGU render backend
   (c) Copyright 2005 Andon Coleman (andon@nothing-inc.com),
                      Jesper Svennevid, and various other PSP Dev. contributors.

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

#include <math.h>
#include "snes9x.h"
#include "3d.h"
#include "gfx.h"
#include "psp.h"
#include "ppu.h"
#include "pg.h"

#include <pspdisplay.h>

#ifdef USE_SCEGU

SceGUData SceGU;

extern "C" {

void S9xSceGUSwapBuffers (void);

bool8 S9xSceGUInit (void)
{
  sceGuInit ();

  sceDisplaySetMode (0, SCREEN_WIDTH, SCREEN_HEIGHT);

  S9xSceGUInit2 ();

  sceGuDrawBuffer (SceGU.pixel_format, (void *)0, SceGU.line_size);
  sceGuDisplay    (1);

  return (TRUE);
}

bool8 S9xSceGUInit2 (void)
{
  SceGU.line_size        = 512;
  SceGU.max_texture_size = 512;

  if (SceGU.max_texture_size >= 512)
  {
      SceGU.texture_size = 512;
//        SceGU.num_textures = 2; // See note in 3d.h
      SceGU.num_textures = 1;
  }
  else
  {
      SceGU.texture_size = SceGU.max_texture_size;
      SceGU.num_textures = 1;
  }

  // TODO
  ///////Settings.SceGUEnable = TRUE;


  // 2nd texture may be reserved for transparency in the future.
#if 0
  if (SceGU.num_textures == 2)
  {
  }
#endif

  // Use a 16-bit pixel format 5-bits for RGB and 1-bit for Alpha (unused)
  SceGU.texture_format = GU_PSM_5551;
  SceGU.pixel_format   = GU_PSM_5551;
  SceGU.ct             = GU_COLOR_5551;
  SceGU.tt             = GU_TEXTURE_16BIT;
  SceGU.mt             = GU_VERTEX_16BIT;
  SceGU.dm             = GU_TRANSFORM_2D;

  sceGuStart (0, SceGU.list);

    sceGuDrawBufferList (SceGU.pixel_format, (void *)0,        SceGU.line_size);
    sceGuDispBuffer     (480, 272,           (void *)0x88000,  SceGU.line_size);
    sceGuDepthBuffer    (                    (void *)0x110000, SceGU.line_size);
    sceGuOffset         (0, 0);
    sceGuViewport       ((480 / 2), (272 / 2), 480, 272);
    sceGuDepthRange     (0xc350, 0x2710);
    sceGuScissor        (0, 0, 480, 272);
    sceGuEnable         (GU_SCISSOR_TEST);
    sceGuDisable        (GU_ALPHA_TEST);
    sceGuDisable        (GU_DEPTH_TEST);
    sceGuEnable         (GU_CULL_FACE);
    sceGuDisable        (GU_LIGHTING);
    sceGuFrontFace      (GU_CW);
    sceGuEnable         (GU_TEXTURE_2D);
    sceGuClear          (GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

#if 0
    sceGuTexWrap        (GE_WRAP_REPEAT, GE_WRAP_REPEAT);
    sceGuTexFunc        (GU_TFX_MODULATE, /*GE_TCC_RGBA*/ GE_TCC_RGB);
#endif
  sceGuFinish ();

  sceGuSync   (0, 0);

  S9xSceGUSwapBuffers ();

  return (TRUE);
}

void S9xSceGUDeinit ()
{
  sceGuDisplay (0);
  sceGuTerm    ();
  /// ...
}

struct Vertex
{
  unsigned short u, v;
  unsigned short color;
  short x, y, z;
};


enum {
  SCR_SIZE_X1,
  SCR_SIZE_FIT,
  SCR_SIZE_FULL,
  SCR_SIZE_FULLFIT,
};

//ScePSPFMatrix4 projview; // Load Identity and pass this

// Utility reserved for future use
inline int S9xSceGuNumBytesPerPixel (int format)
{
  switch (format) {
    // 2 bytes per pixel
    default:
    case GU_PSM_5551:
    case GU_PSM_5650:
    case GU_PSM_4444:
      return 2;

    // 4 bytes per pixel
    case GU_PSM_8888:
      return 4;
  }
}


void* S9xSceGUGetVramBase (void)
{
  return (void *)(0x04000000 + (uint32)SceGU.vram_offset);
}

void* S9xSceGUGetVramAddr (int x, int y)
{
  // If we ever use a non-5551 pixel format, use S9xSceGUBytesPerPixel (...)

  return (void *)((char *)S9xSceGUGetVramBase () + (x * 2) +
                                                   (y * SceGU.line_size * 2));
}


void S9xSceGUSwapBuffers (void)
{
  SceGU.vram_offset = sceGuSwapBuffers ();
}


void S9xSceGURenderTex (char *tex, int width, int height, int x, int y, int xscale, int yscale, int xres, int yres);

void S9xSceGUPutImage (int snes_width, int snes_height)
{
  const int tex_res = (PSP_Settings.bSupportHiRes ? 512 : 256);

  switch (PSP_Settings.iScreenSize)
  {
    case SCR_SIZE_FIT:
    {
      // Most games run at 224 lines, but some have 239
      const int y_res   = IPPU.RenderedScreenHeight;
      // 60 pixels on either side (4:3)...
      const int x_scale = (PSP_Settings.bSupportHiRes ? 720 : 360);
      const int y_scale = (PSP_Settings.bSupportHiRes ? (y_res == 239 ? 584 : 624) :
                                                        (y_res == 239 ? 292 : 312));
      S9xSceGURenderTex ((char *)GFX.Screen, tex_res, tex_res,
                                                  60, 0,
                                             x_scale, y_scale,
                                                 360, 272);
    } break;
    case SCR_SIZE_FULL:
    case SCR_SIZE_FULLFIT:
    {
      // Most games run at 224 lines, but some have 239
      const int y_res   = IPPU.RenderedScreenHeight;
      const int x_scale = (PSP_Settings.bSupportHiRes ? 960 : 480);
      const int y_scale = (PSP_Settings.bSupportHiRes ? (y_res == 239 ? 584 : 624) :
                                                        (y_res == 239 ? 292 : 312));
      S9xSceGURenderTex ((char *)GFX.Screen, tex_res, tex_res,
                                             0,       0,
                                             x_scale, y_scale,
                                             480,     272);
    } break;
    case SCR_SIZE_X1:
    default:
    {
      const int y_res    = IPPU.RenderedScreenHeight;
      const int x_res    = IPPU.RenderedScreenWidth;
      const int xy_scale = (PSP_Settings.bSupportHiRes ? 512 : 256);
      S9xSceGURenderTex ((char *)GFX.Screen, tex_res, tex_res,
                                             (480 / 2) - (  256 / 2),
                                             (272 / 2) - (y_res / 2),
                                      xy_scale, xy_scale,
                                         x_res, y_res);
    } break;
  }
}

//#define SCEGU_DIRECT_COPY
#define SLICE_SIZE 64 // Change this to experiment with different page-cache sizes
//#define DRAW_SINGLE_BATCH

void S9xSceGURenderTex (char *tex, int width, int height, int x, int y, int xscale, int yscale, int xres, int yres)
{
  // If you don't call this, Gu will run into cache problems with
  // reading pixel data...
  sceKernelDcacheWritebackAll ();

  sceGuStart (0, SceGU.list);
  {
    // If the x/y scale matches the width/height, we can take a shortcut and
    // merely copy tex into the VRAM at the given (x,y) coordinate.
    //
    //  NOTE: This disables bilinear filtering, but that's not saying a whole
    //        lot, since the image will not require a min / mag filter.
    if ((xscale == width) && (yscale == height)) {
      sceGuCopyImage (SceGU.pixel_format, 0, 0, xres, yres, width, tex, x, y,
                        SceGU.line_size,
                          (void *)(0x04000000 + (uint32)SceGU.vram_offset));
    }

    // If the scale doesn't match the width/height, we have to perform a
    // special blit to stretch the image.
    else {
#ifdef SCEGU_DIRECT_COPY
      sceGuCopyImage (SceGU.pixel_format, 0, 0, width, height, width, tex, 0, 0,
                        SceGU.line_size,
                          (void *)(0x04000000 + (uint32)SceGU.vram_offset));
#endif

      unsigned int j;

      const int slice_scale = ((float)xscale / (float)width) * (float)SLICE_SIZE;
      const int tex_filter  = (PSP_Settings.bBilinearFilter ? GU_LINEAR :
                                                              GU_NEAREST);

      struct Vertex* vertices;
      struct Vertex* vtx_iter;

      sceGuTexMode      (SceGU.texture_format, 0, 0, 0);
#ifndef SCEGU_DIRECT_COPY
      sceGuTexImage     (0, width, height, width, tex);
#else
      sceGuTexImage     (0, width, height, SceGU.line_size, (void *)(0x04000000 + (uint32)SceGU.vram_offset));
#endif
      sceGuTexFunc      (GU_TFX_REPLACE, 0);
      sceGuTexFilter    (tex_filter, tex_filter);
      sceGuTexScale     (1, 1);
      sceGuTexOffset    (0, 0);
      sceGuAmbientColor (0xffffffff);

      sceGuScissor      (x, y, xres, yres);

#ifdef DRAW_SINGLE_BATCH
      // Allocate (memory map) the "vertex array" beforehand
      const int num_verts = (width / SLICE_SIZE) * 2;
      const int vtx_alloc = num_verts * sizeof (struct Vertex);
                 vertices = (struct Vertex *)sceGuGetMemory (vtx_alloc);
                 vtx_iter = vertices;
#endif

      // Do a striped blit (takes the page-cache into account)
      for (j = 0; j < width; j += SLICE_SIZE, x += slice_scale)
      {
#ifndef DRAW_SINGLE_BATCH
        vtx_iter = (struct Vertex *)sceGuGetMemory (sizeof (struct Vertex) * 2);
#endif
        vtx_iter [0].u = j;                 vtx_iter [0].v = 0;
        vtx_iter [0].x = x;                 vtx_iter [0].y = y;            vtx_iter [0].z = 0;
        vtx_iter [1].u = (j + SLICE_SIZE);  vtx_iter [1].v = height;
        vtx_iter [1].x = (x + slice_scale); vtx_iter [1].y = (y + yscale); vtx_iter [1].z = 0;

        vtx_iter [0].color = vtx_iter [1].color = 0;

#ifndef DRAW_SINGLE_BATCH
			  sceGuDrawArray (GU_SPRITES, SceGU.tt | SceGU.ct | SceGU.mt | SceGU.dm, 2, 0, vtx_iter);
        vtx_iter += 2;
#endif
      }

#ifdef DRAW_SINGLE_BATCH
      sceGuDrawArray (GU_SPRITES, GE_SETREG_VTYPE (SceGU.tt,
                                                   SceGU.ct,
                                                   0,
                                                   SceGU.mt,
                                                   0, 0, 0, 0,
                                                   SceGU.dm),
                                            num_verts,
                                               0,
                                            vertices);
#endif
    }
  }
  sceGuFinish ();
  sceGuSync   (0, 0);

  if (PSP_Settings.bVSync)
    sceDisplayWaitVblankStart ();

  S9xSceGUSwapBuffers ();
}
};


#if 0
void DrawTilePSP (uint32 Tile, uint32 Offset, uint32 StartLine,
      uint32 LineCount)
{
    TILE_PREAMBLE

    float x = Offset % GFX.Pitch;
    float y = Offset / GFX.Pitch;

#define X 0
#define Y 1

#define U 0
#define V 1

    float pos [3][4];
    float tex [2][4];

    static bool8 init = FALSE;

    if (init == FALSE) {
    sceGuStart(0,list);
    sceGuDrawBufferList(GE_PSM_5551,(void*)0,512);
    sceGuDispBuffer(480,272,(void*)0x88000,512);
    sceGuDepthBuffer((void*)0x110000,512);
    sceGuOffset(0,0);
    sceGuViewport(480/2,272/2,480,272);
    sceGuDepthRange(0xc350,0x2710);
    sceGuScissor(0,0,480,272);
    sceGuEnable(GU_STATE_SCISSOR);
    sceGuDisable(GU_STATE_ATE);
    sceGuDisable(GU_STATE_ZTE);
    sceGuEnable(GU_STATE_CULL);
    sceGuDisable(GU_STATE_ALPHA);
    sceGuDisable(GU_STATE_LIGHTING);
    sceGuFrontFace(GE_FACE_CW);
    sceGuEnable(GU_STATE_TEXTURE);
    sceGuClear(GE_CLEAR_COLOR|GE_CLEAR_DEPTH);
    sceGuFinish();
    sceGuSync(0,0);

    init = TRUE;
    }

    pos [0][X] = 0 + x * 1;
    pos [0][Y] = 0 + y * 1;
    pos [1][X] = 0 + (x + 8.0f) * 1;
    pos [1][Y] = 0 + y * 1;
    pos [2][X] = 0 + (x + 8.0f) * 1;
    pos [2][Y] = 0 + (y + LineCount) * 1;
    pos [3][X] = 0 + x * 1;
    pos [4][Y] = 0 + (y + LineCount) * 1;

    if (!(Tile & (V_FLIP | H_FLIP)))
    {
  // Normal
  tex [0][U] = 0.0f;
  tex [0][V] = StartLine;
  tex [1][U] = 8.0f;
  tex [1][V] = StartLine;
  tex [2][U] = 8.0f;
  tex [2][V] = StartLine + LineCount;
  tex [3][U] = 0.0f;
  tex [3][V] = StartLine + LineCount;
    }
    else
    if (!(Tile & V_FLIP))
    {
  // Flipped
  tex [0][U] = 8.0f;
  tex [0][V] = StartLine;
  tex [1][U] = 0.0f;
  tex [1][V] = StartLine;
  tex [2][U] = 0.0f;
  tex [2][V] = StartLine + LineCount;
  tex [3][U] = 8.0f;
  tex [3][V] = StartLine + LineCount;

    }
    else
    if (Tile & H_FLIP)
    {
  // Horizontal and vertical flip
  tex [0][U] = 8.0f;
  tex [0][V] = StartLine + LineCount;
  tex [1][U] = 0.0f;
  tex [1][V] = StartLine + LineCount;
  tex [2][U] = 0.0f;
  tex [2][V] = StartLine;
  tex [3][U] = 8.0f;
  tex [3][V] = StartLine;

    }
    else
    {
  // Vertical flip only
  tex [0][U] = 0.0f;
  tex [0][V] = StartLine + LineCount;
  tex [1][U] = 8.0f;
  tex [1][V] = StartLine + LineCount;
  tex [2][U] = 8.0f;
  tex [2][V] = StartLine;
  tex [3][U] = 0.0f;
  tex [3][V] = StartLine;

    }

    sceGuStart(0,list);

    sceGuTexMode(GU_PSM_5551,0,0,0);
    sceGuTexFunc(GU_TFX_REPLACE,0);
    sceGuTexOffset(0,0);
    sceGuAmbientColor(0xffffffff);


    sceGuTexImage (0, 8, 8, 8, (void *)pCache);
    sceGuTexScale (1.0/8.0f, 1.0f/8.0f);

    struct Vertex *vertices;
    vertices = (struct Vertex *)sceGuGetMemory (4 * sizeof (struct Vertex));

    vertices[0].u = tex[0][U]; vertices[0].v = tex[0][V];
    vertices[0].x = pos[0][X]; vertices[0].y = pos[0][Y]; vertices[0].z = 0.0f;
    vertices[1].u = tex[1][U]; vertices[1].v = tex[1][V];
    vertices[1].x = pos[1][X]; vertices[1].y = pos[1][Y]; vertices[1].z = 0.0f;
    vertices[2].u = tex[2][U]; vertices[2].v = tex[2][V];
    vertices[2].x = pos[2][X]; vertices[2].y = pos[2][Y]; vertices[2].z = 0.0f;
    vertices[3].u = tex[3][U]; vertices[3].v = tex[3][V];
    vertices[3].x = pos[3][X]; vertices[3].y = pos[3][Y]; vertices[3].z = 0.0f;

    sceGuDrawArray (GU_PRIM_TRIANGLES,GE_SETREG_VTYPE(GE_TT_16BIT,GE_CT_5551,0,GE_MT_16BIT,0,0,0,0,GE_BM_2D),4,0,vertices);

    sceGuFinish ();
}
#endif

#endif /* USE_SCEGU */

/* vi:set ts=2 sts=2 sw=2 et: */ /* tw=80 - prefered */

