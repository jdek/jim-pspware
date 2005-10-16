/***********************************************************************************

  Module :	CSkinManager.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 04 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CSkinManager.h"
#include "CTextureManager.h"
#include "CFont.h"
#include "CFrameWork.h"
#include "CConfigFile.h"
#include "TinyXML.h"

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
static CVarString	CVAR_SKIN( "skin", "Default" );

bool				CSkinManager::s_bConfigLoaded( false );
TiXmlDocument *		CSkinManager::s_pSkinConfigFile( NULL );
const CString		CSkinManager::s_szSkinFolder( "Data/Skins/" );
CSkinTexture *		CSkinManager::s_pComponents[ CSkinManager::MAX_SKIN_COMPONENTS ];
const CString		CSkinManager::s_szComponentNames[ MAX_SKIN_COMPONENTS ] =
{
	"font.png",					// SC_FONT,
	"background.png",			// SC_BACKGROUND,
	"wnd_bottom.png",			// SC_WND_BOTTOM,
	"wnd_bottom_left.png",		// SC_WND_BOTTOM_LEFT,
	"wnd_bottom_right.png",		// SC_WND_BOTTOM_RIGHT,
	"wnd_center.png",			// SC_WND_CENTER,
	"wnd_left.png",				// SC_WND_LEFT,
	"wnd_right.png",			// SC_WND_RIGHT,
	"wnd_top.png",				// SC_WND_TOP,
	"wnd_top_left.png",			// SC_WND_TOP_LEFT,
	"wnd_top_right.png",		// SC_WND_TOP_RIGHT,
	"icon_audio.png",			// SC_ICON_AUDIO,
	"icon_back.png",			// SC_ICON_BACK,
	"icon_drive.png",			// SC_ICON_DRIVE,
	"icon_executable.png",		// SC_ICON_EXECUTABLE,
	"icon_file.png",			// SC_ICON_FILE,
	"icon_folder.png",			// SC_ICON_FOLDER,
	"icon_image.png",			// SC_ICON_IMAGE,
	"icon_movie.png",			// SC_ICON_MOVIE,
	"icon_text.png",			// SC_ICON_TEXT,
	"icon_USB.png",				// SC_ICON_USB,
	"button_cross.png",			// SC_BUTTON_CROSS,
	"button_circle.png",		// SC_BUTTON_CIRCLE,
	"button_square.png",		// SC_BUTTON_SQUARE,
	"button_triangle.png",		// SC_BUTTON_TRIANGLE,
	"button_start.png",			// SC_BUTTON_START,
	"button_select.png",		// SC_BUTTON_SELECT,
	"battery_00.png",			// SC_BATTERY_00,
	"battery_01.png",			// SC_BATTERY_01,
	"battery_02.png",			// SC_BATTERY_02,
	"battery_03.png",			// SC_BATTERY_03,
	"progress_bar_empty.png",	// SC_PROGRESS_BAR_EMPTY,
	"progress_bar_full.png",	// SC_PROGRESS_BAR_FULL,
};

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//**********************************************************************************
//
//	Skin texture class implementation
//
//**********************************************************************************
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
CSkinTexture::CSkinTexture()
:	m_pTexture( NULL )
{
}

//**********************************************************************************
//	
//**********************************************************************************
CSkinTexture::~CSkinTexture()
{
	SAFE_DELETE( m_pTexture );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinTexture::Load( const CString & texture_name )
{
	SAFE_DELETE( m_pTexture );

	m_pTexture = CTextureManager::Create( texture_name );
}

//**********************************************************************************
//	
//**********************************************************************************
CTexture *	CSkinTexture::GetTexture() const
{
	return m_pTexture;
}


//**********************************************************************************
//**********************************************************************************
//
//	Skin manager class implementation
//
//**********************************************************************************
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinManager::Open()
{
	for ( u32 i = 0; i < MAX_SKIN_COMPONENTS; ++i )
	{
		s_pComponents[ i ] = new CSkinTexture();
	}

	SetCurrentSkin( *CVAR_SKIN );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinManager::Close()
{
	for ( u32 i = 0; i < MAX_SKIN_COMPONENTS; ++i )
	{
		SAFE_DELETE( s_pComponents[ i ] );
	}

	SAFE_DELETE( s_pSkinConfigFile );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinManager::SetCurrentSkin( const CString & skin_name )
{
	CVAR_SKIN = skin_name;

	CTextureManager::ResetVRAM();

	for ( u32 i = 0; i < MAX_SKIN_COMPONENTS; ++i )
	{
		const CString	component( s_szSkinFolder + skin_name + "/" + s_szComponentNames[ i ] );

		s_pComponents[ i ]->Load( component );
	}

	if ( s_pComponents[ SC_FONT ]->GetTexture() != NULL )
	{
		s_pComponents[ SC_FONT ]->GetTexture()->Upload();
	}

	if ( s_pComponents[ SC_BACKGROUND ]->GetTexture() != NULL )
	{
		s_pComponents[ SC_BACKGROUND ]->GetTexture()->Upload();
	}

	LoadConfig();
}

//**********************************************************************************
//	
//**********************************************************************************
CSkinTexture *	CSkinManager::GetComponent( eSkinComponent component )
{
	return s_pComponents[ component ];
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinManager::LoadConfig()
{
	SAFE_DELETE( s_pSkinConfigFile );

	s_pSkinConfigFile = new TiXmlDocument();

	ASSERT( s_pSkinConfigFile != NULL, "Failed to create the XML document" );

	if ( s_pSkinConfigFile != NULL )
	{
		const CString	config_file( s_szSkinFolder + CVAR_SKIN.Get() + "/skin.xml" );

		if ( s_pSkinConfigFile->LoadFile( config_file ) == true )
		{
			s_bConfigLoaded = true;
		}
		else
		{
			s_bConfigLoaded = false;

			TRACE( "%s\n", s_pSkinConfigFile->ErrorDesc() );

			ASSERT( 0, "Failed to load skin config file!" );
		}
	}
}

//**********************************************************************************
//	
//**********************************************************************************
V2	CSkinManager::GetV2( const CString & section, const CString & sub_section, const V2 & default_val )
{
	if ( s_bConfigLoaded == true )
	{
		TiXmlHandle		doc_handle( s_pSkinConfigFile );
		TiXmlElement *	p_xy( doc_handle.FirstChild( "skin" ).FirstChild( section ).FirstChild( sub_section ).Element() );

		if ( p_xy != NULL )
		{
			V2	val( default_val );

			if ( p_xy->Attribute( "x" ) != NULL )	val.x = atof( p_xy->Attribute( "x" ) );
			if ( p_xy->Attribute( "y" ) != NULL )	val.y = atof( p_xy->Attribute( "y" ) );

			return val;
		}
	}

	return default_val;
}

//**********************************************************************************
//	
//**********************************************************************************
ARGB	CSkinManager::GetColor( const CString & section, const CString & sub_section, const ARGB & default_val )
{
	if ( s_bConfigLoaded == true )
	{
		TiXmlHandle		doc_handle( s_pSkinConfigFile );
		TiXmlElement *	p_color( doc_handle.FirstChild( "skin" ).FirstChild( section ).FirstChild( sub_section ).Element() );

		if ( p_color != NULL )
		{
			ARGB	color( default_val );

			if ( p_color->Attribute( "a" ) != NULL )	color.a = atoi( p_color->Attribute( "a" ) );
			if ( p_color->Attribute( "r" ) != NULL )	color.r = atoi( p_color->Attribute( "r" ) );
			if ( p_color->Attribute( "g" ) != NULL )	color.g = atoi( p_color->Attribute( "g" ) );
			if ( p_color->Attribute( "b" ) != NULL )	color.b = atoi( p_color->Attribute( "b" ) );

			return color;
		}
	}

	return default_val;
}

//**********************************************************************************
//	
//**********************************************************************************
float	CSkinManager::GetFloat( const CString & section, const CString & sub_section, const float & default_val )
{
	if ( s_bConfigLoaded == true )
	{
		TiXmlHandle		doc_handle( s_pSkinConfigFile );
		TiXmlElement *	p_val( doc_handle.FirstChild( "skin" ).FirstChild( section ).FirstChild( sub_section ).Element() );

		if ( p_val != NULL )
		{
			float	val( default_val );

			if ( p_val->Attribute( "val" ) != NULL )	val = atof( p_val->Attribute( "val" ) );

			return val;
		}
	}

	return default_val;
}

//*******************************  END OF FILE  ************************************
