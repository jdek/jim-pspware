/***********************************************************************************

  Module :	CFrameWork.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

#ifndef CFRAMEWORK_H_
#define CFRAMEWORK_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CString.h"

//**********************************************************************************
//   Macros
//**********************************************************************************
#ifndef	_DEBUG

#define	TRACE

#else	// #ifdef	_DEBUG

#define TRACE					CFrameWork::DBGMessage

#endif	// #ifdef	_DEBUG

//**********************************************************************************
//   Types
//**********************************************************************************
class CFile;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CFrameWork
{
	public:

		/* If the type is 0, then load the module in the kernel partition, otherwise load it
		in the user partition. */
		enum eModulePartition
		{
			KERNEL_PARTITION	= 0,
			USER_PARTITION		= 1,
		};

		enum eFirmwareVersion
		{
			PSP_v1_0,
			PSP_v1_5,
		};

	public:

		// Initialises the app
		static bool				Open();

		// Shuts down the app
		static void				Close();

		// Update the app
		static bool				Process();

		// Returns the time in milliseconds since the app was started
		static u32				GetTicks();

		// Returns the time in milliseconds of the last frame
		static float			GetElapsedTime();

		// Displays a debug message on the screen - note, this will be cleared by calls to GCfx::ClearScreen
		static void				DBGMessage( const char * fmt, ... );

		// Load and run the specifed executable file
		static SceUID			LoadModule( const CString & module, eModulePartition partition );
		static bool				LoadAndStartModule( const CString & module, eModulePartition partition );

		// Runs an executable (PBP/ELF) file
		static bool				RunExecutable( const CString & executable );

		// Returns the firmware version of the PSP
		static eFirmwareVersion	GetVersion();

	private:

		static u64				s_Time;
		static u64				s_LastTime;
		static u64				s_TickResolution;
		static CFile *			s_pLogFile;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CFRAMEWORK_H_ */
