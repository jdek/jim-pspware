/***********************************************************************************

  Module :	CInformationDialog.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 21 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CInformationDialog.h"
#include "CFileSystem.h"
#include "CHUD.h"
#include "CFileAssistant.h"

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

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
CInformationDialog::CInformationDialog( const sDirEntry * const p_file_info )
:	CModalMessageBox( "File Information", "" )
{
	CString		text;

	text.Printf( "FileName: %s\n", p_file_info->m_szFileName.GetPtr() );

	//
	//	Display the size of the file
	//
	s32	size( 0 );

	if ( p_file_info->IsDirectory() == true )
	{
		CFileList	dir_files;
		CString		full_path( CFileAssistant::Get()->GetFocusList()->GetCurrentPath() );

		full_path += p_file_info->m_szFileName;

		if ( CFileSystem::GetDirectoryFiles( full_path, dir_files ) == true )
		{
			for ( CFileList::iterator it = dir_files.begin(); it != dir_files.end(); ++it )
			{
				if ( ( *it ).IsFile() == true )
				{
					CFile * const	p_file( CFileSystem::Open( ( *it ).m_szFileName, "rb" ) );

					if ( p_file != NULL )
					{
						size += p_file->GetLength();

						CFileSystem::Close( p_file );
					}
				}
			}
		}
	}
	else
	{
		if ( p_file_info->m_Stats.st_size > 0 )
		{
			size = p_file_info->m_Stats.st_size;
		}
	}

	if ( size > 0 && size < 1024 )
	{
		size = 1;
	}
	else
	{
		size /= 1024;
	}

	text.Printf( "%sSize: %d KB\n", text.GetPtr(), size );

	//
	//	Display the text
	//
	SetText( text );

	AddExitCode( EXIT_CROSS, "Continue" );

	CHUD::SetButtons( "Continue", "", "", "" );
}

//*******************************  END OF FILE  ************************************
