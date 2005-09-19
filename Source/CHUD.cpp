/***********************************************************************************

  Module :	CHUD.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 07 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CHUD.h"
#include "CFrameWork.h"
#include "CRenderable.h"
#include "CGfx.h"
#include "CFont.h"
#include "CSkinManager.h"
#include "CTextureManager.h"
#include "CUSBManager.h"
#include "CMusicFileHandler.h"
#include "CInput.h"
#include "CRenderable.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
static const CString	VERSION_STRING( "v0.08" );
static const float		HUD_HEIGHT( 70.f );
static const float		MUSIC_SCROLL_SPEED( 16.f );

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************
bool					CHUD::s_bVisible( true );
CHUD *					CHUD::s_pInstance( NULL );
CString					CHUD::s_szButtonText[ MAX_BUTTONS ] =	{ "", "", "", "", "", "" };
bool					CHUD::s_bShowProgressBar( false );
float					CHUD::s_fProgressBar( 0.f );
CString					CHUD::s_szProgressTitle( "" );

static const CString	s_szUSBConnected( "USB connected" );
static s32				s_BatteryFlash( 0 );
static float			s_MusicOffset( 0.f );

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
void	CHUD::Create()
{
	ASSERT( s_pInstance == NULL, "HUD has already been created!" );

	s_pInstance = new CHUD();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CHUD::Destroy()
{
	ASSERT( s_pInstance != NULL, "HUD hasn't been created yet!" );

	SAFE_DELETE( s_pInstance );
}

//**********************************************************************************
//	
//**********************************************************************************
CHUD::CHUD()
{
	CRenderable::Register( CRenderable::RO_HUD, Render );
}

//**********************************************************************************
//	
//**********************************************************************************
CHUD::~CHUD()
{
	CRenderable::UnRegister( CRenderable::RO_HUD, Render );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CHUD::Render()
{
	if ( s_pInstance != NULL )
	{
		if ( s_bVisible == true )
		{
			s_pInstance->RenderInternal();
		}
	}

	//
	//	Debug information
	//
/*	if ( CInput::IsButtonDown( CInput::LTRIGGER ) == true )
	{
		CString	text;

		text.Printf( "Mem Free: %d\nMax Block %d\n", sceKernelTotalFreeMemSize(), sceKernelTotalFreeMemSize() );

		CFont::GetDefaultFont()->Print( text, V2( 0.f, 0.f ), 0xffffffff );
	}*/
}

//**********************************************************************************
//	
//**********************************************************************************
static void	DrawButton( const V2 & in_pos, CSkinManager::eSkinComponent button_icon, const CString & text, bool left_justify )
{
	V2					pos( in_pos );
	V2					icon_size( 0.f, 0.f );
	CTexture * const	p_button_texture( CSkinManager::GetComponent( button_icon )->GetTexture() );
	const V2			text_size( CFont::GetDefaultFont()->GetStringSize( text ) );

	if ( p_button_texture != NULL )
	{
		icon_size = V2( 16.f, 16.f );

		CGfx::DrawQuad( p_button_texture, pos, icon_size, 0xffffffff );
	}

	if ( left_justify == false )
	{
		CFont::GetDefaultFont()->Print( text, pos + V2( icon_size.x + 2, -0.5f * ( text_size.y - icon_size.y ) ), 0xffffffff, true );
	}
	else
	{
		CFont::GetDefaultFont()->Print( text, pos - V2( text_size.x + 2, 0.5f * ( text_size.y - icon_size.y ) ), 0xffffffff, true );
	}
}

