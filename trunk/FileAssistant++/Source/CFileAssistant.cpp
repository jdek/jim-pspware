/***********************************************************************************

  Module :	CFileAssistant.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 05 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CFileAssistant.h"
#include "CFrameWork.h"
#include "CFileHandler.h"
#include "CGfx.h"
#include "CInput.h"
#include "CImageFileHandler.h"
#include "CMusicFileHandler.h"
#include "CExecutableFileHandler.h"
#include "CHUD.h"
#include "COptionsMenu.h"
#include "CConfigFile.h"
#include "CTextFileHandler.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
static const float	SCALE_SPEED( 8.f );

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************
CFileAssistant *	CFileAssistant::s_pInstance( NULL );

static CVarString	CVAR_SRC_PATH( "src_path", "" );
static CVarString	CVAR_DST_PATH( "dst_path", "" );

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
void	CFileAssistant::Create()
{
	ASSERT( s_pInstance == NULL, "Instance already created" );

	s_pInstance = new CFileAssistant();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileAssistant::Destroy()
{
	SAFE_DELETE( s_pInstance );
}

//**********************************************************************************
//	
//**********************************************************************************
CFileAssistant *	CFileAssistant::Get()
{
	ASSERT( s_pInstance != NULL, "Trying to access a NULL file assistant" );

	return s_pInstance;
}

//**********************************************************************************
//	
//**********************************************************************************
CFileAssistant::CFileAssistant()
:	m_pSrcList( NULL )
,	m_pDstList( NULL )
,	m_pListFocus( NULL )
,	m_fSrcScale( 0.f )
,	m_fDstScale( 0.f)
,	m_bSrcInFocus( true )
{
	//
	//	Create the HUD
	//
	CHUD::Create();

	//
	//	Open any file handlers
	//
	CMusicFileHandler::Open();

	//
	//	Register the file extensions we're interested in
	//
	CFileHandler::RegisterExtension( "txt", CTextFileHandler::Execute, CTextFileHandler::Information );
	CFileHandler::RegisterExtension( "xml", CTextFileHandler::Execute, CTextFileHandler::Information );

	CFileHandler::RegisterExtension( "pbp", CPBPFileHandler::Execute, CPBPFileHandler::Information, CPBPFileHandler::BackgroundCallback );
	CFileHandler::RegisterExtension( "elf", CELFFileHandler::Execute, CELFFileHandler::Information );
	CFileHandler::RegisterExtension( "prx", CPRXFileHandler::Execute, CPRXFileHandler::Information );
	CFileHandler::RegisterExtension( "bin", CBINFileHandler::Execute, CBINFileHandler::Information );

	CFileHandler::RegisterExtension( "bmp", CImageFileHandler::Execute, CImageFileHandler::Information );
	CFileHandler::RegisterExtension( "jpg", CImageFileHandler::Execute, CImageFileHandler::Information );
	CFileHandler::RegisterExtension( "png", CImageFileHandler::Execute, CImageFileHandler::Information );
	CFileHandler::RegisterExtension( "tga", CImageFileHandler::Execute, CImageFileHandler::Information );

	CFileHandler::RegisterExtension( "mp3", CMusicFileHandler::Execute, CMusicFileHandler::Information );
	CFileHandler::RegisterExtension( "ogg", CMusicFileHandler::Execute, CMusicFileHandler::Information );
	CFileHandler::RegisterExtension( "ahx", CMusicFileHandler::Execute, CMusicFileHandler::Information );
	CFileHandler::RegisterExtension( "ym",  CMusicFileHandler::Execute, CMusicFileHandler::Information );
	CFileHandler::RegisterExtension( "mod", CMusicFileHandler::Execute, CMusicFileHandler::Information );

	//
	//	Create the two directory windows
	//
	m_pSrcList = new CDirectoryList();
	m_pDstList = new CDirectoryList();

	m_pSrcList->SetPos( V2( 16.f, 16.f ) );
	m_pSrcList->SetSize( CSkinManager::GetV2( "directory_list", "src_list_size", V2( 320.f, 220.f ) ) );
	m_pSrcList->SetFocus( true );

	m_pDstList->SetPos( V2( 144.f, 16.f ) );
	m_pDstList->SetSize( CSkinManager::GetV2( "directory_list", "dst_list_size", V2( 320.f, 220.f ) ) );
	m_pDstList->SetFocus( false );

	if ( CVAR_SRC_PATH.Get().IsEmpty() == false )		m_pSrcList->SetPath( CVAR_SRC_PATH.Get() );
	else												m_pSrcList->SetDrive( CDirectoryList::ALL_DRIVES );

	if ( CVAR_DST_PATH.Get().IsEmpty() == false )		m_pDstList->SetPath( CVAR_DST_PATH.Get() );
	else												m_pDstList->SetDrive( CDirectoryList::ALL_DRIVES );

	m_pListFocus = m_pSrcList;

	//
	//	Set the buttons
	//
	CHUD::SetButton( CHUD::BUTTON_START, "Options" );
	CHUD::SetButton( CHUD::BUTTON_SELECT, "Reboot" );
}

//**********************************************************************************
//	
//**********************************************************************************
CFileAssistant::~CFileAssistant()
{
	SAFE_DELETE( m_pSrcList );
	SAFE_DELETE( m_pDstList );

	//
	//	Close any file handlers
	//
	CMusicFileHandler::Close();

	//
	//	Shut the HUD down
	//
	CHUD::Destroy();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileAssistant::Process()
{
	const float	delta( CFrameWork::GetElapsedTime() );
	const float	min_scale( CSkinManager::GetFloat( "directory_list", "min_scale", 0.75f ) );
	const float	max_scale( CSkinManager::GetFloat( "directory_list", "max_scale", 1.f ) );
	const V2	src_pos_on( CSkinManager::GetV2( "directory_list", "src_list_pos_on", V2( 16.f, 16.f ) ) );
	const V2	dst_pos_on( CSkinManager::GetV2( "directory_list", "dst_list_pos_on", V2( 144.f, 16.f ) ) );
	const V2	src_pos_off( CSkinManager::GetV2( "directory_list", "src_list_pos_off", V2( 16.f, 16.f ) ) );
	const V2	dst_pos_off( CSkinManager::GetV2( "directory_list", "dst_list_pos_off", V2( 144.f, 16.f ) ) );
	V2			src_pos( m_pSrcList->GetPos() );
	V2			dst_pos( m_pDstList->GetPos() );
	V2			wanted_src_pos, wanted_dst_pos;

	if ( m_bSrcInFocus == true )
	{
		wanted_src_pos = src_pos_on;
		wanted_dst_pos = dst_pos_off;

		m_fSrcScale += ( max_scale - m_fSrcScale ) * SCALE_SPEED * delta;
		m_fDstScale += ( min_scale - m_fDstScale ) * SCALE_SPEED * delta;
	}
	else
	{
		wanted_src_pos = src_pos_off;
		wanted_dst_pos = dst_pos_on;

		m_fSrcScale += ( min_scale - m_fSrcScale ) * SCALE_SPEED * delta;
		m_fDstScale += ( max_scale - m_fDstScale ) * SCALE_SPEED * delta;
	}

	if ( src_pos.DistanceSq( wanted_src_pos ) < SQUARE( 4.f ) )	src_pos = wanted_src_pos;
	else														src_pos += ( wanted_src_pos - src_pos ) * SCALE_SPEED * delta;

	if ( dst_pos.DistanceSq( wanted_dst_pos ) < SQUARE( 4.f ) )	dst_pos = wanted_dst_pos;
	else														dst_pos += ( wanted_dst_pos - dst_pos ) * SCALE_SPEED * delta;

	m_pSrcList->SetPos( src_pos );
	m_pSrcList->SetSize( CSkinManager::GetV2( "directory_list", "src_list_size", V2( 320.f, 220.f ) ) );

	m_pDstList->SetPos( dst_pos );
	m_pDstList->SetSize( CSkinManager::GetV2( "directory_list", "dst_list_size", V2( 320.f, 220.f ) ) );

	SETMIN( m_fSrcScale, min_scale );
	SETMAX( m_fSrcScale, max_scale );

	SETMIN( m_fDstScale, min_scale );
	SETMAX( m_fDstScale, max_scale );

	m_pSrcList->SetScale( m_fSrcScale );
	m_pDstList->SetScale( m_fDstScale );

	//
	//	Constantly update the src and dst paths
	//
	if ( m_pSrcList->IsDriveList() == false )
	{
		CVAR_SRC_PATH = m_pSrcList->GetCurrentPath();
	}

	if ( m_pDstList->IsDriveList() == false )
	{
		CVAR_DST_PATH = m_pDstList->GetCurrentPath();
	}
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileAssistant::ProcessInput()
{
	if ( m_pSrcList->HasFocus() == true || m_pDstList->HasFocus() == true )
	{
		CDirectoryList * const	p_old_list( m_pListFocus );

		if ( CInput::IsButtonClicked( CInput::START ) == true )
		{
			COptionsMenu * const	p_options_menu( new COptionsMenu() );

			p_options_menu->SetFocus( true );
		}

		if ( CInput::IsButtonClicked( CInput::LEFT ) == true )
		{
			m_bSrcInFocus = true;
			m_pListFocus = m_pSrcList;
		}

		if ( CInput::IsButtonClicked( CInput::RIGHT ) == true )
		{
			m_bSrcInFocus = false;
			m_pListFocus = m_pDstList;
		}

		if ( m_pListFocus != p_old_list )
		{
			m_pListFocus->SetFocus( true );
			p_old_list->SetFocus( false );
		}
	}
}

//**********************************************************************************
//	
//**********************************************************************************
CDirectoryList *	CFileAssistant::GetSrcList() const
{
	return m_pSrcList;
}

//**********************************************************************************
//	
//**********************************************************************************
CDirectoryList *	CFileAssistant::GetDstList() const
{
	return m_pDstList;
}

//**********************************************************************************
//	
//**********************************************************************************
CDirectoryList *	CFileAssistant::GetFocusList() const
{
	return m_pListFocus;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileAssistant::SetListFocus( bool focus )
{
	m_pListFocus->SetFocus( focus );
}

//*******************************  END OF FILE  ************************************
