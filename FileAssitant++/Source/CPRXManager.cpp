/***********************************************************************************

  Module :	CPRXManager.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 21 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CPRXManager.h"

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

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
void	CPRXManager::Open()
{
/*	const SceUID	mod_id( CFrameWork::LoadModule( CFileSystem::MakeFullPath( "Data/Plugins/jpg.prx" ), CFrameWork::KERNEL_PARTITION ) );
	SceModule *		p_mod( sceKernelFindModuleByUID( mod_id ) );

	if ( p_mod != NULL )
	{
		// get args back
		u32		arg( 0 );
		u32 *	args( &arg );
		int		status;

		const int	ret( sceKernelStartModule( mod_id, sizeof( args ), &args, &status, NULL ) );

		sImagePlugInterface * const p_interface( ( ( GetInterfaceFunc )*args )() );

		BREAK_POINT( "Interface Func : %p\n", p_interface );
		BREAK_POINT( "Plugin Name Func : %p\n", p_interface->name_func );
		BREAK_POINT( "Plugin Name Func Ret = %s\n", p_interface->name_func() );
		BREAK_POINT( "Plugin Load Func : %p\n", p_interface->load_func );

		sTextureInfo * const	p_texture( p_interface->load_func( "ms0:/affair.jpg" ) );

		if ( p_texture != NULL )
		{
			TRACE( "%d\n", p_texture->m_nWidth );
			TRACE( "%d\n", p_texture->m_nHeight );
			TRACE( "%d\n", p_texture->m_nCanvasWidth );
			TRACE( "%d\n", p_texture->m_nCanvasHeight );
			TRACE( "%p\n", p_texture->m_pBuffer );
			TRACE( "%p\n", p_texture->m_pOriginalAddress );
		}

		BREAK_POINT( "DONE!" );
	}
	else
	{
		ASSERT( 0, "Failed to find module!" );
	}*/
}

//**********************************************************************************
//	
//**********************************************************************************
void	CPRXManager::Close()
{
}

//*******************************  END OF FILE  ************************************
