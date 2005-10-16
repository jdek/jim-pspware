/***********************************************************************************

  Module :	CUSBManager.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include <pspusb.h>
#include <pspusbstor.h>
#include "CUSBManager.h"
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

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
bool	CUSBManager::Open()
{
	//
	//	Start necessary drivers
	//
	CFrameWork::LoadAndStartModule( "flash0:/kd/semawm.prx", CFrameWork::KERNEL_PARTITION );
	CFrameWork::LoadAndStartModule( "flash0:/kd/usbstor.prx", CFrameWork::KERNEL_PARTITION );
	CFrameWork::LoadAndStartModule( "flash0:/kd/usbstormgr.prx", CFrameWork::KERNEL_PARTITION );
	CFrameWork::LoadAndStartModule( "flash0:/kd/usbstorms.prx", CFrameWork::KERNEL_PARTITION );
	CFrameWork::LoadAndStartModule( "flash0:/kd/usbstorboot.prx", CFrameWork::KERNEL_PARTITION );

	//
	//	Setup USB drivers
	//
	if ( sceUsbStart( PSP_USBBUS_DRIVERNAME, 0, 0 ) != 0 )
	{
		ASSERT( 0, "Error starting USB Bus driver\n" );

		return false;
	}

	if ( sceUsbStart( PSP_USBSTOR_DRIVERNAME, 0, 0 ) != 0 )
	{
		ASSERT( 0, "Error starting USB Mass Storage driver\n " );

		return false;
	}

	if ( sceUsbstorBootSetCapacity( 0x800000 ) != 0 )
	{
		ASSERT( 0, "Error setting capacity with USB Mass Storage driver\n" );

		return false;
	}

	return true;
}

//*************************************************************************************
//	
//*************************************************************************************
void	CUSBManager::Close()
{
	if ( IsActive() == true )
	{
		if ( Deactivate() == false )
		{
			ASSERT( 0, "Error closing USB connection" );
		}
	}

	if ( sceUsbStop( PSP_USBSTOR_DRIVERNAME, 0, 0 ) != 0 )
	{
		ASSERT( 0, "Error stopping USB Mass Storage driver\n" );
	}

	if ( sceUsbStop( PSP_USBBUS_DRIVERNAME, 0, 0 ) != 0 )
	{
		ASSERT( 0, "Error stopping USB BUS driver\n" );
	}
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CUSBManager::Activate()
{
	if ( sceUsbGetState() & PSP_USB_ACTIVATED )
	{
		return false;
	}

	return ( sceUsbActivate( 0x1c8 ) == 0 );
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CUSBManager::Deactivate()
{
	if ( sceUsbGetState() & PSP_USB_ACTIVATED )
	{
		return ( sceUsbDeactivate() == 0 );
	}

	return false;
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CUSBManager::IsActive()
{
	return ( sceUsbGetState() & PSP_USB_ACTIVATED );
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CUSBManager::CableConnected()
{
	return ( sceUsbGetState() & PSP_USB_CABLE_CONNECTED );
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CUSBManager::ConnectionEstablished()
{
	return ( sceUsbGetState() & PSP_USB_CONNECTION_ESTABLISHED );
}

//*******************************  END OF FILE  ************************************
