/***********************************************************************************

  Module :	CGfx.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

#ifndef CGFX_H_
#define CGFX_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CVector.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
class CTexture;

struct sVertexColor
{
	ARGB	color;
	float	x, y, z;
};

struct sVertexTexturedColor
{
	float	u, v;
	ARGB	color;
	float	x, y, z;
};

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CGfx
{
	public:

		// Initialise the GPU
		static bool			Open();

		// Shutdown the GPU
		static void			Close();

		// Prepares the draw list for use
		static void			BeginRender();

		// Terminates the current draw list
		static void			EndRender();

		// Clears the screen to the specified ARGB color
		static void			ClearScreen( u32 color );

		// Swap the draw list buffers
		static void			SwapBuffers();

		// Returns the address of the current draw list
		static u32 *		GetDrawList();

		// Draw a gouraud shaded quad
		static void			DrawQuad( const V2 & pos, const V2 & dimension, ARGB color );
		static void			DrawQuad( const V2 & pos, const V2 & dimension, ARGB color0, ARGB color1, ARGB color2, ARGB color3 );

		static void			DrawQuad( const CTexture * const p_texture, const V2 & pos, const V2 & dimension, ARGB color );
		static void			DrawQuad( const CTexture * const p_texture, const V2 & pos, const V2 & dimension, ARGB color0, ARGB color1, ARGB color2, ARGB color3 );

		// Returns a GPU poly list
		static void			GetPolyList( u32 point_count, sVertexColor ** p_poly_list );
		static void			GetPolyList( u32 point_count, sVertexTexturedColor ** p_poly_list );

		// Draw a polygon list
		static void			DrawPoly( const sVertexColor * const p_poly_list, u32 point_count );
		static void			DrawPoly( const CTexture * const p_texture, const sVertexTexturedColor * const p_poly_list, u32 point_count );

		// Draw a sprite list
		static void			DrawSprite( const sVertexColor * const p_poly_list, u32 point_count );
		static void			DrawSprite( const CTexture * const p_texture, const sVertexTexturedColor * const p_poly_list, u32 point_count );

		// Sets a clip region using the stencil buffer
		static void			SetClipRegion( const V2 & pos, const V2 & size );

		// Sets a clip region using the stencil buffer
		static void			DisableClipRegions();

		// Clears the stencil buffer
		static void			ClearClipRegions();

	public:

		// Width and height of the screen
		static const u32	s_ScreenWidth;
		static const u32	s_ScreenHeight;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CGFX_H_ */
