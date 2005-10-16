/***********************************************************************************

  Module :	CTextInput.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 13 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CFrameWork.h"
#include "CTextInput.h"
#include "CHUD.h"
#include "CInput.h"
#include "CGfx.h"
#include "CFont.h"

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
static const float	TEXT_INPUT_WIDTH( 256.f );
static const float	TEXT_INPUT_HEIGHT( 48.f );

//**********************************************************************************
//   Static Prototypes
//**********************************************************************************

//**********************************************************************************
//   Global Variables
//**********************************************************************************

//**********************************************************************************
//   Static Variables
//**********************************************************************************

//**********************************************************************************
//   Text input class implementation
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
CTextInput::CTextInput( const CString & title, const CString & text )
:	m_szText( text )
,	m_pKeyboard( NULL )
{
	m_pKeyboard = new CKeyboard( this );

	SetTitle( title );

	SetPos( V2( 0.5f * ( CGfx::s_ScreenWidth - TEXT_INPUT_WIDTH ), 0.5f * ( CGfx::s_ScreenHeight - TEXT_INPUT_HEIGHT ) ) );
	SetSize( V2( TEXT_INPUT_WIDTH, TEXT_INPUT_HEIGHT ) );
	SetFocus( true );

	CHUD::SetButtons( "Select", "Cancel", "Delete", "Accept" );
}

//**********************************************************************************
//	
//**********************************************************************************
CTextInput::~CTextInput()
{
	SAFE_DELETE( m_pKeyboard );
}

//**********************************************************************************
//	
//**********************************************************************************
void	CTextInput::Render()
{
	CWindow::Render();

	V2				pos( GetScreenPos() );
	CFont * const	p_font( CFont::GetDefaultFont() );
	const V2		text_size( p_font->GetStringSize( m_szText ) );

	pos.x += 0.5f * ( TEXT_INPUT_WIDTH - text_size.x );
	pos.y += 0.5f * ( TEXT_INPUT_HEIGHT - text_size.y );

	p_font->Print( m_szText, pos, GetTextColor() );
}

//**********************************************************************************
//	
//**********************************************************************************
CTextInput::eReturnCode	CTextInput::Show()
{
	CProcess::Pause( true );

	while ( 1 )
	{
		CFrameWork::Process();

		m_pKeyboard->ProcessInput();

		if ( CInput::IsButtonClicked( CInput::TRIANGLE ) == true )
		{
			CProcess::Pause( false );

			return RC_OK;
		}

		if ( CInput::IsButtonClicked( CInput::CIRCLE ) == true )
		{
			CProcess::Pause( false );

			return RC_CANCEL;
		}
	}

	return RC_CANCEL;
}

//**********************************************************************************
//	
//**********************************************************************************
const CString &	CTextInput::GetText() const
{
	return m_szText;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CTextInput::Message( CKeyboardListener::eMessage message, const void * const p_data )
{
	char * const	p_string( const_cast< char * >( m_szText.GetPtr() ) );

	switch ( message )
	{
	case MSG_DELETE:
		{
			const u32	length( m_szText.Length() );

			if ( length > 0 )
			{
				p_string[ length - 1 ] = '\0';
			}
		}
		break;

	case MSG_CHARACTER:
		{
			const char	buffer[ 2 ] = { static_cast< char >( ( u32 )( p_data ) ), '\0' };

			m_szText += buffer;
		}
		break;

	case MSG_CURSOR:
		{
		}
		break;

	default:
		{
			ASSERT( 0, "Received unknown keyboard message" );
		}
		break;
	}
}

//*******************************  END OF FILE  ************************************
