/***********************************************************************************

  Module :	CWindowTable.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 03 August 2005 T Swann

***********************************************************************************/

#ifndef CWINDOWTABLE_H_
#define CWINDOWTABLE_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CWindowItem.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
class CWindowTable;
class CWindowTableItem;

typedef std::list< CWindowTableItem * >	CTableItemList;

typedef	int		( * Compare )( const void * arg1, const void * arg2 );
typedef	void	( * SelectCallback )( CWindowTableItem * p_item, u32 item_no );


//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
class CWindowTableItem
{
	public:

		CWindowTableItem();
		virtual ~CWindowTableItem();

		virtual void		Process();
		virtual V2			Render( V2 pos, bool highlight, float scroll_offset );

		virtual V2			GetSize() const;

		CWindowTable *		GetParent() const;
		void				SetParent( CWindowTable * const p_parent );

		void				Select( u32 item_no );
		void				SetCallback( SelectCallback callback );

	protected:

		CWindowTable *		m_pParent;
		SelectCallback		m_pCallback;
};

//*************************************************************************************
//	
//*************************************************************************************
class CWindowTable : public CWindowItem
{
	public:

		CWindowTable();
		~CWindowTable();

		virtual void			Render();
		virtual void			Process();
		virtual void			ProcessInput();

		void					AddItem( CWindowTableItem * const p_item );
		void					ClearItems( bool maintain_selection = false );
		CTableItemList &		GetItemList();

		void					Sort( Compare compare_func );

		void					ResetSelection();
		u32						GetCurrentSelection();
		void					SetCurrentSelection( u32 selection );
		CWindowTableItem *		GetSelectedItem();

		V2						GetAllItemsSize();

		void					ScrollUp();
		void					ScrollDown();

	protected:

		void					CapSelection();
		void					CalculateMaxDisplayItems();

	protected:

		CTableItemList			m_Items;

		s32						m_nCursorPos;
		s32						m_nItemsOffset;
		s32						m_nMaxDisplayItems;
		float					m_fScrollOffset;
		s32						m_nRepeatCount;

		bool					m_bSizeDirty;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CWINDOWTABLE_H_ */
