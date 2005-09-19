/***********************************************************************************

  Module :	CTextFileHandler.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 07 September 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTextFileHandler.h"
#include "CRenderable.h"
#include "CHUD.h"
#include "CInput.h"
#include "CFileSystem.h"
#include "CProcess.h"
#include "CFrameWork.h"
#include "CMessageBox.h"

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
static const sFileExtensionInfo	s_FileExtensionInfo =
{
	"Read",
	CSkinManager::SC_ICON_TEXT
};

static const char *	s_pText( NULL );

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
bool	CTextFileHandler::Execute( const CString & file )
{
	CFile * const	p_file( CFileSystem::Open( file, "rt" ) );

	if ( p_file != NULL )
	{
		s_pText = new char[ p_file->GetLength() ];

		if ( s_pText != NULL )
		{
			CProcess::Pause( true );

			CHUD::SetButton( CHUD::BUTTON_START, "Hide UI" );
			CHUD::SetButtons( "", "Quit", "", "" );

			CRenderable::Register( CRenderable::RO_WINDOWS, CTextFileHandler::Render );

			while ( CInput::IsButtonClicked( CInput::CIRCLE ) == false )
			{
				CFrameWork::Process();

				if ( CInput::IsButtonClicked( CInput::START ) == true )
				{
					CHUD::Show( ( CHUD::IsVisible() == false ) );
				}
			}

			CRenderable::UnRegister( CRenderable::RO_WINDOWS, CTextFileHandler::Render );

			CProcess::Pause( false );

			CHUD::Show( true );
		}
		else
		{
			CErrorMessage	box( "Not enough memory to display text file" );

			box.Show();
		}

		SAFE_DELETE( s_pText );

		CFileSystem::Close( p_file );

		return true;
	}

	CErrorMessage	box( "Invalid text file" );

	box.Show();

	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
const sFileExtensionInfo &	CTextFileHandler::Information( const CString & file )
{
	return s_FileExtensionInfo;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CTextFileHandler::Render()
{

}

//*******************************  END OF FILE  ************************************
