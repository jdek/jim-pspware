/***********************************************************************************

  Module :	COptionsMenu.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 13 August 2005 71M

***********************************************************************************/

#ifndef COPTIONSMENU_H_
#define COPTIONSMENU_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CWindow.h"
#include "CFileHandler.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
class CWindowTable;
class CWindowTableItem;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class COptionsMenu : public CWindow
{
	public:

		enum eOption
		{
			OPTION_MAKE_DIRECTORY,
			OPTION_USB_CONNECTION,
			OPTION_SKIN_SELECTION,
			OPTION_HIDE_CORRUPT_FILES,
			OPTION_STOP_MUSIC,

			MAX_OPTIONS
		};

	public:

		COptionsMenu();
		~COptionsMenu();

		virtual void				Process();
		virtual void				ProcessInput();
		virtual void				SetFocus( bool focus );

	protected:

		void						AddOptions();

		static void					OptionSelectedCallback( CWindowTableItem * p_item, u32 item_no );

	protected:

		CWindowTable *				m_pOptions;

		static const CString		s_szOptionNames[ MAX_OPTIONS ];
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* COPTIONSMENU_H_ */
