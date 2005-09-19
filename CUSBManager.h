/***********************************************************************************

  Module :	CUSBManager.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

#ifndef CUSBMANAGER_H_
#define CUSBMANAGER_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************

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
class CUSBManager
{
	public:

		// Initialise the USB manager
		static bool		Open();

		// Shutdown the USB manager
		static void		Close();

		// Activate the USB connection, returns true if successful
		static bool		Activate();

		// Deactivate the USB connection, returns true if successful
		static bool		Deactivate();

		// Returns true if the USB connection is active
		static bool		IsActive();

		// Returns true if the USB cable is plugged in
		static bool		CableConnected();

		// Returns true if the USB cable is plugged in
		static bool		ConnectionEstablished();
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CUSBMANAGER_H_ */
