/***********************************************************************************

  Module :	CMessageBox.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 05 August 2005 71M

***********************************************************************************/

#ifndef CMESSAGEBOX_H_
#define CMESSAGEBOX_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CWindow.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
class CWindowText;

typedef	void	( * ButtonCallback )();

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CMessageBox : public CWindow
{
	public:

		enum eButton
		{
			CROSS_CALLBACK,
			CIRCLE_CALLBACK,
			SQUARE_CALLBACK,
			TRIANGLE_CALLBACK,

			MAX_BUTTON_CALLBACKS
		};

	public:

		CMessageBox();

		virtual void		Render();
		virtual void		ProcessInput();

		void				SetText( const CString & text );
		void				SetCallback( eButton button, ButtonCallback callback );

	private:

		CWindowText *		m_pText;
		ButtonCallback		m_Callback[ MAX_BUTTON_CALLBACKS ];
};

//**********************************************************************************
//	
//**********************************************************************************
class CModalMessageBox : public CMessageBox
{
	public:

		enum eExitCode
		{
			EXIT_NULL,
			EXIT_CROSS,
			EXIT_CIRCLE,
			EXIT_SQUARE,
			EXIT_TRIANGLE,
		};

		CModalMessageBox( const CString & title, const CString & text );

		eExitCode			Show();

		void				AddExitCode( eExitCode code, const CString & text );

		virtual void		ProcessInput();

	private:

		bool				m_bExitCross;
		bool				m_bExitCircle;
		bool				m_bExitSquare;
		bool				m_bExitTriangle;
};

//**********************************************************************************
//	
//**********************************************************************************
class CErrorMessage : public CModalMessageBox
{
	public:

		CErrorMessage( const CString & error_msg );
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CMESSAGEBOX_H_ */
