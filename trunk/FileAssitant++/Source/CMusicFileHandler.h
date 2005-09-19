/***********************************************************************************

  Module :	CMusicFileHandler.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 07 August 2005 71M

***********************************************************************************/

#ifndef CMUSICFILEHANDLER_H_
#define CMUSICFILEHANDLER_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CFileHandler.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CMusicFileHandler
{
	public:

		enum eError
		{
			SUCCESS,
			NO_CODEC,
			REPLAY_ERROR,
		};

	public:

		static void							Open();
		static void							Close();

		static bool							Execute( const CString & file );
		static const sFileExtensionInfo &	Information( const CString & file );

		static eError						Play( const CString & file );
		static void							Stop();
		static void							Pause();
		static bool							IsPlaying();
		static const CString &				GetFilePlaying();
		static CString						GetPlayingTime();
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CMUSICFILEHANDLER_H_ */