//**********************************************************************************
//	
//**********************************************************************************
void	CHUD::RenderInternal()
{
	const float	hud_ypos( CGfx::s_ScreenHeight - HUD_HEIGHT );

	//
	//	Draw the pad buttons
	//
	static const float	BUTTONS_X( 400.f );

	DrawButton( V2( BUTTONS_X, hud_ypos + 0.f ), CSkinManager::SC_BUTTON_TRIANGLE, s_szButtonText[ BUTTON_TRIANGLE ], true );
	DrawButton( V2( BUTTONS_X - 16.f, hud_ypos + 16.f ), CSkinManager::SC_BUTTON_SQUARE, s_szButtonText[ BUTTON_SQUARE ], true );
	DrawButton( V2( BUTTONS_X + 16.f, hud_ypos + 16.f ), CSkinManager::SC_BUTTON_CIRCLE, s_szButtonText[ BUTTON_CIRCLE ], false );
	DrawButton( V2( BUTTONS_X, hud_ypos + 32.f ), CSkinManager::SC_BUTTON_CROSS, s_szButtonText[ BUTTON_CROSS ], false );
	DrawButton( V2( BUTTONS_X + 16.f, hud_ypos + 48.f ), CSkinManager::SC_BUTTON_START, s_szButtonText[ BUTTON_START ], false );
	DrawButton( V2( BUTTONS_X, hud_ypos + 48.f ), CSkinManager::SC_BUTTON_SELECT, s_szButtonText[ BUTTON_SELECT ], true );

	//
	//	Draw the battery meter
	//
	CTexture *	p_battery;
	const s32	battery_power( scePowerGetBatteryLifePercent() );

	if ( battery_power <= 30 )			p_battery = CSkinManager::GetComponent( CSkinManager::SC_BATTERY_01 )->GetTexture();
	else if ( battery_power <= 60 )		p_battery = CSkinManager::GetComponent( CSkinManager::SC_BATTERY_02 )->GetTexture();
	else if ( battery_power <= 90 )		p_battery = CSkinManager::GetComponent( CSkinManager::SC_BATTERY_03 )->GetTexture();
	else								p_battery = CSkinManager::GetComponent( CSkinManager::SC_BATTERY_03 )->GetTexture();

	if ( scePowerIsBatteryCharging() == 0 )
	{
		s_BatteryFlash = 0;
	}
	else
	{
		s_BatteryFlash = ( s_BatteryFlash + 1 ) % 64;
	}

	if ( ( s_BatteryFlash >> 5 ) == 0 )
	{
		CGfx::DrawQuad( p_battery, V2( CGfx::s_ScreenWidth - 48.f, 16.f ), V2( p_battery->m_nCanvasWidth, p_battery->m_nCanvasHeight ), 0x80ffffff );
	}

	//
	//	Draw the USB status
	//
	if ( CUSBManager::IsActive() == true )
	{
		const V2			usb_pos( 16.f, CGfx::s_ScreenHeight - 32.f );
		CTexture * const	p_usb( CSkinManager::GetComponent( CSkinManager::SC_ICON_USB )->GetTexture() );

		CGfx::DrawQuad( p_usb, usb_pos, V2( p_usb->m_nCanvasWidth, p_usb->m_nCanvasHeight ), 0xa0ffffff );

		if ( CUSBManager::CableConnected() == true && CUSBManager::ConnectionEstablished() == true )
		{
			const V2	text_size( CFont::GetDefaultFont()->GetStringSize( s_szUSBConnected ) );

			CFont::GetDefaultFont()->Print( s_szUSBConnected, usb_pos + V2( p_usb->m_nWidth + 2, -0.5f * ( text_size.y - p_usb->m_nHeight ) ), 0xffffffff );
		}
	}

	//
	//	If music is playing, display the info
	//
	if ( CMusicFileHandler::IsPlaying() == true )
	{
		const CString &	filename( CMusicFileHandler::GetFilePlaying() );
		const CString	play_time( CMusicFileHandler::GetPlayingTime() );
		const V2		play_text_size( CFont::GetDefaultFont()->GetStringSize( play_time ) );
		const V2		file_text_size( CFont::GetDefaultFont()->GetStringSize( filename ) + V2( 32.f, 0.f ) );
		const V2		music_size( 200.f, file_text_size.y );
		const V2		music_pos( 0.5f * ( CGfx::s_ScreenWidth - music_size.x ), CGfx::s_ScreenHeight - 16.f - music_size.y );

		CFont::GetDefaultFont()->Print( play_time, music_pos - V2( 0.f, play_text_size.y ), 0xffffffff );

		CGfx::DrawQuad( music_pos, music_size, 0x40000000 );

		if ( file_text_size.x > music_size.x )
		{
			CGfx::SetClipRegion( music_pos, music_size );

			CFont::GetDefaultFont()->Print( filename, music_pos + V2( s_MusicOffset, 0.f ), 0xffffffff );
			CFont::GetDefaultFont()->Print( filename, music_pos + V2( s_MusicOffset + file_text_size.x, 0.f ), 0xffffffff );

			CGfx::DisableClipRegions();

			s_MusicOffset -= MUSIC_SCROLL_SPEED * CFrameWork::GetElapsedTime();

			if ( s_MusicOffset < -file_text_size.x )
			{
				s_MusicOffset += file_text_size.x;
			}
		}
		else
		{
			CFont::GetDefaultFont()->Print( filename, music_pos, 0xffffffff );
		}
	}
	else
	{
		s_MusicOffset = 0.f;
	}

	//
	//	Draw the progress bar
	//
	if ( s_bShowProgressBar == true )
	{
		CTexture * const		p_progress_full( CSkinManager::GetComponent( CSkinManager::SC_PROGRESS_BAR_FULL )->GetTexture() );
		CTexture * const		p_progress_empty( CSkinManager::GetComponent( CSkinManager::SC_PROGRESS_BAR_EMPTY )->GetTexture() );
		const float				progress_u( p_progress_full->m_nWidth * s_fProgressBar );
		const V2				progress_pos( CSkinManager::GetV2( "hud", "progress_pos", V2( 140.f, 220.f ) ) );
		sVertexTexturedColor *	p_full_verts;
		sVertexTexturedColor *	p_empty_verts;

		CGfx::GetPolyList( 2, &p_full_verts );
		CGfx::GetPolyList( 2, &p_empty_verts );

		p_full_verts[ 0 ].u = 0.f;
		p_full_verts[ 0 ].v = 0.f;
		p_full_verts[ 0 ].color = 0xffffffff;
		p_full_verts[ 0 ].x = progress_pos.x;
		p_full_verts[ 0 ].y = progress_pos.y;
		p_full_verts[ 0 ].z = 0.f;

		p_full_verts[ 1 ].u = progress_u;
		p_full_verts[ 1 ].v = p_progress_full->m_nHeight;
		p_full_verts[ 1 ].color = 0xffffffff;
		p_full_verts[ 1 ].x = progress_pos.x + progress_u;
		p_full_verts[ 1 ].y = progress_pos.y + p_progress_full->m_nHeight;
		p_full_verts[ 1 ].z = 0.f;

		p_empty_verts[ 0 ].u = progress_u;
		p_empty_verts[ 0 ].v = 0.f;
		p_empty_verts[ 0 ].color = 0xffffffff;
		p_empty_verts[ 0 ].x = progress_pos.x + progress_u;
		p_empty_verts[ 0 ].y = progress_pos.y;
		p_empty_verts[ 0 ].z = 0.f;

		p_empty_verts[ 1 ].u = p_progress_empty->m_nWidth;
		p_empty_verts[ 1 ].v = p_progress_empty->m_nHeight;
		p_empty_verts[ 1 ].color = 0xffffffff;
		p_empty_verts[ 1 ].x = progress_pos.x + p_progress_empty->m_nWidth;
		p_empty_verts[ 1 ].y = progress_pos.y + p_progress_empty->m_nHeight;
		p_empty_verts[ 1 ].z = 0.f;

		CGfx::DrawSprite( p_progress_full, p_full_verts, 2 );
		CGfx::DrawSprite( p_progress_empty, p_empty_verts, 2 );

		const V2	text_size( CFont::GetDefaultFont()->GetStringSize( s_szProgressTitle ) );

		CFont::GetDefaultFont()->Print( s_szProgressTitle, V2( progress_pos.x + ( 0.5f * ( p_progress_empty->m_nWidth - text_size.x ) ), progress_pos.y - text_size.y - 2.f ), 0xffffffff );
	}

	//
	//	Draw version number
	//
	const V2	text_size( CFont::GetDefaultFont()->GetStringSize( VERSION_STRING ) );

	CFont::GetDefaultFont()->Print(	VERSION_STRING, V2( 0.f, CGfx::s_ScreenHeight - text_size.y ), 0x60ffffff, 1.f );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CHUD::SetButton( eButton button, const CString & text )
{
	s_szButtonText[ button ] = text;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CHUD::SetButtons( const CString & cross, const CString & circle, const CString & square, const CString & triangle )
{
	SetButton( BUTTON_CROSS, cross );
	SetButton( BUTTON_CIRCLE, circle );
	SetButton( BUTTON_SQUARE, square );
	SetButton( BUTTON_TRIANGLE, triangle );
}

//**********************************************************************************
//	
//**********************************************************************************
float	CHUD::GetProgressBar()
{
	return s_fProgressBar;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CHUD::SetProgressBar( float val )
{
	s_fProgressBar = val;

	CRenderable::Render();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CHUD::ShowProgressBar( bool show, const CString & title )
{
	s_fProgressBar = 0.f;
	s_bShowProgressBar = show;
	s_szProgressTitle = title;

	CRenderable::Render();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CHUD::Show( bool show )
{
	s_bVisible = show;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CHUD::IsVisible()
{
	return s_bVisible;
}

//*******************************  END OF FILE  ************************************
