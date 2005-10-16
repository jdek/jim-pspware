/***********************************************************************************

  Module :	CConfigFile.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 20 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CConfigFile.h"
#include "TinyXML.h"
#include "CFrameWork.h"
#include "CFileSystem.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
static const CString	s_szConfigFile( "Data/Config.xml" );

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************
CConfigFile::CVarList *	CConfigFile::s_pVarList( NULL );
CConfigFile *			CConfigFile::s_pInstance( NULL );

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
void	CConfigFile::Open()
{
	ASSERT( s_pInstance == NULL, "CConfigFile is already open!" );

	s_pInstance = new CConfigFile();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CConfigFile::Close()
{
	ASSERT( s_pInstance != NULL, "CConfigFile isn't open!" );

	SAFE_DELETE( s_pInstance );
}

//**********************************************************************************
//	
//**********************************************************************************
CConfigFile::CConfigFile()
:	m_pDocument( new TiXmlDocument() )
{
	ASSERT( m_pDocument != NULL, "Failed to create the XML document" );

	if ( m_pDocument != NULL )
	{
		if ( m_pDocument->LoadFile( s_szConfigFile ) == true )
		{
			TiXmlHandle		doc_handle( m_pDocument );

			//
			//	Initialise the config vars
			//
			TiXmlElement *	p_options( doc_handle.FirstChild( "FileAssistant" ).FirstChild( "options" ).Element() );

			if ( p_options != NULL )
			{
				for ( CVarList::iterator it = s_pVarList->begin(); it != s_pVarList->end(); ++it )
				{
					CConfigVarBase *	p_var( *it );
					const char * const	p_attribute( p_options->Attribute( p_var->GetName() ) );

					if ( p_attribute != NULL )
					{
						p_var->Set( p_attribute );
					}
				}
			}

			//
			//	Parse the image plugin configuration
			//
			TiXmlElement *	p_plugin( doc_handle.FirstChild( "FileAssistant" ).FirstChild( "plugins" ).FirstChild( "plugin" ).Element() );

			for ( p_plugin ; p_plugin; p_plugin = p_plugin->NextSiblingElement() )
			{
				if ( p_plugin->Attribute( "var" ) != NULL )
				{
					TRACE( "plugin name = %s\n", p_plugin->Attribute( "var" ) );

					TiXmlElement *	p_settings( p_plugin->FirstChild()->ToElement() );

					for ( p_settings ; p_settings; p_settings = p_settings->NextSiblingElement() )
					{
						if ( p_settings->Attribute( "var" ) != NULL )
						{
							TRACE( "\tsetting = %s\n", p_settings->Attribute( "var" ) );
						}
					}
				}
			}
		}
		else
		{
			TRACE( "%s\n", m_pDocument->ErrorDesc() );

			ASSERT( 0, "Failed to load config file!" );
		}
	}
}

//**********************************************************************************
//	
//**********************************************************************************
CConfigFile::~CConfigFile()
{
	//
	//	Now save all the changes to the config file
	//
	if ( m_pDocument != NULL )
	{
		TiXmlHandle		doc_handle( m_pDocument );
		TiXmlElement *	p_options( doc_handle.FirstChild( "FileAssistant" ).FirstChild( "options" ).Element() );

		if ( p_options != NULL )
		{
			for ( CVarList::iterator it = s_pVarList->begin(); it != s_pVarList->end(); ++it )
			{
				CConfigVarBase *	p_var( *it );

				p_var->Save( p_options );
			}
		}

		if ( CFileSystem::MakeWritable( s_szConfigFile ) == true )
		{
			if ( m_pDocument->SaveFile( s_szConfigFile ) == false )
			{
				ASSERT( 0, "Failed to save config file!" );
			}
		}
	}
}

//**********************************************************************************
//	
//**********************************************************************************
void	CConfigFile::AddVar( CConfigVarBase * const p_var )
{
	if ( s_pVarList == NULL )
	{
		s_pVarList = new CVarList();
	}

	s_pVarList->push_back( p_var );
}

//*******************************  END OF FILE  ************************************
