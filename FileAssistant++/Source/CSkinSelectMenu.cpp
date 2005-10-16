/***********************************************************************************

  Module :	CSkinSelectMenu.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 16 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CSkinSelectMenu.h"
#include "CWindowTable.h"
#include "CFont.h"
#include "CGfx.h"
#include "CHUD.h"
#include "CInput.h"
#include "CFileSystem.h"
#include "CSkinManager.h"
#include "CFrameWork.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
static const float	WINDOW_BORDER( 16.f );
static const V2		SELECTION_SIZE( 200.f, 100.f );

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

//**********************************************************************************
//**********************************************************************************
//
//	Skin list item implementation
//
//**********************************************************************************
//**********************************************************************************
class CSkinListItem : public CWindowTableItem
{
	public:

		CSkinListItem( const CString & skin_name );

		virtual V2			Render( V2 pos, bool highlight, float scroll_offset );

		virtual V2			GetSize() const;

		const CString &		GetSkinName() const;

		static int			Compare( const void * arg1, const void * arg2 );

	protected:

		CString				m_szSkinName;
};

//**********************************************************************************
//	
//**********************************************************************************
CSkinListItem::CSkinListItem( const CString & skin_name )
:	m_szSkinName( skin_name )
{
}

//**********************************************************************************
//	
//**********************************************************************************
V2	CSkinListItem::Render( V2 ret_pos, bool highlight, float scroll_offset )
{
	V2			pos( ret_pos );
	const ARGB	color( GetParent()->GetColor() );
	const float	scale( GetParent()->GetScale() );
	const V2	text_size( CFont::GetDefaultFont()->GetStringSize( m_szSkinName ) * scale );

	CGfx::SetClipRegion( pos, text_size );

	CFont::GetDefaultFont()->Print( m_szSkinName, pos + V2( scroll_offset, 0.f ), color, scale );

	if ( scroll_offset != 0.f )
	{
		CFont::GetDefaultFont()->Print( m_szSkinName, pos + V2( GetSize().x + scroll_offset, 0.f ), color, scale );
	}

	CGfx::DisableClipRegions();

	ret_pos.y += text_size.y + ( 1.f * scale );

	return ret_pos;
}

//**********************************************************************************
//	
//**********************************************************************************
V2	CSkinListItem::GetSize() const
{
	const V2	size( CFont::GetDefaultFont()->GetStringSize( m_szSkinName ) );

	return ( size * GetParent()->GetScale() );
}

//**********************************************************************************
//	
//**********************************************************************************
const CString &	CSkinListItem::GetSkinName() const
{
	return m_szSkinName;
}

//**********************************************************************************
//	
//**********************************************************************************
int	CSkinListItem::Compare( const void * arg1, const void * arg2 )
{
	const CSkinListItem * const	p_l( ( *( CSkinListItem ** )( arg1 ) ) );
	const CSkinListItem * const	p_r( ( *( CSkinListItem ** )( arg2 ) ) );

	return StringCompare( p_l->GetSkinName(), p_r->GetSkinName() );
}

//**********************************************************************************
//**********************************************************************************
//
//	Directory list item implementation
//
//**********************************************************************************
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
CSkinSelectMenu::CSkinSelectMenu()
:	m_pSkinList( new CWindowTable() )
,	m_bQuit( false )
{
	m_pSkinList->SetPos( V2( WINDOW_BORDER, WINDOW_BORDER ) );
	m_pSkinList->SetSize( V2( 0.f, 0.f ) );

	AddItem( m_pSkinList );

	AddAllSkins();

	SetTitle( "Skin Selection" );
	SetPos( V2( 0.5f * ( CGfx::s_ScreenWidth - SELECTION_SIZE.x ), 0.5f * ( CGfx::s_ScreenHeight - SELECTION_SIZE.y ) ) );
	SetSize( SELECTION_SIZE );
	SetFocus( true );
}

//**********************************************************************************
//	
//**********************************************************************************
CSkinSelectMenu::~CSkinSelectMenu()
{
	SAFE_DELETE( m_pSkinList );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinSelectMenu::SetSize( const V2 & size )
{
	CWindow::SetSize( size );

	m_pSkinList->SetSize( V2( size.x - ( 2.f * WINDOW_BORDER ), size.y - ( 2.f * WINDOW_BORDER ) ) );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinSelectMenu::SetFocus( bool focus )
{
	m_pSkinList->SetFocus( focus );

	if ( focus == true )
	{
		CHUD::SetButtons( "Select", "Back", "", "" );
	}

	CWindow::SetFocus( focus );
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CSkinSelectMenu::Show()
{
	while ( m_bQuit == false )
	{
		CFrameWork::Process();
	}

	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinSelectMenu::Quit()
{
	m_bQuit = true;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinSelectMenu::ProcessInput()
{
	if ( HasFocus() == true )
	{
		if ( CInput::IsButtonClicked( CInput::CIRCLE ) == true )
		{
			Quit();
		}
	}

	CWindow::ProcessInput();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinSelectMenu::AddAllSkins()
{
	FIND_FILE_HANDLE	handle;
	sDirEntry			dir_entry;

	if ( CFileSystem::FindFirstFile( CSkinManager::s_szSkinFolder, handle ) == true )
	{
		while ( CFileSystem::FindNextFile( dir_entry, handle ) == true )
		{
			if ( dir_entry.IsDirectory() == true )
			{
				if ( dir_entry.m_szFileName != "." && dir_entry.m_szFileName != ".." )
				{
					CSkinListItem * const	p_item( new CSkinListItem( dir_entry.m_szFileName ) );

					p_item->SetCallback( SelectCallback );

					m_pSkinList->AddItem( p_item );
				}
			}
		}

		m_pSkinList->Sort( CSkinListItem::Compare );

		CFileSystem::FindCloseFile( handle );
	}
}

//**********************************************************************************
//	
//**********************************************************************************
void	CSkinSelectMenu::SelectCallback( CWindowTableItem * p_item, u32 item_no )
{
	CSkinListItem * const	p_list_item( static_cast< CSkinListItem * >( p_item ) );
	CSkinSelectMenu * const	p_window( static_cast< CSkinSelectMenu * >( p_list_item->GetParent()->GetParent() ) );

	CSkinManager::SetCurrentSkin( p_list_item->GetSkinName() );

	p_window->Quit();
}

//*******************************  END OF FILE  ************************************
