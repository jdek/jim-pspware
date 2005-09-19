/***********************************************************************************

  Module :	CGfx.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CGfx.h"
#include "CFrameWork.h"
#include "CTextureManager.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************
const u32			CGfx::s_ScreenWidth( 480 );
const u32			CGfx::s_ScreenHeight( 272 );

static const u32	BUF_WIDTH( 512 );

static u32 __attribute__( ( aligned( 16 ) ) )	s_DrawList[ 0x40000 ];

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//*************************************************************************************
//	Initialise the GPU
//*************************************************************************************
bool	CGfx::Open()
{
	sceGuInit();
	sceGuStart( GU_DIRECT, GetDrawList() );

	u8 *	p_vram( 0x00000000 );

	sceGuDrawBuffer( GU_PSM_8888, p_vram, BUF_WIDTH );
	p_vram += BUF_WIDTH * s_ScreenHeight * 4;
	sceGuDispBuffer( s_ScreenWidth, s_ScreenHeight, p_vram, BUF_WIDTH );
//	p_vram += BUF_WIDTH * s_ScreenHeight * 4;
//	sceGuDepthBuffer( p_vram, BUF_WIDTH );

	sceGuClear( GU_COLOR_BUFFER_BIT | GU_STENCIL_BUFFER_BIT );
//	sceGuClear( GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT | GU_STENCIL_BUFFER_BIT );

	sceGuOffset( 2048 - ( s_ScreenWidth / 2 ), 2048 - ( s_ScreenHeight / 2 ) );
	sceGuViewport( 2048, 2048, s_ScreenWidth, s_ScreenHeight );

//	sceGuDepthRange( 0xc350, 0x2710 );

	sceGuScissor( 0, 0, s_ScreenWidth, s_ScreenHeight );
	sceGuEnable( GU_SCISSOR_TEST );

	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );

	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
//	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_DST_COLOR, 0, 0 );
	sceGuEnable( GU_BLEND );

//	sceGuDepthFunc( GU_GEQUAL );
//	sceGuEnable( GU_DEPTH_TEST );

	sceGuShadeModel( GU_SMOOTH );
	sceGuEnable( GU_TEXTURE_2D );

	sceGuDepthMask( GU_FALSE );
	sceGuDisable( GU_STENCIL_TEST );

	//
	//	Flush the new GPU settings
	//
	sceGuFinish();
	sceGuSync( 0, 0 );

	sceDisplayWaitVblankStart();
	sceGuDisplay( GU_TRUE );

	//
	//	Clear all the GPU matrices
	//
	ScePspFMatrix4 mtx;

	mtx.x.x = 1.f;	mtx.x.y = 0.f;	mtx.x.z = 0.f;	mtx.x.w = 0.f;
	mtx.y.x = 0.f;	mtx.y.y = 1.f;	mtx.y.z = 0.f;	mtx.y.w = 0.f;
	mtx.z.x = 0.f;	mtx.z.y = 0.f;	mtx.z.z = 1.f;	mtx.z.w = 0.f;
	mtx.w.x = 0.f;	mtx.w.y = 0.f;	mtx.w.z = 0.f;	mtx.w.w = 1.f;

	sceGuSetMatrix( GU_VIEW,		&mtx );
	sceGuSetMatrix( GU_MODEL,		&mtx );
	sceGuSetMatrix( GU_PROJECTION,	&mtx );

	return true;
}

//*************************************************************************************
//	Shutdown the GPU
//*************************************************************************************
void	CGfx::Close()
{
}

//*************************************************************************************
//	Kick off a draw list
//*************************************************************************************
void	CGfx::BeginRender()
{
	sceGuStart( GU_DIRECT, GetDrawList() );
}

//*************************************************************************************
//	End the drawlist and send it to the GPU
//*************************************************************************************
void	CGfx::EndRender()
{
	sceGuFinish();
	sceGuSync( 0, 0 );
}

//*************************************************************************************
//	Clear the screen to the specified color
//*************************************************************************************
void	CGfx::ClearScreen( u32 color )
{
	sceGuClearColor( color );
//	sceGuClearDepth( 0 );
	sceGuClearStencil( 0 );
	sceGuClear( GU_COLOR_BUFFER_BIT | GU_STENCIL_BUFFER_BIT );
//	sceGuClear( GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT | GU_STENCIL_BUFFER_BIT );
}

//*************************************************************************************
//	Swap the display buffers
//*************************************************************************************
void	CGfx::SwapBuffers()
{
	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

//*************************************************************************************
//	Returns a pointer to the display list
//*************************************************************************************
u32 *	CGfx::GetDrawList()
{
	return s_DrawList;
}

//*************************************************************************************
//	
//*************************************************************************************
void	CGfx::DrawQuad( const V2 & pos, const V2 & dimension, ARGB color )
{
	DrawQuad( pos, dimension, color, color, color, color );
}

//*************************************************************************************
//	
//*************************************************************************************
void	CGfx::DrawQuad( const V2 & pos, const V2 & dimension, ARGB color0, ARGB color1, ARGB color2, ARGB color3 )
{
	sVertexColor *	p_vert;

	GetPolyList( 4, &p_vert );

	p_vert[ 0 ].color = color0.color;
	p_vert[ 0 ].x = pos.x;
	p_vert[ 0 ].y = pos.y;
	p_vert[ 0 ].z = 0.f;

	p_vert[ 1 ].color = color1.color;
	p_vert[ 1 ].x = pos.x + dimension.x;
	p_vert[ 1 ].y = pos.y;
	p_vert[ 1 ].z = 0.f;

	p_vert[ 2 ].color = color2.color;
	p_vert[ 2 ].x = pos.x;
	p_vert[ 2 ].y = pos.y + dimension.y;
	p_vert[ 2 ].z = 0.f;

	p_vert[ 3 ].color = color3.color;
	p_vert[ 3 ].x = pos.x + dimension.x;
	p_vert[ 3 ].y = pos.y + dimension.y;
	p_vert[ 3 ].z = 0.f;

	DrawPoly( p_vert, 4 );
}

//*************************************************************************************
//	
//*************************************************************************************
void	CGfx::DrawQuad( const CTexture * const p_texture, const V2 & pos, const V2 & dimension, ARGB color )
{
	DrawQuad( p_texture, pos, dimension, color, color, color, color );
}

//*************************************************************************************
//	
//*************************************************************************************
void	CGfx::DrawQuad( const CTexture * const p_texture, const V2 & pos, const V2 & dimension, ARGB color0, ARGB color1, ARGB color2, ARGB color3 )
{
	sVertexTexturedColor *	p_vert;

	GetPolyList( 4, &p_vert );

	p_vert[ 0 ].u = 0.f;
	p_vert[ 0 ].v = 0.f;
	p_vert[ 0 ].color = color0.color;
	p_vert[ 0 ].x = pos.x;
	p_vert[ 0 ].y = pos.y;
	p_vert[ 0 ].z = 0.f;

	p_vert[ 1 ].u = p_texture->m_nWidth;
	p_vert[ 1 ].v = 0.f;
	p_vert[ 1 ].color = color1.color;
	p_vert[ 1 ].x = pos.x + dimension.x;
	p_vert[ 1 ].y = pos.y;
	p_vert[ 1 ].z = 0.f;

	p_vert[ 2 ].u = 0.f;
	p_vert[ 2 ].v = p_texture->m_nHeight;
	p_vert[ 2 ].color = color2.color;
	p_vert[ 2 ].x = pos.x;
	p_vert[ 2 ].y = pos.y + dimension.y;
	p_vert[ 2 ].z = 0.f;

	p_vert[ 3 ].u = p_texture->m_nWidth;
	p_vert[ 3 ].v = p_texture->m_nHeight;
	p_vert[ 3 ].color = color3.color;
	p_vert[ 3 ].x = pos.x + dimension.x;
	p_vert[ 3 ].y = pos.y + dimension.y;
	p_vert[ 3 ].z = 0.f;

	DrawPoly( p_texture, p_vert, 4 );
}

//*************************************************************************************
//	
//*************************************************************************************
void	CGfx::GetPolyList( u32 point_count, sVertexColor ** p_poly_list )
{
	*p_poly_list = ( sVertexColor * )sceGuGetMemory( point_count * sizeof( sVertexColor ) );
}

//*************************************************************************************
//	
//*************************************************************************************
void	CGfx::GetPolyList( u32 point_count, sVertexTexturedColor ** p_poly_list )
{
	*p_poly_list = ( sVertexTexturedColor * )sceGuGetMemory( point_count * sizeof( sVertexTexturedColor ) );
}

//*************************************************************************************
//	
//*************************************************************************************
void	CGfx::DrawPoly( const sVertexColor * const p_poly_list, u32 point_count )
{
	sceGuDisable( GU_TEXTURE_2D );
	sceGuShadeModel( GU_SMOOTH );
	sceGuAmbientColor( 0xffffffff );
	sceGuDrawArray( GU_TRIANGLE_STRIP, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, point_count, 0, p_poly_list );
}

//*************************************************************************************
//	
//*************************************************************************************
void	CGfx::DrawPoly( const CTexture * const p_texture, const sVertexTexturedColor * const p_poly_list, u32 point_count )
{
	sceGuEnable( GU_TEXTURE_2D );
	sceGuShadeModel( GU_SMOOTH );
	if ( p_texture->m_pVRAM != NULL )
	{
		sceGuTexMode( GU_PSM_8888, 0, 0, 0 );
		sceGuTexImage( 0, p_texture->m_nCanvasWidth, p_texture->m_nCanvasHeight, p_texture->m_nCanvasWidth, p_texture->m_pVRAM );
	}
	else
	{
		sceGuTexMode( GU_PSM_8888, 0, 0, 0 );
		sceGuTexImage( 0, p_texture->m_nCanvasWidth, p_texture->m_nCanvasHeight, p_texture->m_nCanvasWidth, p_texture->m_pBuffer );
	}
	sceGuTexFunc( GU_TFX_MODULATE, GU_TCC_RGBA );
	sceGuTexWrap( GU_CLAMP, GU_CLAMP );
	sceGuTexFilter( GU_LINEAR, GU_LINEAR );
	sceGuTexScale( 1.0f, 1.0f );
	sceGuTexOffset( 0.0f, 0.0f );
	sceGuAmbientColor( 0xffffffff );
	sceGuDrawArray( GU_TRIANGLE_STRIP, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, point_count, 0, p_poly_list );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CGfx::DrawSprite( const sVertexColor * const p_poly_list, u32 point_count )
{
	sceGuDisable( GU_TEXTURE_2D );
	sceGuShadeModel( GU_SMOOTH );
	sceGuAmbientColor( 0xffffffff );
	sceGuDrawArray( GU_SPRITES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, point_count, 0, p_poly_list );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CGfx::DrawSprite( const CTexture * const p_texture, const sVertexTexturedColor * const p_poly_list, u32 point_count )
{
	sceGuEnable( GU_TEXTURE_2D );
	sceGuShadeModel( GU_SMOOTH );
	if ( p_texture->m_pVRAM != NULL )
	{
		sceGuTexMode( GU_PSM_8888, 0, 0, 0 );
		sceGuTexImage( 0, p_texture->m_nCanvasWidth, p_texture->m_nCanvasHeight, p_texture->m_nCanvasWidth, p_texture->m_pVRAM );
	}
	else
	{
		sceGuTexMode( GU_PSM_8888, 0, 0, 0 );
		sceGuTexImage( 0, p_texture->m_nCanvasWidth, p_texture->m_nCanvasHeight, p_texture->m_nCanvasWidth, p_texture->m_pBuffer );
	}
	sceGuTexFunc( GU_TFX_MODULATE, GU_TCC_RGBA );
	sceGuTexWrap( GU_CLAMP, GU_CLAMP );
	sceGuTexFilter( GU_LINEAR, GU_LINEAR );
	sceGuTexScale( 1.0f, 1.0f );
	sceGuTexOffset( 0.0f, 0.0f );
	sceGuAmbientColor( 0xffffffff );
	sceGuDrawArray( GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, point_count, 0, p_poly_list );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CGfx::SetClipRegion( const V2 & pos, const V2 & size )
{
	sceGuEnable( GU_STENCIL_TEST );					// Stencil test
	sceGuDepthMask( GU_TRUE );
	sceGuStencilFunc( GU_ALWAYS, 1, 1 );			// always set 1 bit in 1 bit mask
	sceGuStencilOp( GU_KEEP, GU_KEEP, GU_REPLACE );	// keep value on failed test (fail and zfail) and replace on pass
	sceGuDepthMask( GU_FALSE );

	DrawQuad( pos, size, 0x01ffffff );

	sceGuStencilFunc( GU_EQUAL, 1, 1 );				// allow drawing where stencil is 1
	sceGuStencilOp( GU_KEEP, GU_KEEP, GU_KEEP );	// keep the stencil buffer unchanged
}

//**********************************************************************************
//	
//**********************************************************************************
void	CGfx::DisableClipRegions()
{
	sceGuDisable( GU_STENCIL_TEST );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CGfx::ClearClipRegions()
{
	sceGuClearStencil( 0 );
	sceGuClear( GU_STENCIL_BUFFER_BIT );

	EndRender();
	BeginRender();
}

//*******************************  END OF FILE  ************************************
