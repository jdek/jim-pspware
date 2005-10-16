/***********************************************************************************

  Module :	CFrameWork.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CFrameWork.h"
#include "CGfx.h"
#include "CUSBManager.h"
#include "CInput.h"
#include "CFileSystem.h"
#include "CProcess.h"
#include "CFont.h"
#include "CSkinManager.h"
#include "CExecutableFileHandler.h"
#include "CFileHandler.h"
#include "CBackground.h"
#include "CRenderable.h"
#include "CWindow.h"
#include "CConfigFile.h"
#include "CPRXManager.h"
#include "CLanguage.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
#define RUN_KERNEL_MODE

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************
// Define the module info section, note the 0x1000 flag to enable start in kernel mode
#ifdef RUN_KERNEL_MODE

PSP_MODULE_INFO( "FileAssistant", 0x1000, 0, 1 );  //0->1 ?
PSP_MAIN_THREAD_ATTR( 0 );

#else	// #ifdef RUN_KERNEL_MODE

// Define the thread attribute as 0 so that the main thread does not get converted to user mode
PSP_MODULE_INFO( "FileAssistant", 0, 0, 1 );  //0->1
PSP_MAIN_THREAD_ATTR( THREAD_ATTR_USER );

#endif	// #ifdef RUN_KERNEL_MODE

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************
u64					CFrameWork::s_Time;
u64					CFrameWork::s_LastTime;
u64					CFrameWork::s_TickResolution;
CFile *				CFrameWork::s_pLogFile;

static u32			s_nVersion;
static CString		s_szLogFile( "71M.txt" );
static bool			s_ExitCallbackCalled( false );
static CVarString	CVAR_USB_ON( "usb", "on" );

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
static int	SoftResetThread( SceSize args, void * argp )
{
	while ( 1 )
	{
		TRACE( "THREAD\n" );
	}

/*	bool	reset( false );

	while ( reset == false )
	{
		if ( CInput::IsButtonDown( CInput::LTRIGGER ) == true )
		{
			reset = true;
		}
	}

	CPBPFileHandler::Execute( "ms0:/PSP/GAME/FileAs_________________________1/EBOOT.PBP" );*/

	return 0;
}

//**********************************************************************************
//	
//**********************************************************************************
static int	InstallSoftResetThread()
{
	const int thread_id( sceKernelCreateThread( "soft_reset", SoftResetThread, 0x19, 0x10000, 0, NULL ) );

	if ( thread_id >= 0 )
	{
		sceKernelStartThread( thread_id, 0, NULL );
	}

	return thread_id;
}

//*************************************************************************************
//	
//*************************************************************************************
static void	SetKernelPatch()
{
	//Patch PSP OS
	s_nVersion = *( ( u32 * )0x8805c510 );		// Save the real version number

	if ( CFrameWork::GetVersion() == CFrameWork::PSP_v1_5 )
	{
		// Firmware v1.5
		*((unsigned int *)0x8805c510)=0x00001021; // good PRX
		*((unsigned int *)0x8805c834)=0x00000000; // good VSH
		*((unsigned int *)0x8805c690)=0x00000000; // VSHByID
		*((unsigned int *)0x8805c3b8)=0x00000000; // modByID
		*((unsigned int *)0x8805c834)=0x00000000; // VSH
		*((unsigned int *)0x8805ca40)=0x00000000; // VSHPlain

		//that will write a j MyFunction to AddressOfFunction
		//u32 data;
		//data = (u32) MyFunction & 0xFFFFFFF;
		//data = 0x8000000 | (data >> 2);
		//_sw(data, AddressOfFunction);

		//Patch sceExitGame
		//*((unsigned int *)0x8806882c)=0x03e00008; // disable sceExitGame with jr $ra
		//*((unsigned int *)0x88068830)=0x00001021; // addu v0, zero, zero
#ifdef DEBUG
		printf( "\tKernel patched : \n\t\tPRX loadmodule,\n\t\tsceExitGame\n" );
		sceKernelDelayThread(1000000);
#endif
	}
	else
	{
#ifdef DEBUG
		printf( "Bad OS version, 1.5 required, no patch\n" );
		sceKernelDelayThread(1000000);
#endif
	}
}

