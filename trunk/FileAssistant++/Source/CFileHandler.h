/***********************************************************************************

  Module :	CFileHandler.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 05 August 2005 71M

***********************************************************************************/

#ifndef CFILEHANDLER_H_
#define CFILEHANDLER_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CSkinManager.h"
#include "CFileSystem.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
struct sFileExtensionInfo
{
	CString							m_szExecutionName;
	CSkinManager::eSkinComponent	m_Icon;
};

typedef	bool						( * ExecutionCallback )( const CString & filename );
typedef	const sFileExtensionInfo &	( * InformationCallback )( const CString & filename );
typedef	void						( * BackgroundImageCallback )( const CString & filename );

struct sFileHandlerInfo
{
	CString						m_szExtension;
	ExecutionCallback			m_ExecuteCallback;
	InformationCallback			m_InformationCallback;
	BackgroundImageCallback		m_BackgroundCallback;
};

typedef std::list< sFileHandlerInfo * >	CFileHandleList;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CFileHandler
{
	public:

		static bool					Open();
		static void					Close();

		static void					RegisterExtension( const CString & extension,
													   ExecutionCallback execute_callback,
													   InformationCallback info_callback,
													   BackgroundImageCallback background_callback = NULL );

		static sFileHandlerInfo *	FindHandler( const CString & file_name );

		static void					SetFileList( const CFileList & file_list );
		static bool					MultiSelection();
		static const CString &		GetFile();
		static const CString &		GetPrevFile();
		static const CString &		GetNextFile();

	private:

		static CFileList			s_FileList;
		static CFileList::iterator	s_CurrentFile;
		static CFileHandleList		s_HandleList;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CFILEHANDLER_H_ */
