/***********************************************************************************

  Module :	CFont.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 03 August 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CFont.h"
#include "CFrameWork.h"
#include "CTextureManager.h"
#include "CGfx.h"
#include "CSkinManager.h"

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
CFont *		CFont::s_pDefaultFont( NULL );
u32			CFont::s_LookupTable[ 255 ];
const u8	CFont::s_FontMap[ CFont::FONT_MAP_HEIGHT ][ CFont::FONT_MAP_WIDTH ] =
{
	{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', },
	{ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '6', '7', '8', '9', '0', },
	{ '{', '}', '[', ']', '(', ')', '<', '>', '$', '*', '-', '+', '=', '/', '#', '_', '%', '^', '@', '\\', '&', '|', '~', '?', '\'', '"', '!', ',', '.', ';', ':', },
};

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
void	CFont::Open()
{
	//
	//	Build the lookup table
	//
	for ( u8 c = 0; c < 255; ++c )
	{
		for ( u32 y = 0; y < FONT_MAP_HEIGHT; ++y )
		{
			for ( u32 x = 0; x < FONT_MAP_WIDTH; ++x )
			{
				if ( c == s_FontMap[ y ][ x ] )
				{
					s_LookupTable[ c ] = x + ( y * FONT_MAP_WIDTH );

					break;
				}
			}
		}
	}

	//
	//	Create the default font
	//
	s_pDefaultFont = new CFont( CSkinManager::GetV2( "font", "size", V2( 7.f, 12.f ) ) );
}

//*************************************************************************************
//	
//*************************************************************************************
void	CFont::Close()
{
	SAFE_DELETE( s_pDefaultFont );
}

//*************************************************************************************
//	
//*************************************************************************************
CFont *	CFont::Create( const V2 & size )
{
	return new CFont( size );
}

//*************************************************************************************
//	
//*************************************************************************************
CFont *	CFont::GetDefaultFont()
{
	ASSERT( s_pDefaultFont != NULL, "Trying to access a NULL font" );

	return s_pDefaultFont;
}

//*************************************************************************************
//	
//*************************************************************************************
CFont::CFont( const V2 & size )
:	m_Size( size )
{
	m_pTexture = CSkinManager::GetComponent( CSkinManager::SC_FONT );

	ASSERT( m_pTexture != NULL, "Couldn't create the font!" );

	CreateGlyphData();
}

//*************************************************************************************
//	
//*************************************************************************************
CFont::~CFont()
{
}

//*************************************************************************************
//	
//*************************************************************************************
void	CFont::CreateGlyphData()
{
	const CTexture * const	p_texture( m_pTexture->GetTexture() );
	const s32				page_width( p_texture->m_nCanvasWidth );
	const u8 * const		p_pixels( p_texture->m_pBuffer );
	const s32				font_width( static_cast< s32 >( m_Size.x ) );
	const s32				font_height( static_cast< s32 >( m_Size.y ) );
	sGlyphData *			p_glyph_data( m_GlyphData );

	for ( u32 y = 0; y < FONT_MAP_HEIGHT; ++y )
	{
		for ( u32 x = 0; x < FONT_MAP_WIDTH; ++x )
		{
			s32					glyph_end( 0 );
			s32					glyph_begin( font_width );
			const u8 * const	p_glyph_pix( &p_pixels[ ( ( x * font_width ) + ( y * font_height * page_width ) ) * 4 ] );

			for ( s32 gx = 0; gx < font_width; ++gx )
			{
				for ( s32 gy = 0; gy < font_height; ++gy )
				{
					const u32	offset( ( gx + ( gy * page_width ) ) * 4 );

					if ( p_glyph_pix[ offset + 0 ] != 0 ||
						 p_glyph_pix[ offset + 1 ] != 0 ||
						 p_glyph_pix[ offset + 2 ] != 0 )
					{
						if ( gx <= glyph_begin )
						{
							glyph_begin = gx;
						}

						if ( gx >= glyph_end )
						{
							glyph_end = gx;
						}

						break;
					}
				}
			}

			p_glyph_data->x = ( x * font_width ) + glyph_begin;
			p_glyph_data->y = ( y * font_height );
			p_glyph_data->m_Width = ( glyph_end - glyph_begin ) + 1;

			++p_glyph_data;
		}
	}
}

//*************************************************************************************
//	
//*************************************************************************************
const CFont::sGlyphData *	CFont::FindGlyph( char glyph ) const
{
	return &m_GlyphData[ s_LookupTable[ static_cast< u32 >( glyph ) ] ];
}

//*************************************************************************************
//	
//*************************************************************************************
void	CFont::Print( const CString & string, const V2 & pos, ARGB color, float scale, bool shadow )
{
	const char *			p_char( string );
	const V2				orig_pos( static_cast< float >( static_cast< s32 >( pos.x ) ), static_cast< float >( static_cast< s32 >( pos.y ) ) );
	V2						text_pos( orig_pos );
	u32						sprite_count( 0 );
	sVertexTexturedColor *	p_vert;
	sVertexTexturedColor *	p_dst;
	const CTexture * const	p_texture( m_pTexture->GetTexture() );

	CGfx::GetPolyList( 2 * string.Length(), &p_vert );

	p_dst = p_vert;

	for ( s32 i = 0; i < string.Length(); ++i )
	{
		switch ( *p_char )
		{
		case ' ':
			{
				text_pos.x += ( m_Size.x * scale );
			}
			break;

		case '\n':
			{
				text_pos.x = orig_pos.x;
				text_pos.y += ( ( m_Size.y + 1 ) * scale );
			}
			break;

		default:
			{
				const sGlyphData * const	p_glyph_data( FindGlyph( *p_char ) );

				if ( p_glyph_data != NULL )
				{
					p_dst->u = p_glyph_data->x;
					p_dst->v = p_glyph_data->y;
					p_dst->color = color;
					p_dst->x = text_pos.x;
					p_dst->y = text_pos.y;
					p_dst->z = 0.f;
					++p_dst;

					p_dst->u = p_glyph_data->x + p_glyph_data->m_Width;
					p_dst->v = p_glyph_data->y + m_Size.y;
					p_dst->color = color;
					p_dst->x = text_pos.x + ( p_glyph_data->m_Width * scale );
					p_dst->y = text_pos.y + ( m_Size.y * scale );
					p_dst->z = 0.f;
					++p_dst;

					text_pos.x += ( ( p_glyph_data->m_Width + 1 ) * scale );

					sprite_count += 2;
				}
			}
			break;
		}

		p_char++;
	}

	if ( shadow == false )
	{
		CGfx::DrawSprite( p_texture, p_vert, sprite_count );
	}
	else
	{
		sVertexTexturedColor *	p_shadow_vert;

		CGfx::GetPolyList( sprite_count, &p_shadow_vert );

		memcpy( p_shadow_vert, p_vert, sprite_count * sizeof( sVertexTexturedColor ) );

		for ( u32 i = 0; i < sprite_count; ++i )
		{
			p_shadow_vert[ i ].color.r = 0x00;
			p_shadow_vert[ i ].color.g = 0x00;
			p_shadow_vert[ i ].color.b = 0x00;
			p_shadow_vert[ i ].x += 1.f;
			p_shadow_vert[ i ].y += 1.f;
		}

		CGfx::DrawSprite( p_texture, p_shadow_vert, sprite_count );
		CGfx::DrawSprite( p_texture, p_vert, sprite_count );
	}
}

//*************************************************************************************
//	
//*************************************************************************************
V2	CFont::GetStringSize( const CString & string ) const
{
	const char *	p_char( string );
	V2				size( 0.f, m_Size.y );
	float			last_x( 0.f );

	for ( s32 i = 0; i < string.Length(); ++i )
	{
		switch ( *p_char )
		{
		case ' ':
			{
				size.x += m_Size.x;
			}
			break;

		case '\n':
			{
				if ( last_x < size.x )
				{
					last_x = size.x;
				}

				size.x = 0.f;
				size.y += ( m_Size.y + 1 );
			}
			break;

		default:
			{
				const sGlyphData * const	p_glyph_data( FindGlyph( *p_char ) );

				if ( p_glyph_data != NULL )
				{
					size.x += ( p_glyph_data->m_Width + 1 );
				}
			}
			break;
		}

		p_char++;
	}

	if ( last_x > size.x )
	{
		size.x = last_x;
	}

	return size;
}

//*******************************  END OF FILE  ************************************
