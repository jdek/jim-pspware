/***********************************************************************************

  Module :	CFileAssistant.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 05 August 2005 71M

***********************************************************************************/

#ifndef CFILEASSISTANT_H_
#define CFILEASSISTANT_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CProcess.h"
#include "CDirectoryList.h"

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
class CFileAssistant : public CProcess
{
	public:

		static void				Create();
		static void				Destroy();

		static CFileAssistant *	Get();

	public:

		virtual void			Process();
		virtual void			ProcessInput();

		CDirectoryList *		GetSrcList() const;
		CDirectoryList *		GetDstList() const;
		CDirectoryList *		GetFocusList() const;

		void					SetListFocus( bool focus );

	private:

		CFileAssistant();
		~CFileAssistant();

	private:

		CDirectoryList *		m_pSrcList;
		CDirectoryList *		m_pDstList;

		CDirectoryList *		m_pListFocus;

		float					m_fSrcScale;
		float					m_fDstScale;

		bool					m_bSrcInFocus;

	private:

		static CFileAssistant *	s_pInstance;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CFILEASSISTANT_H_ */
