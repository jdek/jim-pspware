/***********************************************************************************

  Module :	CFileHandler.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 05 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CFileHandler.h"
#include "CFileSystem.h"
#include "CFrameWork.h"

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
CFileList			CFileHandler::s_FileList;
CFileList::iterator	CFileHandler::s_CurrentFile;
CFileHandleList		CFileHandler::s_HandleList;

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileHandler::Open()
{
	s_FileList.clear();
	s_HandleList.clear();

	return true;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileHandler::Close()
{
	for ( CFileHandleList::iterator it = s_HandleList.begin(); it != s_HandleList.end(); ++it )
	{
		sFileHandlerInfo *	p_info( *it );

		SAFE_DELETE( p_info );
	}

	s_HandleList.clear();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileHandler::RegisterExtension( const CString & extension,
										 ExecutionCallback execute_callback,
										 InformationCallback info_callback,
										 BackgroundImageCallback background_callback )
{
	sFileHandlerInfo * const	p_info( new sFileHandlerInfo() );

	p_info->m_szExtension = extension;
	p_info->m_ExecuteCallback = execute_callback;
	p_info->m_InformationCallback = info_callback;
	p_info->m_BackgroundCallback = background_callback;

	s_HandleList.push_back( p_info );
}

//**********************************************************************************
//	
//**********************************************************************************
sFileHandlerInfo *	CFileHandler::FindHandler( const CString & file_name )
{
	const CString	extension( CFileSystem::GetFileExtension( file_name ) );

	for ( CFileHandleList::iterator it = s_HandleList.begin(); it != s_HandleList.end(); ++it )
	{
		if ( ( *it )->m_szExtension.IEquals( extension ) == true )
		{
			return ( *it );
		}
	}

	return NULL;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileHandler::SetFileList( const CFileList & file_list )
{
	s_FileList = file_list;
	s_CurrentFile = s_FileList.begin();
}

//**********************************************************************************
//	
//**********************************************************************************
const CString &	CFileHandler::GetFile()
{
	return s_CurrentFile->m_szFileName;
}

//**********************************************************************************
//	
//**********************************************************************************
const CString &	CFileHandler::GetPrevFile()
{
	if ( s_CurrentFile == s_FileList.begin() )
	{
		s_CurrentFile = s_FileList.end();
	}

	--s_CurrentFile;

	return s_CurrentFile->m_szFileName;
}

//**********************************************************************************
//	
//**********************************************************************************
const CString &	CFileHandler::GetNextFile()
{
	++s_CurrentFile;

	if ( s_CurrentFile == s_FileList.end() )
	{
		s_CurrentFile = s_FileList.begin();
	}

	return s_CurrentFile->m_szFileName;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileHandler::MultiSelection()
{
	return ( s_FileList.size() > 1 );
}

//*******************************  END OF FILE  ************************************
