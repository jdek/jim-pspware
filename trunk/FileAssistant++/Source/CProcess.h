/***********************************************************************************

  Module :	CProcess.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 03 August 2005 T Swann

***********************************************************************************/

#ifndef CPROCESS_H_
#define CPROCESS_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
class CProcess;

typedef std::list< CProcess * >	CProcessList;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CProcess
{
	public:

		static void				Open();
		static void				Close();
		static void				ProcessList();

		static void				Pause( bool pause );
		static bool				IsPaused();

	public:

		CProcess();

		void					Delete();

		void					MoveToBackOfList();
		void					MoveToFrontOfList();

		virtual void			Process() = 0;
		virtual void			ProcessInput() = 0;

	protected:

		virtual ~CProcess();

	private:

		static bool				s_bPaused;
		static CProcessList		s_ProcessList;
		static CProcessList		s_NewProcessList;
		static CProcessList		s_DeletedProcessList;
		static CProcessList		s_MoveToBackProcessList;
		static CProcessList		s_MoveToFrontProcessList;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CPROCESS_H_ */