//*************************************************************************************
//	
//*************************************************************************************
#ifndef RUN_KERNEL_MODE

__attribute__ ((constructor))
void	loaderInit()
{
	pspKernelSetKernelPC();
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();
	pspDebugInstallKprintfHandler( NULL );
	SetKernelPatch();
}

#endif	// #ifndef RUN_KERNEL_MODE

//*************************************************************************************
//	Framework exit point
//*************************************************************************************
static int	ExitCallback( int arg1, int arg2, void * common )
{
	s_ExitCallbackCalled = true;

	CFrameWork::Close();

	sceKernelExitGame();

	return 0;
}

//*************************************************************************************
//	HOME button callback thread
//*************************************************************************************
static int	CallbackThread( SceSize args, void * argp )
{
	const int cbid( sceKernelCreateCallback( "Exit Callback", ExitCallback, NULL ) );

	sceKernelRegisterExitCallback( cbid );

	sceKernelSleepThreadCB();

	return 0;
}

//*************************************************************************************
//	Setup the HOME button callbacks
//*************************************************************************************
static int	SetupCallbacks()
{
	const int thread_id( sceKernelCreateThread( "update_thread", CallbackThread, 0x11, 0xFA0, 0, 0 ) );

	if ( thread_id >= 0 )
	{
		sceKernelStartThread( thread_id, 0, NULL );
	}

	return thread_id;
}

//*************************************************************************************
//	
//*************************************************************************************
#ifdef RUN_KERNEL_MODE

static void	HomeButtonCallback()
{
/*	bool	time_to_leave( false );

	while ( time_to_leave == false )
	{
		CInput::Process();

		if ( CInput::IsButtonClicked( CInput::HOME ) == true || CInput::IsButtonClicked( CInput::CIRCLE ) == true )
		{
			time_to_leave = true;
		}

		if ( CInput::IsButtonClicked( CInput::CROSS ) == true )
		{
			time_to_leave = true;

			if ( s_ExitCallbackCalled == true )
			{
				ExitCallback( 0, 0, NULL ); //sceKernelExitGame();
			}
		}
	}*/

	ExitCallback( 0, 0, NULL ); //sceKernelExitGame();
}

#endif

//**********************************************************************************
//	
//**********************************************************************************
static void	ExceptionHandler( PspDebugRegBlock * p_regs )
{
	bool	flash( false );

	pspDebugScreenInit();

	pspDebugScreenSetBackColor( 0x00000000 );
	pspDebugScreenSetTextColor( 0xff0000ff );

	pspDebugScreenClear();

	TRACE( "\n\n\n\n\n\n\n\n\n");
	TRACE( "Exception Details:\n" );
	pspDebugDumpException( p_regs );
	TRACE("\n\n\n\n\n\t\tTIME TO FLIP THAT BATTERY!!!\n");

	while ( 1 )
	{
		pspDebugScreenSetXY( 0, 0 );

		if ( flash == true )
		{
			pspDebugScreenSetTextColor( 0xff0000ff );
		}
		else
		{
			pspDebugScreenSetTextColor( 0xff000000 );
		}

		TRACE( "*******************************************************************\n");
		TRACE( "*                                                                 *\n");
		TRACE( "*                                                                 *\n");
		TRACE( "*                                                                 *\n");
		TRACE( "*******************************************************************\n");

		pspDebugScreenSetTextColor( 0xff0000ff );
		pspDebugScreenSetXY( 27, 2 );
		TRACE( "Guru Meditation");

		//
		//	Pause for some time
		//
		for ( u32 i = 0; i < 30; ++i )
		{
			sceDisplayWaitVblankStart();
		}

		flash = ( flash == false );
	}
}

