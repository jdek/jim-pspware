/***********************************************************************************

  Module :	CSkinManager.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 04 August 2005 71M

***********************************************************************************/

#ifndef CSKINMANAGER_H_
#define CSKINMANAGER_H_

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
class CTexture;
class TiXmlDocument;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************

//**********************************************************************************
//**********************************************************************************
//
//	Skin texture class definition
//
//**********************************************************************************
//**********************************************************************************
class CSkinTexture
{
	public:

		CSkinTexture();
		~CSkinTexture();

		void				Load( const CString & texture_name );

		CTexture *			GetTexture() const;

	private:

		CTexture *			m_pTexture;
};


//**********************************************************************************
//**********************************************************************************
//
//	Skin manager class definition
//
//**********************************************************************************
//**********************************************************************************
class CSkinManager
{
	public:

		enum eSkinComponent
		{
			SC_FONT,

			SC_BACKGROUND,

			SC_WND_BOTTOM,
			SC_WND_BOTTOM_LEFT,
			SC_WND_BOTTOM_RIGHT,
			SC_WND_CENTER,
			SC_WND_LEFT,
			SC_WND_RIGHT,
			SC_WND_TOP,
			SC_WND_TOP_LEFT,
			SC_WND_TOP_RIGHT,

			SC_ICON_AUDIO,
			SC_ICON_BACK,
			SC_ICON_DRIVE,
			SC_ICON_EXECUTABLE,
			SC_ICON_FILE,
			SC_ICON_FOLDER,
			SC_ICON_IMAGE,
			SC_ICON_MOVIE,
			SC_ICON_TEXT,
			SC_ICON_USB,

			SC_BUTTON_CROSS,
			SC_BUTTON_CIRCLE,
			SC_BUTTON_SQUARE,
			SC_BUTTON_TRIANGLE,
			SC_BUTTON_START,
			SC_BUTTON_SELECT,

			SC_BATTERY_00,
			SC_BATTERY_01,
			SC_BATTERY_02,
			SC_BATTERY_03,

			SC_PROGRESS_BAR_EMPTY,
			SC_PROGRESS_BAR_FULL,

			MAX_SKIN_COMPONENTS,

			SC_WND_COMPONENT_START	= SC_WND_TOP,
			SC_WND_COMPONENT_END	= SC_WND_CENTER + 1,
			NUM_WND_COMPONENTS		= SC_WND_COMPONENT_END - SC_WND_COMPONENT_START,
		};

	public:

		static void				Open();
		static void				Close();

		static void				SetCurrentSkin( const CString & skin_name );

		static CSkinTexture *	GetComponent( eSkinComponent component );

		static V2				GetV2( const CString & section, const CString & sub_section, const V2 & default_val );
		static ARGB				GetColor( const CString & section, const CString & sub_section, const ARGB & default_val );
		static float			GetFloat( const CString & section, const CString & sub_section, const float & default_val );

	protected:

		static void				LoadConfig();

	public:

		static const CString	s_szSkinFolder;

	private:

		static bool				s_bConfigLoaded;
		static TiXmlDocument *	s_pSkinConfigFile;
		static const CString	s_szComponentNames[ MAX_SKIN_COMPONENTS ];
		static CSkinTexture *	s_pComponents[ MAX_SKIN_COMPONENTS ];
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CSKINMANAGER_H_ */
