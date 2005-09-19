/***********************************************************************************

  Module :	CFileOptions.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 04 August 2005 71M

***********************************************************************************/

#ifndef CFILEOPTIONS_H_
#define CFILEOPTIONS_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CWindow.h"
#include "CFileHandler.h"
#include "CFileSystem.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
class CWindowTable;
class CWindowTableItem;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CFileOptions : public CWindow
{
	public:

		enum eOption
		{
			OPTION_NULL,
			OPTION_EXECUTE,
			OPTION_COPY,
			OPTION_DELETE,
			OPTION_MOVE,
			OPTION_RENAME,

			MAX_OPTIONS
		};

	public:

		CFileOptions( const CFileList & file_list );
		~CFileOptions();

		virtual void				ProcessInput();

	protected:

		void						AddOptions();

		void						ExecuteFile();

		static void					OptionSelectedCallback( CWindowTableItem * p_item, u32 item_no );

		static void					CopyCallback();
		static void					MoveCallback();
		static void					DeleteCallback();
		static void					RenameCallback();

	protected:

		CWindowTable *				m_pOptions;
		CFileList					m_FileList;
		CString						m_szFirstFile;
		const sFileHandlerInfo *	m_pHandleInfo;

		static const CString		s_szOptionNames[ MAX_OPTIONS ];
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CFILEOPTIONS_H_ */