//*************************************************************************************
//	Initialise the framework sub systems
//*************************************************************************************
bool	CFrameWork::Open()
{
	//
	//	Record the start time
	//
	s_TickResolution = sceRtcGetTickResolution();

	sceRtcGetCurrentTick( &s_Time );

	s_LastTime = s_Time;

	//
	//	Patch the Kernel
	//
#ifdef RUN_KERNEL_MODE

	SetKernelPatch();
	pspSdkInstallNoPlainModuleCheckPatch();

#else	// #ifdef RUN_KERNEL_MODE

	pspKernelSetKernelPC();
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();
	pspDebugInstallKprintfHandler( NULL );

#endif	// #ifdef RUN_KERNEL_MODE

	//
	//	Initialise the debug output
	//
	pspDebugScreenInit();

	//
	//	Install my error handler
	//
	pspDebugInstallErrorHandler( ExceptionHandler );

	//
	//	Setup our HOME button callbacks
	//
	SetupCallbacks();

	//
	//	Install soft reset thread
	//
	InstallSoftResetThread();

	//
	//	Set the clock speed
	//
//	scePowerSetBusClockFrequency( 166 );
//	scePowerSetCpuClockFrequency( 333 );

	//
	//	Open the log file
	//
	s_pLogFile = CFileSystem::Open( s_szLogFile, "wb" );

	ASSERT( s_pLogFile != NULL, "Failed to open log file" );

	//
	//	Initialise the random seed
	//
	srand( static_cast< u32 >( time( NULL ) ) );

	//
	//	Open the configuration file
	//
	CConfigFile::Open();

	//
	//	Open the language database
	//
	if ( CLanguage::Open() == false )
	{
		ASSERT( 0, "Failed to initialise the language database" );

		return false;
	}

	//
	//	Open the process manager
	//
	CProcess::Open();

	//
	//	Initialise the USB manager
	//
	if ( CUSBManager::Open() == false )
	{
		ASSERT( 0, "USB Manager failed to open" );

		return false;
	}

	//
	//	Initialise the graphics sub system
	//
	if( CGfx::Open() == false )
	{
		ASSERT( 0, "GPU failed to initialise" );

		return false;
	}

	//
	//	Initialise the PRX manager
	//
	CPRXManager::Open();

	//
	//	Open the render manager
	//
	CRenderable::Open();

	//
	//	Open the skin manager
	//
	CSkinManager::Open();

	//
	//	Open the font system
	//
	CFont::Open();

	//
	//	Initialise the input controller
	//
	if ( CInput::Open() == false )
	{
		ASSERT( 0, "Input Manager failed to open" );

		return false;
	}

	//
	//	Open the file handler class
	//
	if ( CFileHandler::Open() == false )
	{
		ASSERT( 0, "File handler failed to open" );

		return false;
	}

	//
	//	Open the background renderer
	//
	CBackground::Open();

	//
	//	Open the window manager
	//
	CWindow::Open();

	//
	//	In debug we always want the USB connection to be on
	//
	if ( CVAR_USB_ON.Get() == "on" )
	{
		if ( CUSBManager::Activate() == false )
		{
			ASSERT( 0, "Failed to activate USB connection" );
		}
	}

	CFileSystem::MakeReadOnly( "ms0:/71M.png" );

	return true;
}

