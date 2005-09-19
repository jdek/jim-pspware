/***********************************************************************************

  Module :	CImageFileHandler.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 07 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CImageFileHandler.h"
#include "CMessageBox.h"
#include "CTextureManager.h"
#include "CGfx.h"
#include "CInput.h"
#include "CFrameWork.h"
#include "CRenderable.h"
#include "CFont.h"
#include "CHUD.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
static const s32	MAX_UV( 512 );
static const float	SCALE_SPEED( 0.5f );

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************
static const sFileExtensionInfo	s_FileExtensionInfo =
{
	"View",
	CSkinManager::SC_ICON_IMAGE
};

CTexture *	CImageFileHandler::s_pTexture( NULL );
V2			CImageFileHandler::s_Size( 0.f, 0.f );
float		CImageFileHandler::s_fScale( 1.f );

static bool	s_bFlip( false );
static bool	s_bMirror( false );

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
bool	CImageFileHandler::Execute( const CString & file )
{
	s_pTexture = CTextureManager::Create( file );

	if ( s_pTexture != NULL )
	{
		//
		//	Register our render callback
		//
		CRenderable::Register( CRenderable::RO_WINDOWS, CImageFileHandler::Render );

		//
		//	Draw the image in our own special loop
		//
		CProcess::Pause( true );

		s_fScale = 1.f;
		s_bFlip = false;
		s_bMirror = false;

		CalculateSize();

		CHUD::SetButton( CHUD::BUTTON_START, "Hide UI" );
		CHUD::SetButtons( "", "Quit", "Flip", "Mirror" );

		while ( CInput::IsButtonClicked( CInput::CIRCLE ) == false )
		{
//			const float	delta( CFrameWork::GetElapsedTime() );

			CFrameWork::Process();

/*			if ( CInput::IsButtonDown( CInput::RTRIGGER ) == true )
			{
				s_fScale += SCALE_SPEED * delta;
			}

			if ( CInput::IsButtonDown( CInput::LTRIGGER ) == true )
			{
				s_fScale -= SCALE_SPEED * delta;

				SETMIN( s_fScale, 1.f );
			}*/

			if ( CFileHandler::MultiSelection() == true )
			{
				CString	file( "" );

				if ( CInput::IsButtonClicked( CInput::LTRIGGER ) == true )
				{
					file = CFileHandler::GetPrevFile();
				}

				if ( CInput::IsButtonClicked( CInput::RTRIGGER ) == true )
				{
					file = CFileHandler::GetNextFile();
				}

				if ( file.IsEmpty() == false )
				{
					SAFE_DELETE( s_pTexture );

					s_pTexture = CTextureManager::Create( file );

					if ( s_pTexture == NULL )
					{
						break;
					}

					s_fScale = 1.f;
					s_bFlip = false;
					s_bMirror = false;

					CalculateSize();
				}
			}

			if ( CInput::IsButtonClicked( CInput::SQUARE ) == true )
			{
				s_bFlip = ( s_bFlip == false );
			}

			if ( CInput::IsButtonClicked( CInput::TRIANGLE ) == true )
			{
				s_bMirror = ( s_bMirror == false );
			}

			if ( CInput::IsButtonClicked( CInput::START ) == true )
			{
				CHUD::Show( ( CHUD::IsVisible() == false ) );
			}
		}

		//
		//	Remove our render callback
		//
		CRenderable::UnRegister( CRenderable::RO_WINDOWS, CImageFileHandler::Render );

		SAFE_DELETE( s_pTexture );

		CProcess::Pause( false );

		CHUD::Show( true );

		return true;
	}

	CErrorMessage	box( "Invalid image file!" );

	box.Show();

	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
const sFileExtensionInfo &	CImageFileHandler::Information( const CString & file )
{
	return s_FileExtensionInfo;
}

//**********************************************************************************
//	Calculate the scale to fit the image on screen
//**********************************************************************************
void	CImageFileHandler::CalculateSize()
{
	if ( s_pTexture->m_nWidth <= CGfx::s_ScreenWidth && s_pTexture->m_nHeight <= CGfx::s_ScreenHeight )
	{
		s_Size.x = s_pTexture->m_nWidth;
		s_Size.y = s_pTexture->m_nHeight;
	}
	else
	{
		s_Size.x = static_cast< float >( CGfx::s_ScreenWidth ) / s_pTexture->m_nWidth;
		s_Size.y = static_cast< float >( CGfx::s_ScreenHeight ) / s_pTexture->m_nHeight;

		if ( s_Size.x < s_Size.y )
		{
			s_Size.x = s_pTexture->m_nWidth * s_Size.x;
			s_Size.y = s_pTexture->m_nHeight * s_Size.x;
		}
		else
		{
			s_Size.x = s_pTexture->m_nWidth * s_Size.y;
			s_Size.y = s_pTexture->m_nHeight * s_Size.y;
		}
	}
}

