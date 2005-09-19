/***********************************************************************************

  Module :	CMusicFileHandler.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 07 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CMusicFileHandler.h"
#include "Codec.h"
#include "AHX/AHX.h"
#include "MOD/MODPlayer.h"
#include "MP3/MP3Player.h"
#include "OGG/OGGPlayer.h"
#include "CFrameWork.h"
#include "CFileSystem.h"
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
static sFileExtensionInfo	s_FileExtensionInfo =
{
	"Play",
	CSkinManager::SC_ICON_AUDIO
};

static codecStubs			s_Stubs[ 100 ];
static codecStubs *			s_pDecoder( NULL );
static u32					s_CodecNum( 0 );
static bool					s_bMusicIsPlaying( false );
static bool					s_bMusicIsPaused( false );
static CString				s_szFilePlaying;

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
void	CMusicFileHandler::Open()
{
	//get codecStubs
	int	stubnum( 0 );

	//CODEC_INITSTUBS
	OGGsetStubs( &s_Stubs[ stubnum++ ] );
	MODsetStubs( &s_Stubs[ stubnum++ ] );
	AHXsetStubs( &s_Stubs[ stubnum++ ] );
	MP3setStubs( &s_Stubs[ stubnum++ ] );
	//YMPLAYsetStubs(&s_Stubs[stubnum++]);
	//XMPLAYsetStubs(&s_Stubs[stubnum++]);

	s_CodecNum = stubnum;

	s_szFilePlaying = "";

	pspAudioInit();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CMusicFileHandler::Close()
{
	Stop();

	pspAudioEnd();
}

//**********************************************************************************
//	
//**********************************************************************************
const sFileExtensionInfo &	CMusicFileHandler::Information( const CString & file )
{
	if ( file == s_szFilePlaying )
	{
		s_FileExtensionInfo.m_szExecutionName = "Stop";
	}
	else
	{
		s_FileExtensionInfo.m_szExecutionName = "Play";
	}

	return s_FileExtensionInfo;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CMusicFileHandler::Execute( const CString & filename )
{
	if ( filename.IEquals( s_szFilePlaying ) == true )
	{
		Stop(); 

		return true;
	}
	else
	{
		Stop(); 

		switch ( Play( filename ) )
		{
		case NO_CODEC:
			{
				CErrorMessage	error_msg( "No codec found!" );

				error_msg.Show();
			}
			break;

		case REPLAY_ERROR:
			{
				CErrorMessage	error_msg( "Music file is invalid!" );

				error_msg.Show();
			}
			break;

		default:
			{
				s_szFilePlaying = filename;
			}
			return true;
		}

		return false;
	}
}

//**********************************************************************************
//	
//**********************************************************************************
CMusicFileHandler::eError	CMusicFileHandler::Play( const CString & filename )
{
	const CString	extension( CFileSystem::GetFileExtension( filename ) );

	// Determine codec of the file
	s_pDecoder = NULL;

	for ( u32 codec = 0; codec <= s_CodecNum; ++codec )
	{
		const char *	p_extension( &( s_Stubs[ codec ].extension[ 0 ] ) );

		while ( *p_extension != 0 )
		{
			if ( extension.IEquals( p_extension ) == true )
			{
				s_pDecoder = &s_Stubs[ codec ];

				break;
			}

			p_extension += 4;
		}
	}

	if ( s_pDecoder == NULL )
	{
		return NO_CODEC;
	}

	s_pDecoder->init( 0 );

	if ( s_pDecoder->load( const_cast< char * >( filename.GetPtr() ) ) )
	{
		s_bMusicIsPlaying = true;

		s_pDecoder->play();
	}
	else
	{
		s_bMusicIsPlaying = false;

		s_pDecoder->stop();
		s_pDecoder->end();

		// Cannot replay file...
		return REPLAY_ERROR;
	}

	// ok !
	s_bMusicIsPaused = false;

	return SUCCESS;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CMusicFileHandler::Stop()
{
	if ( s_bMusicIsPlaying == true )
	{
		if ( s_pDecoder != NULL )
		{
			s_pDecoder->stop();
			s_pDecoder->end();
		}

		s_szFilePlaying = "";
		s_bMusicIsPlaying = false;
	}
}

//**********************************************************************************
//	
//**********************************************************************************
void	CMusicFileHandler::Pause()
{
	if ( s_bMusicIsPlaying == true )
	{
		if ( s_pDecoder != NULL )
		{
			s_pDecoder->pause();
		}

		s_bMusicIsPaused = !s_bMusicIsPaused;
	}
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CMusicFileHandler::IsPlaying()
{
	return s_bMusicIsPlaying;
}

//**********************************************************************************
//	
//**********************************************************************************
CString	CMusicFileHandler::GetPlayingTime()
{
	if ( s_pDecoder != NULL )
	{
		char	time[ 200 ];

		s_pDecoder->time( time );

		return time;
	}
	else
	{
		return "00:00:00";
	}
}

//**********************************************************************************
//	
//**********************************************************************************
const CString &		CMusicFileHandler::GetFilePlaying()
{
	return s_szFilePlaying;
}

//*******************************  END OF FILE  ************************************