//*************************************************************************************
//	
//*************************************************************************************
void	CFrameWork::Close()
{
	//
	//	Shut down the USB manager
	//
	CUSBManager::Close();

	//
	//	Close the window manager
	//
	CWindow::Close();

	//
	//	Open the background renderer
	//
	CBackground::Close();

	//
	//	Close the file handler class
	//
	CFileHandler::Close();

	//
	//	Close the input controller
	//
	CInput::Close();

	//
	//	Close the skin manager
	//
	CSkinManager::Close();

	//
	//	Close the font system
	//
	CFont::Close();

	//
	//	Close the PRX manager
	//
	CPRXManager::Close();

	//
	//	Close the render manager
	//
	CRenderable::Close();

	//
	//	Shut down the GPU
	//
	CGfx::Close();

	//
	//	Close the config file
	//
	CConfigFile::Close();

	//
	//	Close the log file
	//
	CFileSystem::Close( s_pLogFile );

	//
	//	Close the process manager
	//
	CProcess::Close();

	TRACE( "CFrameWork::Close\n" );
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CFrameWork::Process()
{
	//
	//	Update the timers
	//
	s_LastTime = s_Time;
	sceRtcGetCurrentTick( &s_Time );

	//
	//	Update the input
	//
	CInput::Process();

	//
	//	Update the process list
	//
	CProcess::ProcessList();

	//
	//	Check for the home button
	//
#ifdef RUN_KERNEL_MODE

	if ( s_ExitCallbackCalled == true )
	{
		ExitCallback( 0, 0, NULL ); //sceKernelExitGame();
	}

	if ( CInput::IsButtonClicked( CInput::HOME ) == true )
	{
		HomeButtonCallback();
	}

#endif

	//
	//	Render everything
	//
	CRenderable::Render();

	//
	//	If we're in debug, allow EBOOT.PBP to be reloaded
	//
#ifdef _DEBUG

	if ( CInput::IsButtonClicked( CInput::SELECT ) == true )
	{
		CPBPFileHandler::Execute( "ms0:/PSP/GAME/FileAs_________________________1/EBOOT.PBP" );

		return false;
	}

#endif	// #ifdef _DEBUG

	return true;
}

//*************************************************************************************
//	
//*************************************************************************************
void	CFrameWork::DBGMessage( const char * fmt, ... )
{
	pspDebugScreenPrintf( fmt );
}

//*************************************************************************************
//	
//*************************************************************************************
u32		CFrameWork::GetTicks()
{
	return s_Time;
}

//*************************************************************************************
//	
//*************************************************************************************
float	CFrameWork::GetElapsedTime()
{
	return ( static_cast< float >( s_Time - s_LastTime ) / s_TickResolution );
}

//*************************************************************************************
//	
//*************************************************************************************
SceUID	CFrameWork::LoadModule( const CString & module, eModulePartition partition )
{
	SceUID	load_id;

	switch ( partition )
	{
	case KERNEL_PARTITION:
		{
			load_id = sceKernelLoadModule( module, 0, NULL );
		}
		break;

	case USER_PARTITION:
		{
			SceKernelLMOption	option;

			option.flags = 0;
			option.access = 1;
			option.position = 0;
			option.mpidtext = 2;
			option.mpiddata = 2;
			option.size = sizeof( option );

			load_id = sceKernelLoadModule( module, option.flags, &option );
		}
		break;
	}

	if ( load_id & 0x80000000 )
	{
		BREAK_POINT( "sceKernelLoadModule - %X", load_id );
	}

	return load_id;
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CFrameWork::LoadAndStartModule( const CString & module, eModulePartition partition )
{
	s32		status;
	SceUID	start_id;
	SceUID	load_id( LoadModule( module, partition ) );

	if ( ( load_id & 0x80000000 ) == 0 )
	{
		start_id = sceKernelStartModule( load_id, 0, NULL, &status, NULL );
	}

	if ( load_id != start_id )
	{
		BREAK_POINT( "sceKernelStartModule - %X, %X", start_id, status );

		return false;
	}

	return true;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFrameWork::RunExecutable( const CString & executable )
{
	SceKernelLoadExecParam	execParam;
	const u32				total_length( executable.Length() + 1 );

	execParam.args = total_length;
	execParam.argp = const_cast< char * >( executable.GetPtr() );
	execParam.key = NULL;
	execParam.size = sizeof( execParam ) + total_length;

	if ( sceKernelLoadExec( executable, &execParam ) < 0 )
	{
		return false;
	}

	return true;
}

//**********************************************************************************
//	
//**********************************************************************************
CFrameWork::eFirmwareVersion	CFrameWork::GetVersion()
{
	if ( s_nVersion == 0x0e017c43 )
	{
		return PSP_v1_5;
	}

	return PSP_v1_0;
}

//*******************************  END OF FILE  ************************************
