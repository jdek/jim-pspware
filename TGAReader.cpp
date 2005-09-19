/***********************************************************************************

  Module :	TGAReader.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 02 August 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CFrameWork.h"
#include "CTextureManager.h"
#include "CFileSystem.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
struct HEADER
{
	char  idlength;
	char  colourmaptype;
	char  datatypecode;
	short int colourmaporigin;
	short int colourmaplength;
	char  colourmapdepth;
	short int x_origin;
	short int y_origin;
	short width;
	short height;
	char  bitsperpixel;
	char  imagedescriptor;
};

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
static void	MergeBytes( u8 * const pixel, const u8 * const p, int bytes )
{
	if ( bytes == 4 )
	{
		pixel[ 0 ] = p[ 2 ];
		pixel[ 1 ] = p[ 1 ];
		pixel[ 2 ] = p[ 0 ];
		pixel[ 3 ] = p[ 3 ];
	}
	else if ( bytes == 3 )
	{
		pixel[ 0 ] = p[ 2 ];
		pixel[ 1 ] = p[ 1 ];
		pixel[ 2 ] = p[ 0 ];
		pixel[ 3 ] = 0xff;
	}
	else if ( bytes == 2 )
	{
		pixel[ 0 ] = ( p[ 1 ] & 0x7c ) << 1;
		pixel[ 1 ] = ( ( p[ 1 ] & 0x03 ) << 6 ) | ( ( p[ 0 ] & 0xe0 ) >> 2 );
		pixel[ 2 ] = ( p[ 0 ] & 0x1f ) << 3;
		pixel[ 3 ] = ( p[ 1 ] & 0x80 );
	}
	else
	{
		ASSERT( 0, "Unknown number of bytes!" );
	}
}

//**********************************************************************************
//	
//**********************************************************************************
#define NEXT_PIXEL()									\
	x++;												\
	p_dst += 4;											\
														\
	if ( x == p_texture->m_nWidth )						\
	{													\
		x = 0;											\
		++y;											\
		p_dst -= ( p_texture->m_nWidth << 2	);			\
		p_dst += ( p_texture->m_nCanvasWidth << 2 );	\
	}

//*************************************************************************************
//	
//*************************************************************************************
CTexture *	CTextureManager::ReadTGA( CFile * const p_file )
{
	HEADER		header;
	CTexture *	p_texture( NULL );

	header.idlength = p_file->FGetC();
	header.colourmaptype = p_file->FGetC();
	header.datatypecode = p_file->FGetC();
	p_file->Read( &header.colourmaporigin, 2 );
	p_file->Read( &header.colourmaplength, 2 );
	header.colourmapdepth = p_file->FGetC();
	p_file->Read( &header.x_origin, 2 );
	p_file->Read( &header.y_origin, 2 );
	p_file->Read( &header.width, 2 );
	p_file->Read( &header.height, 2 );
	header.bitsperpixel = p_file->FGetC();
	header.imagedescriptor = p_file->FGetC();

	if ( header.datatypecode != 2 && header.datatypecode != 10 )
	{
		ASSERT( 0, "Can only handle image type 2 and 10\n" );

		return NULL;
	}

	if ( header.bitsperpixel != 16 &&  header.bitsperpixel != 24 && header.bitsperpixel != 32 )
	{
		ASSERT( 0, "Can only handle pixel depths of 16, 24, and 32\n" );

		return NULL;
	}

	if ( header.colourmaptype != 0 && header.colourmaptype != 1 )
	{
		ASSERT( 0, "Can only handle colour map types of 0 and 1\n" );

		return NULL;
	}

	p_texture = new CTexture();

	ASSERT( p_texture != NULL, "Out of memory!" );

	if ( p_texture != NULL )
	{
		if ( p_texture->Init( header.width, header.height ) == false )
		{
			SAFE_DELETE( p_texture );
		}
		else
		{
			// Skip some crap
			p_file->Seek( header.idlength + ( header.colourmaptype * header.colourmaplength ), SEEK_CUR );

			// Read the image
			u8			p[ 5 ];
			u32			bytes_read;
			const u32	bytes2read( header.bitsperpixel / 8 );
			u32			x( 0 );
			s32			y( 0 );
			u8 *		p_dst( p_texture->m_pBuffer );

			while ( y < header.height )
			{
				// Uncompressed
				if ( header.datatypecode == 2 )
				{
					if ( p_file->Read( p, bytes2read, &bytes_read ) == false || bytes_read != bytes2read )
					{
						ASSERT( 0, "Unexpected end of file\n" );

						SAFE_DELETE( p_texture );

						return NULL;
					}

					MergeBytes( p_dst, p, bytes2read );

					NEXT_PIXEL();
				}
				// Compressed
				else if ( header.datatypecode == 10 )
				{
					if ( p_file->Read( p, ( bytes2read + 1 ), &bytes_read ) == false || bytes_read != ( bytes2read + 1 ) )
					{
						ASSERT( 0, "Unexpected end of file\n" );

						SAFE_DELETE( p_texture );

						return NULL;
					}

					const u32	j( p[ 0 ] & 0x7f );

					MergeBytes( p_dst, &( p[ 1 ] ), bytes2read );

					NEXT_PIXEL();

					if ( p[ 0 ] & 0x80 )
					{
						// RLE chunk
						for ( u32 i = 0; i < j; i++ )
						{
							MergeBytes( p_dst, &( p[ 1 ] ), bytes2read );

							NEXT_PIXEL();
						}
					}
					else
					{
						// Normal chunk
						for ( u32 i = 0; i < j; i++ )
						{
							if ( p_file->Read( p, bytes2read, &bytes_read ) == false || bytes_read != bytes2read )
							{
								ASSERT( 0, "Unexpected end of file\n" );

								SAFE_DELETE( p_texture );

								return NULL;
							}

							MergeBytes( p_dst, p, bytes2read );

							NEXT_PIXEL();
						}
					}
				}
			}
		}
	}

	return p_texture;
}

//*******************************  END OF FILE  ************************************
