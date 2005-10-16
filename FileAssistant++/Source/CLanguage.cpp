/***********************************************************************************

  Module :	CLanguage.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 07 September 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CLanguage.h"
#include <psputility.h>

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
eLanguage	CLanguage::s_eLanguage( LANG_ENGLISH );

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
bool	CLanguage::Open()
{
	s32	val;

	if ( sceUtilityGetSystemParamInt( PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &val ) != PSP_SYSTEMPARAM_RETVAL_FAIL )
	{
		switch ( val )
		{
		case PSP_SYSTEMPARAM_LANGUAGE_JAPANESE:
			s_eLanguage = LANG_JAPANESE;
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_ENGLISH:
			s_eLanguage = LANG_ENGLISH;
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_FRENCH:
			s_eLanguage = LANG_FRENCH;
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_SPANISH:
			s_eLanguage = LANG_SPANISH;
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_GERMAN:
			s_eLanguage = LANG_GERMAN;
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_ITALIAN:
			s_eLanguage = LANG_ITALIAN;
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_DUTCH:
			s_eLanguage = LANG_DUTCH;
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_PORTUGUESE:
			s_eLanguage = LANG_PORTUGESE;
			break;
		case PSP_SYSTEMPARAM_LANGUAGE_KOREAN:
			s_eLanguage = LANG_KOREAN;
			break;
		default:
			break;
		}

		return true;
	}

	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CLanguage::Close()
{
}

//**********************************************************************************
//	
//**********************************************************************************
const char * const	CLanguage::Translate( ePhrase phrase )
{
	return "TEXT";
}

//*******************************  END OF FILE  ************************************
