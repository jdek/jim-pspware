/***********************************************************************************

  Module :	CInput.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include <pspctrl.h>
#include "CInput.h"
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
static SceCtrlData	s_PadData;
static bool			s_bPrevButton[ CInput::MAX_BUTTONS ];
static s32			s_nButtonRepeat[ CInput::MAX_BUTTONS ];
static const u32	s_ButtonMap[ CInput::MAX_BUTTONS ] =
{
	PSP_CTRL_SELECT,	// SELECT
	PSP_CTRL_START,		// START
	PSP_CTRL_UP,		// UP
	PSP_CTRL_RIGHT,		// RIGHT
	PSP_CTRL_DOWN,		// DOWN
	PSP_CTRL_LEFT,		// LEFT
	PSP_CTRL_LTRIGGER,	// LTRIGGER
	PSP_CTRL_RTRIGGER,	// RTRIGGER
	PSP_CTRL_TRIANGLE,	// TRIANGLE
	PSP_CTRL_CIRCLE,	// CIRCLE
	PSP_CTRL_CROSS,		// CROSS
	PSP_CTRL_SQUARE,	// SQUARE
	PSP_CTRL_HOME,		// HOME
	PSP_CTRL_HOLD,		// HOLD
	PSP_CTRL_NOTE,		// NOTE
};

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//*************************************************************************************
//	
//*************************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
bool	CInput::Open()
{
	for ( u32 b = 0; b < MAX_BUTTONS; ++b )
	{
		s_bPrevButton[ b ] = false;
	}

	sceCtrlSetSamplingCycle( 0 );
	sceCtrlSetSamplingMode( PSP_CTRL_MODE_ANALOG );

	return true;
}

//*************************************************************************************
//	
//*************************************************************************************
void	CInput::Close()
{
}

//*************************************************************************************
//	
//*************************************************************************************
void	CInput::Process()
{
	for ( u32 i = 0; i < MAX_BUTTONS; ++i )
	{
		s_bPrevButton[ i ] = ( s_PadData.Buttons & s_ButtonMap[ i ] );
	}

	for ( u32 i = 0; i < MAX_BUTTONS; ++i )
	{
		if ( IsButtonDown( static_cast< eButton >( i ) ) == true )
		{
			s_nButtonRepeat[ i ]++;
		}
		else
		{
			s_nButtonRepeat[ i ] = 0;
		}
	}

	sceCtrlReadBufferPositive( &s_PadData, 1 );
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CInput::IsButtonDown( eButton button )
{
	return ( s_PadData.Buttons & s_ButtonMap[ button ] );
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CInput::IsButtonClicked( eButton button )
{
	if ( IsButtonDown( button ) == true )
	{
		if ( s_bPrevButton[ button ] == false )
		{
			s_bPrevButton[ button ] = true;

			return true;
		}
	}

	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CInput::IsButtonRepeat( eButton button, s32 repeat )
{
	if ( IsButtonDown( button ) == true )
	{
		return ( ( s_nButtonRepeat[ button ] % repeat ) == 0 );
	}

	return false;
}

//*************************************************************************************
//	
//*************************************************************************************
V2	CInput::GetAnalogStick()
{
	float	analog_x( static_cast< float >( s_PadData.Lx ) );
	float	analog_y( static_cast< float >( s_PadData.Ly ) );

	analog_x *= 2.f;
	analog_y *= 2.f;

	analog_x /= 255.f;
	analog_y /= 255.f;

	analog_x -= 1.f;
	analog_y -= 1.f;

	return V2( analog_x, analog_y );
}

//*******************************  END OF FILE  ************************************
