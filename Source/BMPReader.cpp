/***********************************************************************************

  Module :	BMPReader.cpp

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
typedef struct                       /**** BMP file header structure ****/
{
	unsigned short bfType;           /* Magic number for file */
	unsigned int   bfSize;           /* Size of file */
	unsigned short bfReserved1;      /* Reserved */
	unsigned short bfReserved2;      /* ... */
	unsigned int   bfOffBits;        /* Offset to bitmap data */
} BITMAPFILEHEADER;

#  define BF_TYPE 0x4D42             /* "MB" */

typedef struct                       /**** BMP file info structure ****/
{
	unsigned int   biSize;           /* Size of info header */
	int            biWidth;          /* Width of image */
	int            biHeight;         /* Height of image */
	unsigned short biPlanes;         /* Number of color planes */
	unsigned short biBitCount;       /* Number of bits per pixel */
	unsigned int   biCompression;    /* Type of compression to use */
	unsigned int   biSizeImage;      /* Size of image data */
	int            biXPelsPerMeter;  /* X pixels per meter */
	int            biYPelsPerMeter;  /* Y pixels per meter */
	unsigned int   biClrUsed;        /* Number of colors used */
	unsigned int   biClrImportant;   /* Number of important colors */
} BITMAPINFOHEADER;

/*
* Constants for the biCompression field...
*/

#  define BI_RGB       0             /* No compression - straight BGR data */
#  define BI_RLE8      1             /* 8-bit run-length compression */
#  define BI_RLE4      2             /* 4-bit run-length compression */
#  define BI_BITFIELDS 3             /* RGB bitmap with RGB masks */

typedef struct                       /**** Colormap entry structure ****/
{
	unsigned char  rgbBlue;          /* Blue value */
	unsigned char  rgbGreen;         /* Green value */
	unsigned char  rgbRed;           /* Red value */
	unsigned char  rgbReserved;      /* Reserved */
} RGBQUAD;

typedef struct                       /**** Bitmap information structure ****/
{
	BITMAPINFOHEADER bmiHeader;      /* Image header */
	RGBQUAD          bmiColors[256]; /* Image colormap */
} BITMAPINFO;

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
CTexture *	CTextureManager::ReadBMP( CFile * const p_file )
{
	u8 *				bits;		// Bitmap pixel bits
	s32					bitsize;	// Size of bitmap
	s32					infosize;	// Size of header information
	BITMAPFILEHEADER	header;		// File header
	BITMAPINFO			info;
	CTexture *			p_texture( NULL );

	// Read the file header and any following bitmap information...
	p_file->Read( &header.bfType, sizeof( header.bfType ) );
	p_file->Read( &header.bfSize, sizeof( header.bfSize ) );
	p_file->Read( &header.bfReserved1, sizeof( header.bfReserved1 ) );
	p_file->Read( &header.bfReserved2, sizeof( header.bfReserved2 ) );
	p_file->Read( &header.bfOffBits, sizeof( header.bfOffBits ) );

	if ( header.bfType != BF_TYPE )	// Check for BM reversed...
	{
		ASSERT( 0, "Not a bitmap file" );

		return NULL;
	}

	infosize = header.bfOffBits - 18;

	p_file->Read( &info.bmiHeader.biSize, sizeof( info.bmiHeader.biSize ) );
	p_file->Read( &info.bmiHeader.biWidth, sizeof( info.bmiHeader.biWidth ) );
	p_file->Read( &info.bmiHeader.biHeight, sizeof( info.bmiHeader.biHeight ) );
	p_file->Read( &info.bmiHeader.biPlanes, sizeof( info.bmiHeader.biPlanes ) );
	p_file->Read( &info.bmiHeader.biBitCount, sizeof( info.bmiHeader.biBitCount ) );
	p_file->Read( &info.bmiHeader.biCompression, sizeof( info.bmiHeader.biCompression ) );
	p_file->Read( &info.bmiHeader.biSizeImage, sizeof( info.bmiHeader.biSizeImage ) );
	p_file->Read( &info.bmiHeader.biXPelsPerMeter, sizeof( info.bmiHeader.biXPelsPerMeter ) );
	p_file->Read( &info.bmiHeader.biYPelsPerMeter, sizeof( info.bmiHeader.biYPelsPerMeter ) );
	p_file->Read( &info.bmiHeader.biClrUsed, sizeof( info.bmiHeader.biClrUsed ) );
	p_file->Read( &info.bmiHeader.biClrImportant, sizeof( info.bmiHeader.biClrImportant ) );

	if (infosize > 40)
	{
		if ( p_file->Read( &info.bmiColors, infosize - 40 ) == false )
		{
			ASSERT( 0, "Couldn't read the bitmap header - return NULL..." );

			return NULL;
		}
	}

	// Now that we have all the header info read in, allocate memory for
	// the bitmap and read *it* in...                                    
	if ( ( bitsize = info.bmiHeader.biSizeImage ) == 0 )
	{
		bitsize = ( info.bmiHeader.biWidth * info.bmiHeader.biBitCount + 7 ) / 8 * abs( info.bmiHeader.biHeight );
	}

	if ( ( bits = new u8[ bitsize ] ) == NULL )
	{
		ASSERT( 0, "Couldn't allocate memory - return NULL!" );

		return NULL;
	}

	if ( p_file->Read( bits, bitsize ) == false )
	{
		ASSERT( 0, "Couldn't read bitmap - free memory and return NULL!" );

		delete bits;

		return NULL;
	}

	const s32	height( abs( info.bmiHeader.biHeight ) );

	p_texture = new CTexture();

	if ( p_texture != NULL )
	{
		if ( p_texture->Init( info.bmiHeader.biWidth, height ) == false )
		{
			SAFE_DELETE( p_texture );
		}
		else
		{
			const u32	bpp( info.bmiHeader.biBitCount >> 3 );
			u8 *		p_dst( p_texture->m_pBuffer );

			for ( s32 y = 0; y < height; ++y )
			{
				for ( s32 x = 0; x < info.bmiHeader.biWidth; ++x )
				{
					const u32	dst_offset( ( x << 2 ) );
					const u32	src_offset( ( ( x + ( ( height - 1 - y ) * info.bmiHeader.biWidth ) ) * bpp ) );

					p_dst[ dst_offset + 0 ] = bits[ src_offset + 2 ];
					p_dst[ dst_offset + 1 ] = bits[ src_offset + 1 ];
					p_dst[ dst_offset + 2 ] = bits[ src_offset + 0 ];

					if ( bpp == 3 )
					{
						p_dst[ dst_offset + 3 ] = 0xff;
					}
					else
					{
						p_dst[ dst_offset + 3 ] = bits[ src_offset + 3 ];
					}
				}

				p_dst += ( p_texture->m_nCanvasWidth << 2 );
			}
		}
	}

	SAFE_RDELETE( bits );

	return p_texture;
}

//*******************************  END OF FILE  ************************************
