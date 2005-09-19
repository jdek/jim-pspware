/***********************************************************************************

  Module :	CInformationDialog.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 21 August 2005 71M

***********************************************************************************/

#ifndef CINFORMATIONDIALOG_H_
#define CINFORMATIONDIALOG_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CMessageBox.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
struct sDirEntry;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************
class CInformationDialog : public CModalMessageBox
{
	public:

		CInformationDialog( const sDirEntry * const p_file_info );
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CINFORMATIONDIALOG_H_ */
