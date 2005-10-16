/***********************************************************************************

  Module :	CTypes.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 03 August 2005 T Swann

***********************************************************************************/

#ifndef CTYPES_H_
#define CTYPES_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <psptypes.h>
#include <pspsysmem.h>
#include <psploadexec.h>
#include <psprtc.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspaudio.h>
#include <pspaudiolib.h>
#include <psphprm.h>
#include <pspiofilemgr.h>
#include <pspumd.h>
#include <psppower.h>
#include <list>
#include <vector>
#include "CMath.h"
#include "CVector.h"
#include "CAssert.h"

//**********************************************************************************
//   Macros
//**********************************************************************************
#define SAFE_DELETE( x )		\
{								\
	delete x;					\
	x = NULL;					\
}

#define SAFE_RDELETE( x )		\
{								\
	delete [] x;				\
	x = NULL;					\
}

#define MAKE_ARGB( a, r, g, b )	( ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | ( r << 0 ) )

//**********************************************************************************
//   Types
//**********************************************************************************
struct ARGB
{
	ARGB() : color( 0x00000000 )	{}
	ARGB( u32 col ) : color( col )	{}
	ARGB( u8 r, u8 g, u8 b ) : color( MAKE_ARGB( 255, r, g, b ) )		{}
	ARGB( u8 a, u8 r, u8 g, u8 b ) : color( MAKE_ARGB( a, r, g, b ) )	{}

	union
	{
		u32	color;

		struct
		{
			u8	r, g, b, a;//a, b, g, r;
		};
	};
};

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************


#endif /* CTYPES_H_ */
