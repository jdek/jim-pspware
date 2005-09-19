/***********************************************************************************

  Module :	CFont.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 03 August 2005 T Swann

***********************************************************************************/

#ifndef CFONT_H_
#define CFONT_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CVector.h"
#include "CString.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
class CSkinTexture;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CFont
{
	private:

		enum eFontMapDimensions
		{
			FONT_MAP_WIDTH	= 31,
			FONT_MAP_HEIGHT	= 3,
		};

		struct sGlyphData
		{
			u32	x, y;
			u32	m_Width;
		};

	public:

		static void			Open();
		static void			Close();
		static CFont *		Create( const V2 & size );

		static CFont *		GetDefaultFont();

	public:

		~CFont();

		void				Print( const CString & string, const V2 & pos, ARGB color, float scale = 1.f, bool shadow = false );

		V2					GetStringSize( const CString & string ) const;

	private:

		CFont( const V2 & size );

		void				CreateGlyphData();

		const sGlyphData *	FindGlyph( char glyph ) const;

	public:

		CSkinTexture *		m_pTexture;
		V2					m_Size;
		sGlyphData			m_GlyphData[ FONT_MAP_WIDTH * FONT_MAP_HEIGHT ];

	private:

		static CFont *		s_pDefaultFont;
		static const u8		s_FontMap[ FONT_MAP_HEIGHT ][ FONT_MAP_WIDTH ];
		static u32			s_LookupTable[ 255 ];
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CFONT_H_ */
