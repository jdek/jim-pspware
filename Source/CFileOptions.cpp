/***********************************************************************************

  Module :	CFileOptions.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 04 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CFileOptions.h"
#include "CWindowTable.h"
#include "CFont.h"
#include "CFileAssistant.h"
#include "CMessageBox.h"
#include "CInput.h"
#include "CFileSystem.h"
#include "CFrameWork.h"
#include "CFileHandler.h"
#include "CHUD.h"
#include "CTextInput.h"
#include "CGfx.h"
#include "CBackground.h"
#include "CMusicFileHandler.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
static const float	WINDOW_BORDER( 16.f );
static const float	MIN_TABLE_WIDTH( 150.f );

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************
const CString	CFileOptions::s_szOptionNames[ MAX_OPTIONS ] =
{
	"No options",	// OPTION_NULL,
	"Execute",		// OPTION_EXECUTE,
	"Copy",			// OPTION_COPY,
	"Delete",		// OPTION_DELETE,
	"Move",			// OPTION_MOVE,
	"Rename",		// OPTION_RENAME,
};

//**********************************************************************************
//**********************************************************************************
//
//	File option item implementation
//
//**********************************************************************************
//**********************************************************************************
class CFileOptionsMenuItem : public CWindowTableItem
{
	public:

		CFileOptionsMenuItem( CFileOptions::eOption option, const CString & text );

		virtual V2				Render( V2 pos, bool highlight, float scroll_offset );

		virtual V2				GetSize() const;

		CFileOptions::eOption	GetOptionNumber() const;

	protected:

		CString					m_szText;
		CFileOptions::eOption	m_OptionNumber;
};

//**********************************************************************************
//	
//**********************************************************************************
CFileOptionsMenuItem::CFileOptionsMenuItem( CFileOptions::eOption option, const CString & text )
:	m_szText( text )
,	m_OptionNumber( option )
{
}

//**********************************************************************************
//	
//**********************************************************************************
V2	CFileOptionsMenuItem::Render( V2 pos, bool highlight, float scroll_offset )
{
	CFont::GetDefaultFont()->Print( m_szText, pos, m_pParent->GetColor() );

	pos.y += GetSize().y + 1;

	return pos;
}

//**********************************************************************************
//	
//**********************************************************************************
V2	CFileOptionsMenuItem::GetSize() const
{
	return CFont::GetDefaultFont()->GetStringSize( m_szText );
}

//**********************************************************************************
//	
//**********************************************************************************
CFileOptions::eOption	CFileOptionsMenuItem::GetOptionNumber() const
{
	return m_OptionNumber;
}


//**********************************************************************************
//**********************************************************************************
//
//	File option menu implementation
//
//**********************************************************************************
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
CFileOptions::CFileOptions( const CFileList & file_list )
:	m_pOptions( new CWindowTable() )
,	m_pHandleInfo( NULL )
,	m_FileList( file_list )
,	m_szFirstFile( ( *file_list.begin() ).m_szFileName )
{
	m_pHandleInfo = CFileHandler::FindHandler( m_szFirstFile );

	//
	//	Add all the available options
	//
	AddOptions();

	//
	//	Set the size depending on the number of items
	//
	V2	table_size( m_pOptions->GetAllItemsSize() );
	V2	window_size( MIN_TABLE_WIDTH + ( 2.f * WINDOW_BORDER ), table_size.y + ( 3.f * WINDOW_BORDER ) );

	SetTitle( "File Options" );
	SetPos( V2( 0.5f * ( CGfx::s_ScreenWidth - window_size.x ), 0.5f * ( CGfx::s_ScreenHeight - window_size.y ) ) );
	SetSize( window_size );

	table_size.x = MIN_TABLE_WIDTH;

	m_pOptions->SetFocus( true );
	m_pOptions->SetPos( V2( WINDOW_BORDER, WINDOW_BORDER ) );
	m_pOptions->SetSize( table_size );

	AddItem( m_pOptions );

	CHUD::SetButtons( "Select", "Exit", "", "" );

	if ( m_pHandleInfo != NULL )
	{
		if ( m_pHandleInfo->m_BackgroundCallback != NULL )
		{
			m_pHandleInfo->m_BackgroundCallback( m_szFirstFile );
		}
	}

	CFileAssistant::Get()->SetListFocus( false );
}

//**********************************************************************************
//	
//**********************************************************************************
CFileOptions::~CFileOptions()
{
	SAFE_DELETE( m_pOptions );

	CBackground::SetBackgroundTexture( NULL );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileOptions::ProcessInput()
{
	if ( CInput::IsButtonClicked( CInput::CIRCLE ) == true )
	{
		Delete();

		CFileAssistant::Get()->SetListFocus( true );
	}

	CWindow::ProcessInput();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileOptions::AddOptions()
{
	CFileOptionsMenuItem *			p_item( NULL );
	const CDirectoryList * const	p_dst_list( CFileAssistant::Get()->GetDstList() );
	const CDirectoryList * const	p_focus_list( CFileAssistant::Get()->GetFocusList() );

	//
	//	Execute
	//
	if ( m_pHandleInfo != NULL )
	{
		p_item = new CFileOptionsMenuItem( OPTION_EXECUTE, m_pHandleInfo->m_InformationCallback( m_szFirstFile ).m_szExecutionName );
		p_item->SetCallback( OptionSelectedCallback );
		m_pOptions->AddItem( p_item );
	}

	//
	//	Copy
	//
	if ( p_dst_list->ReadOnly() == false )
	{
		if ( p_focus_list != p_dst_list )
		{
			p_item = new CFileOptionsMenuItem( OPTION_COPY, s_szOptionNames[ OPTION_COPY ] );
			p_item->SetCallback( OptionSelectedCallback );
			m_pOptions->AddItem( p_item );
		}
	}

	//
	//	Delete
	//
	if ( p_focus_list->ReadOnly() == false )
	{
		p_item = new CFileOptionsMenuItem( OPTION_DELETE, s_szOptionNames[ OPTION_DELETE ] );
		p_item->SetCallback( OptionSelectedCallback );
		m_pOptions->AddItem( p_item );
	}

	//
	//	Move
	//
	if ( p_focus_list->ReadOnly() == false )
	{
		if ( p_dst_list->ReadOnly() == false )
		{
			if ( p_focus_list != p_dst_list )
			{
				p_item = new CFileOptionsMenuItem( OPTION_MOVE, s_szOptionNames[ OPTION_MOVE ] );
				p_item->SetCallback( OptionSelectedCallback );
				m_pOptions->AddItem( p_item );
			}
		}
	}

	//
	//	Rename
	//
	if ( p_focus_list->ReadOnly() == false )
	{
		p_item = new CFileOptionsMenuItem( OPTION_RENAME, s_szOptionNames[ OPTION_RENAME ] );
		p_item->SetCallback( OptionSelectedCallback );
		m_pOptions->AddItem( p_item );
	}

	//
	//	Check to see if we have any options at all
	//
	if ( m_pOptions->GetItemList().empty() == true )
	{
		//
		//	We don't have any options to display
		//
		p_item = new CFileOptionsMenuItem( OPTION_NULL, s_szOptionNames[ OPTION_NULL ] );
		p_item->SetCallback( OptionSelectedCallback );
		m_pOptions->AddItem( p_item );
	}
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileOptions::OptionSelectedCallback( CWindowTableItem * p_item, u32 item_no )
{
	CFileOptionsMenuItem * const	p_option_item( static_cast< CFileOptionsMenuItem * >( p_item ) );
	CWindowTable * const			p_table( p_option_item->GetParent() );
	CFileOptions * const			p_window( static_cast< CFileOptions * >( p_table->GetParent() ) );

	//
	//	Move the window off screen
	//
	p_window->SetFocus( false );
	p_window->SetPos( V2( 1000.f, 1000.f ) );

	switch ( p_option_item->GetOptionNumber() )
	{
	case OPTION_EXECUTE:
		{
			p_window->ExecuteFile();
		}
		break;

	case OPTION_COPY:
		{
			CopyCallback();
		}
		break;

	case OPTION_DELETE:
		{
			DeleteCallback();
		}
		break;

	case OPTION_MOVE:
		{
			MoveCallback();
		}
		break;

	case OPTION_RENAME:
		{
			RenameCallback();
		}
		break;

	case OPTION_NULL:
		{
		}
		break;

	default:
		{
			ASSERT( 0, "Unhandled option number!" );
		}
	}

	p_window->Delete();

	CFileAssistant::Get()->SetListFocus( true );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileOptions::ExecuteFile()
{
	if ( m_pHandleInfo != NULL )
	{
		CFileHandler::SetFileList( m_FileList );

		if ( m_pHandleInfo->m_ExecuteCallback( CFileHandler::GetFile() ) == false )
		{
		}
	}
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileOptions::CopyCallback()
{
	CString					text;
	CFileList				copy_items;
	CDirectoryList * const	p_src_list( CFileAssistant::Get()->GetSrcList() );
	CDirectoryList * const	p_dst_list( CFileAssistant::Get()->GetDstList() );
	const CString &			src_path( p_src_list->GetCurrentPath() );
	const CString &			dst_path( p_dst_list->GetCurrentPath() );

	p_src_list->GetSelectedFiles( copy_items, false );

	if ( copy_items.size() > 1 )
	{
		text = "Do you want to copy";
		text += "\n";
		text += "the selected files";
	}
	else
	{
		text = "Do you want to copy";
		text += "\n";
		text += ( *copy_items.begin() ).m_szFileName;
	}

	text += "\n";
	text += "to";
	text += " ";
	text += dst_path + CString( "?" );

	CModalMessageBox	box( "Copy Confirm", text );

	box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Yes" );
	box.AddExitCode( CModalMessageBox::EXIT_CIRCLE, "No" );

	if ( box.Show() == CModalMessageBox::EXIT_CROSS )
	{
		bool	success( true );

		//
		//	Work out the values for the progess bar
		//
		float		progress( 0.f );
		const float	progress_inc( 1.f / copy_items.size() );

		CHUD::ShowProgressBar( true, "Copying..." );

		for ( CFileList::iterator it = copy_items.begin(); success == true && it != copy_items.end(); ++it )
		{
			const sDirEntry	item( *it );
			const CString	src_file( src_path + item.m_szFileName );
			const CString	dst_file( dst_path + item.m_szFileName );

			CHUD::SetProgressBar( progress );

			if ( item.IsFile() == true )
			{
				if ( CFileSystem::FileExists( dst_file ) == false )
				{
					success = CFileSystem::CopyFile( src_file, dst_file, progress_inc );
				}
				else
				{
					CString	text;

					text.Printf( "%s\n%s\n%s %s?", CString( "Do you want to overwrite" ).GetPtr(), dst_file.GetPtr(), CString( "with" ).GetPtr(), src_file.GetPtr() );

					CModalMessageBox	overwrite_box( "Overwrite Confirm", text );

					overwrite_box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Yes" );
					overwrite_box.AddExitCode( CModalMessageBox::EXIT_CIRCLE, "No" );

					if ( overwrite_box.Show() == CModalMessageBox::EXIT_CROSS )
					{
						success = CFileSystem::CopyFile( src_file, dst_file, progress_inc );
					}
				}
			}
			else
			{
				if ( CFileSystem::DirectoryExists( dst_file ) == false )
				{
					success = CFileSystem::CopyDirectory( src_file, dst_file, progress_inc );
				}
				else
				{
					CString	text;

					text.Printf( "%s\n%s\n%s %s?", "Do you want to overwrite", dst_file.GetPtr(), "with", src_file.GetPtr() );

					CModalMessageBox	overwrite_box( "Overwrite Confirm", text );

					overwrite_box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Yes" );
					overwrite_box.AddExitCode( CModalMessageBox::EXIT_CIRCLE, "No" );

					if ( overwrite_box.Show() == CModalMessageBox::EXIT_CROSS )
					{
						success = CFileSystem::CopyDirectory( src_file, dst_file, progress_inc );
					}
				}
			}

			progress += progress_inc;
		}

		CHUD::ShowProgressBar( false, "" );

		//
		//	Display the error/success message
		//
		if ( success == false )
		{
			CErrorMessage	error_msg( "Error copying file(s)!" );

			error_msg.Show();
		}
		else
		{
			CModalMessageBox	success_box( "Copy Success", "File(s) copied successfully!" );

			success_box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Continue" );

			success_box.Show();
		}
	}

	//
	//	Refresh the directory lists
	//
	CFileAssistant::Get()->GetSrcList()->RefreshList( true );
	CFileAssistant::Get()->GetDstList()->RefreshList( true );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileOptions::DeleteCallback()
{
	CString					text;
	CFileList				delete_items;
	CDirectoryList * const	p_list( CFileAssistant::Get()->GetFocusList() );

	p_list->GetSelectedFiles( delete_items, false );

	if ( delete_items.size() > 1 )
	{
		text = "Do you want to delete\nthe selected files?";
	}
	else
	{
		text = CString( "Do you want to delete\n" ) + ( *delete_items.begin() ).m_szFileName + CString( "?" );
	}

	CModalMessageBox	box( "Delete Confirm", text );

	box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Yes" );
	box.AddExitCode( CModalMessageBox::EXIT_CIRCLE, "No" );

	if ( box.Show() == CModalMessageBox::EXIT_CROSS )
	{
		bool			success( true );
		const CString &	src_path( p_list->GetCurrentPath() );

		for ( CFileList::iterator it = delete_items.begin(); success == true && it != delete_items.end(); ++it )
		{
			sDirEntry		item( *it );
			const CString	src_file( src_path + item.m_szFileName );

			if ( item.IsFile() == true )
			{
				//
				//	If this is the currently playing song, then make sure it's stopped before it's deleted
				//
				if ( src_file == CMusicFileHandler::GetFilePlaying() )
				{
					CMusicFileHandler::Stop();
				}

				success = CFileSystem::DeleteFile( src_file );
			}
			else
			{
				success = CFileSystem::DeleteDirectory( src_file );
			}
		}

		//
		//	Display the error/success message
		//
		if ( success == false )
		{
			CErrorMessage	error_msg( "Error deleting file(s)!" );

			error_msg.Show();
		}
		else
		{
			CModalMessageBox	success_box( "Delete Success", "File(s) deleted successfully!" );

			success_box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Continue" );

			success_box.Show();
		}
	}

	//
	//	Refresh the directory lists
	//
	CFileAssistant::Get()->GetSrcList()->RefreshList( true );
	CFileAssistant::Get()->GetDstList()->RefreshList( true );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileOptions::MoveCallback()
{
	CString					text;
	CFileList				move_items;
	CDirectoryList * const	p_src_list( CFileAssistant::Get()->GetSrcList() );
	CDirectoryList * const	p_dst_list( CFileAssistant::Get()->GetDstList() );
	const CString &			src_path( p_src_list->GetCurrentPath() );
	const CString &			dst_path( p_dst_list->GetCurrentPath() );

	p_src_list->GetSelectedFiles( move_items, false );

	if ( move_items.size() > 1 )
	{
		text = "Do you want to move\nthe selected files";
	}
	else
	{
		text = CString( "Do you want to move\n" ) + ( *move_items.begin() ).m_szFileName;
	}

	text += CString( "\nto " ) + dst_path + CString( "?" );

	CModalMessageBox	box( "Move Confirm", text );

	box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Yes" );
	box.AddExitCode( CModalMessageBox::EXIT_CIRCLE, "No" );

	if ( box.Show() == CModalMessageBox::EXIT_CROSS )
	{
		//
		//	Work out the values for the progess bar
		//
		float		progress( 0.f );
		const float	progress_inc( 1.f / move_items.size() );

		CHUD::ShowProgressBar( true, "Moving..." );

		//
		//	Copy all the files to the destination
		//
		bool	success( true );

		for ( CFileList::iterator it = move_items.begin(); success == true && it != move_items.end(); ++it )
		{
			const sDirEntry	item( *it );
			const CString	src_file( src_path + item.m_szFileName );
			const CString	dst_file( dst_path + item.m_szFileName );

			CHUD::SetProgressBar( progress );

			if ( item.IsFile() == true )
			{
				if ( CFileSystem::FileExists( dst_file ) == false )
				{
					success = CFileSystem::CopyFile( src_file, dst_file );
				}
				else
				{
					const CString		text( CString( "Do you want to overwrite\n" ) + dst_file + CString( "\nwith\n" ) + src_file + CString( "?" ) );
					CModalMessageBox	overwrite_box( "Overwrite Confirm", text );

					overwrite_box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Yes" );
					overwrite_box.AddExitCode( CModalMessageBox::EXIT_CIRCLE, "No" );

					if ( overwrite_box.Show() == CModalMessageBox::EXIT_CROSS )
					{
						success = CFileSystem::CopyFile( src_file, dst_file );
					}
				}
			}
			else
			{
				if ( CFileSystem::DirectoryExists( dst_file ) == false )
				{
					success = CFileSystem::CopyDirectory( src_file, dst_file );
				}
				else
				{
					const CString		text( CString( "Do you want to overwrite\n" ) + dst_file + CString( "\nwith\n" ) + src_file + CString( "?" ) );
					CModalMessageBox	overwrite_box( "Overwrite Confirm", text );

					overwrite_box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Yes" );
					overwrite_box.AddExitCode( CModalMessageBox::EXIT_CIRCLE, "No" );

					if ( overwrite_box.Show() == CModalMessageBox::EXIT_CROSS )
					{
						success = CFileSystem::CopyDirectory( src_file, dst_file );
					}
				}
			}

			progress += progress_inc;
		}

		//
		//	Now delete all the files that've been copied
		//
		for ( CFileList::iterator it = move_items.begin(); success == true && it != move_items.end(); ++it )
		{
			sDirEntry		item( *it );
			const CString	src_file( src_path + item.m_szFileName );

			if ( item.IsFile() == true )
			{
				success = CFileSystem::DeleteFile( src_file );
			}
			else
			{
				//
				//	If the destination is within the source dircetory then don't delete it!
				//
				if ( ( item.IsFile() == true ) || ( item.IsDirectory() == true && dst_path.Find( src_path ) == NULL ) )
				{
					success = CFileSystem::DeleteDirectory( src_file );
				}
			}
		}

		CHUD::ShowProgressBar( false, "" );

		//
		//	Display the error/success message
		//
		if ( success == false )
		{
			CErrorMessage	error_msg( "Error moving file(s)!" );

			error_msg.Show();
		}
		else
		{
			CModalMessageBox	success_box( "Move Success", "File(s) moved successfully!" );

			success_box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Continue" );

			success_box.Show();
		}
	}

	//
	//	Refresh the directory lists
	//
	CFileAssistant::Get()->GetSrcList()->RefreshList( true );
	CFileAssistant::Get()->GetDstList()->RefreshList( true );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileOptions::RenameCallback()
{
	bool					confirmed( true );
	CFileList				rename_items;
	CDirectoryList * const	p_list( CFileAssistant::Get()->GetFocusList() );
	const CString &			src_path( p_list->GetCurrentPath() );

	//
	//	Get the selected files
	//
	p_list->GetSelectedFiles( rename_items, false );

	//
	//	Show the confirmation dialog box
	//
	if ( rename_items.size() > 1 )
	{
		CModalMessageBox	box( "Rename Confirm", "Do you want to rename\nthe selected files" );

		box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Yes" );
		box.AddExitCode( CModalMessageBox::EXIT_CIRCLE, "No" );

		if ( box.Show() == CModalMessageBox::EXIT_CIRCLE )
		{
			confirmed = false;
		}
	}

	if ( confirmed == true )
	{
		bool					success( true );
		CTextInput::eReturnCode	return_code( CTextInput::RC_OK );

		for ( CFileList::iterator it = rename_items.begin(); success == true && it != rename_items.end(); ++it )
		{
			sDirEntry	item( *it );
			CTextInput	text_input( "Rename File", item.m_szFileName );

			return_code = text_input.Show();

			if ( return_code == CTextInput::RC_OK )
			{
				success = CFileSystem::Rename( src_path + item.m_szFileName, src_path + text_input.GetText() );
			}
		}

		//
		//	Display the error/success message
		//
		if ( return_code == CTextInput::RC_OK )
		{
			if ( success == false )
			{
				CErrorMessage	error_msg( "Error renaming file(s)!" );

				error_msg.Show();
			}
			else
			{
				CModalMessageBox	success_box( "Rename Success", "File(s) renamed successfully!" );

				success_box.AddExitCode( CModalMessageBox::EXIT_CROSS, "Continue" );

				success_box.Show();
			}
		}
	}

	//
	//	Refresh the directory lists
	//
	CFileAssistant::Get()->GetSrcList()->RefreshList( true );
	CFileAssistant::Get()->GetDstList()->RefreshList( true );
}

//*******************************  END OF FILE  ************************************
