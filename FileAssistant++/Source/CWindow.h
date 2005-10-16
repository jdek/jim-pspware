/***********************************************************************************

  Module :	CWindow.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 03 August 2005 T Swann

***********************************************************************************/

#ifndef CWINDOW_H_
#define CWINDOW_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CString.h"
#include "CProcess.h"
#include "CSizedItem.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
class CWindowItem;

typedef std::list< CWindowItem * >	CWindowItemList;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
class CWindow : public CProcess, public CSizedItem
{
	public:

		static void		Open();
		static void		Close();

		static void		RenderAll();

		static void		DrawWindow( const V2 & pos, const V2 & size, ARGB color );

	public:

		CWindow();

		virtual void	Render();
		virtual void	Process();
		virtual void	ProcessInput();

		ARGB			GetColor();
		ARGB			GetTextColor();
		ARGB			GetTitleColor();

		void			AddItem( CWindowItem * const p_item );
		void			RemoveItem( CWindowItem * const p_item );
		void			ClearItems();

		const CString &	GetTitle() const;
		void			SetTitle( const CString & title );

		bool			HasFocus() const;
		void			SetFocus( bool focus );

		bool			IsVisible() const;
		void			SetVisible( bool visible );

	protected:

		virtual ~CWindow();

	protected:

		CWindowItemList	m_ItemList;
		CString			m_szTitle;

		bool			m_bInFocus;
		bool			m_bVisible;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CWINDOW_H_ */
