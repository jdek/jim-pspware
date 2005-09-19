/***********************************************************************************

  Module :	CHUD.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 07 August 2005 71M

***********************************************************************************/

#ifndef CHUD_H_
#define CHUD_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CString.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
class CProgressBar;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CHUD
{
	public:

		enum eButton
		{
			BUTTON_CROSS,
			BUTTON_CIRCLE,
			BUTTON_SQUARE,
			BUTTON_TRIANGLE,
			BUTTON_START,
			BUTTON_SELECT,

			MAX_BUTTONS
		};

	public:

		static void				Create();
		static void				Destroy();

		static void				Show( bool show );
		static bool				IsVisible();

		static void				SetButton( eButton button, const CString & text );
		static void				SetButtons( const CString & cross, const CString & circle, const CString & square, const CString & triangle );

		static void				ShowProgressBar( bool show, const CString & title );
		static float			GetProgressBar();
		static void				SetProgressBar( float val );

	private:

		CHUD();
		~CHUD();

		static void				Render();
		void					RenderInternal();

	private:

		static bool				s_bVisible;

		static CHUD *			s_pInstance;
		static CString			s_szButtonText[ MAX_BUTTONS ];

		static float			s_fProgressBar;
		static bool				s_bShowProgressBar;
		static CString			s_szProgressTitle;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CHUD_H_ */