//**********************************************************************************
//	
//**********************************************************************************
void	CImageFileHandler::Render()
{
	s32			width( s_pTexture->m_nWidth );
	s32			height( s_pTexture->m_nHeight );
	const V2	pos( 0.5f * ( CGfx::s_ScreenWidth - ( s_Size.x * s_fScale ) ), 0.5f * ( CGfx::s_ScreenHeight - ( s_Size.y * s_fScale ) ) );
	const float	x_scale( ( s_Size.x * s_fScale ) / width );
	const float	y_scale( ( s_Size.y * s_fScale ) / height );

	CGfx::DrawQuad( V2( 0.f, 0.f ), V2( CGfx::s_ScreenWidth, CGfx::s_ScreenHeight ), 0x80000000 );

	//
	//	Render the image in 512x512 chunks as the PSP doesn't
	//	support textures greater than 512x512 :(
	//
	while( height > 0 )
	{
		width = s_pTexture->m_nWidth;

		while( width > 0 )
		{
			s32		rwidth( width );

			if ( s_bFlip )
			{
				const s32	remainder( rwidth % MAX_UV );

				if ( width == s_pTexture->m_nWidth )
				{
					rwidth = remainder;
				}
				else if ( width == remainder )
				{
					rwidth = MAX_UV;
				}
			}

			sVertexTexturedColor *	p_vert;
			const s32				x_size( s_pTexture->m_nWidth - rwidth );
			const s32				y_size( s_pTexture->m_nHeight - height );
			s32						u_step( rwidth >= MAX_UV ? MAX_UV : rwidth );
			s32						v_step( height >= MAX_UV ? MAX_UV : height );
			s32						u[ 2 ], v[ 2 ];

			CGfx::GetPolyList( 2, &p_vert );

			u[ 0 ] = 0;
			v[ 0 ] = 0;
			u[ 1 ] = u_step;
			v[ 1 ] = v_step;

			if ( s_bFlip == true )
			{
				const s32	temp( u[ 0 ] );
				u[ 0 ] = u[ 1 ];
				u[ 1 ] = temp;
			}

			if ( s_bMirror == true )
			{
				const s32	temp( v[ 0 ] );
				v[ 0 ] = v[ 1 ];
				v[ 1 ] = temp;
			}

			p_vert[ 0 ].u = u[ 0 ];
			p_vert[ 0 ].v = v[ 0 ];
			p_vert[ 0 ].x = pos.x + ( x_size * x_scale );
			p_vert[ 0 ].y = pos.y + ( y_size * y_scale );
			p_vert[ 0 ].z = 0.f;
			p_vert[ 0 ].color = 0xffffffff;

			p_vert[ 1 ].u = u[ 1 ];
			p_vert[ 1 ].v = v[ 1 ];
			p_vert[ 1 ].x = pos.x + ( ( x_size + u_step ) * x_scale );
			p_vert[ 1 ].y = pos.y + ( ( y_size + v_step ) * y_scale );
			p_vert[ 1 ].z = 0.f;
			p_vert[ 1 ].color = 0xffffffff;

			sceGuEnable( GU_TEXTURE_2D );
			sceGuShadeModel( GU_SMOOTH );
			sceGuTexMode( GU_PSM_8888, 0, 0, 0 );
			sceGuTexImage( 0, MAX_UV, MAX_UV, s_pTexture->m_nCanvasWidth, s_pTexture->m_pBuffer + ( 4 * ( x_size + ( y_size * s_pTexture->m_nCanvasWidth ) ) ) );
			sceGuTexFunc( GU_TFX_MODULATE, GU_TCC_RGBA );
			sceGuTexWrap( GU_CLAMP, GU_CLAMP );
			sceGuTexFilter( GU_LINEAR, GU_LINEAR );
			sceGuTexScale( 1.0f, 1.0f );
			sceGuTexOffset( 0.0f, 0.0f );
			sceGuAmbientColor( 0xffffffff );
			sceGuDrawArray( GU_SPRITES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, p_vert );

			width -= MAX_UV;
		}

		height -= MAX_UV;
	}

	//
	//	Display some image information
	//
/*	CString	text;

	text.Printf( "Width = %d\nHeight = %d\n", s_pTexture->m_nWidth, s_pTexture->m_nHeight );

	CFont::GetDefaultFont()->Print( text, V2( 17.f, 17.f ), 0xff000000 );
	CFont::GetDefaultFont()->Print( text, V2( 16.f, 16.f ), 0xffffffff );*/
}

//*******************************  END OF FILE  ************************************
