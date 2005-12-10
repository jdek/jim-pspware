/***********************************************************************************

  Module :	CInput.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

#ifndef CINPUT_H_
#define CINPUT_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CVector.h"

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
class CInput
{
	public:

		enum eButton
		{
			SELECT,
			START,
			UP,
			RIGHT,
			DOWN,
			LEFT,
			LTRIGGER,
			RTRIGGER,
			TRIANGLE,
			CIRCLE,
			CROSS,
			SQUARE,
			HOME,
			HOLD,
			NOTE,

			MAX_BUTTONS
		};

		static bool	Open();
		static void	Close();
		static void	Process();

		static bool	IsButtonDown( eButton button );
		static bool	IsButtonClicked( eButton button );
		static bool	IsButtonRepeat( eButton button, s32 repeat );

		static V2	GetAnalogStick();
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CINPUT_H_ */