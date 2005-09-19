/***********************************************************************************

  Module :	Main.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 27 August 2005 71M

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>
#include <malloc.h>
#include <setjmp.h>
#include <CPluginImageInterface.h>
//#include <jpeglib.h>

//**********************************************************************************
//   Local Macros
//**********************************************************************************

//**********************************************************************************
//   Local Constants
//**********************************************************************************
PSP_MODULE_INFO( "JPG LOADER", 0x1000, 1, 1 );

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
static sImagePlugInterface	s_PluginInterface;
/*
typedef struct
{
	struct jpeg_error_mgr	pub;
	jmp_buf					setjmp_buffer;
} my_error_mgr;

typedef my_error_mgr *	my_error_ptr;
*/

//**********************************************************************************
//	
//**********************************************************************************
/*void	my_error_exit( j_common_ptr cinfo )
{
	// cinfo->err really points to a my_error_mgr struct, so coerce pointer
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	// Always display the message.
	// We could postpone this until after returning, if we chose.
	(*cinfo->err->output_message) (cinfo);

	// Return control to the setjmp point
	longjmp( myerr->setjmp_buffer, 1 );
}

//*************************************************************************************
//	
//*************************************************************************************
int	ReadJPG( FILE * p_file, sTextureInfo * p_info )
{
	struct jpeg_decompress_struct	cinfo;
	JSAMPARRAY				buffer;
	my_error_mgr			jerr;

	cinfo.err = jpeg_std_error( &jerr.pub );
	jerr.pub.error_exit = my_error_exit;

	if ( setjmp( jerr.setjmp_buffer ) != 0 )
	{
		jpeg_destroy_decompress( &cinfo );

		return 0;
	}

	jpeg_create_decompress( &cinfo );

	jpeg_stdio_src( &cinfo, p_file );

	jpeg_read_header( &cinfo, TRUE );

	jpeg_calc_output_dimensions( &cinfo );

	buffer = ( *cinfo.mem->alloc_sarray )( ( j_common_ptr )&cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1 );

	jpeg_start_decompress( &cinfo );

	if ( cinfo.out_color_components >= 3 )
	{
		p_info->m_nWidth = cinfo.output_width;
		p_info->m_nHeight = cinfo.output_height;
		p_info->m_nCanvasWidth = 1;
		p_info->m_nCanvasHeight = 1;

		//
		//	Round the width and height up to a power of 2
		//
		while ( p_info->m_nCanvasWidth < p_info->m_nWidth )
		{
			p_info->m_nCanvasWidth <<= 1;
		}

		while ( p_info->m_nCanvasHeight < p_info->m_nHeight )
		{
			p_info->m_nCanvasHeight <<= 1;
		}

		//
		//	Allign the memory to 16 bytes
		//
//		m_pOriginalAddress = new u8[ ( 4 * m_nCanvasWidth * m_nCanvasHeight ) + 16 ];
//		m_pBuffer = reinterpret_cast< u8 * >( ( ( reinterpret_cast< u32 >( m_pOriginalAddress ) + 16 ) & 0xfffffff0 ) );

//		u8 *	p_dest( p_texture->m_pBuffer );

		while ( cinfo.output_scanline < cinfo.output_height )
		{
//			JSAMPLE *		p_src( *buffer );
			jpeg_read_scanlines( &cinfo, buffer, 1 );

//			for ( u32 x = 0; x < cinfo.output_width; ++x )
//			{
//				for ( s32 c = 0; c < cinfo.out_color_components; ++c )
//				{
//					p_dest[ ( x << 2 ) + c ] = *p_src++;
//				}
//
//				if ( cinfo.out_color_components == 3 )
//				{
//					p_dest[ ( x << 2 ) + 3 ] = 0xff;
//				}
//			}
//
//			p_dest += ( p_texture->m_nCanvasWidth << 2 );
		}
	}

	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	return 1;
}*/

//**********************************************************************************
//	
//**********************************************************************************
int thread_func( SceSize args, void * argp )
{
	sceKernelSleepThread();

	return 0;
}

//**********************************************************************************
//	
//**********************************************************************************
sImagePlugInterface *	GetInterface()
{
	return &s_PluginInterface;
}

//**********************************************************************************
//	
//**********************************************************************************
const char *	GetPluginName()
{
	return "JPG PLUGIN";
}

//**********************************************************************************
//	
//**********************************************************************************
sTextureInfo *	LoadTexture( const char * p_filename )
{
	FILE *	p_file = fopen( p_filename, "rb" );

	if ( p_file != NULL )
	{
		SceUID	mem_id = sceKernelAllocPartitionMemory( 2, "test", PSP_SMEM_High, sizeof( sTextureInfo ), NULL );

		if ( mem_id >= 0 )
		{
			sTextureInfo *	p_info = ( sTextureInfo * )( sceKernelGetBlockHeadAddr( mem_id ) );

			if ( p_info != NULL )
			{
/*				if ( ReadJPG( p_file, p_info ) )
				{
					return p_info;
				}*/
			}
		}
	}

	return NULL;
}

//**********************************************************************************
//	
//**********************************************************************************
int	module_start( SceSize args, void * argp )
{
	int	thid = sceKernelCreateThread( "JPGThread", thread_func, 0x19, 0x10000, 0, NULL );

	if ( thid >= 0 )
	{
		sceKernelStartThread( thid, args, argp );
	}

	s_PluginInterface.name_func = GetPluginName;
	s_PluginInterface.load_func = LoadTexture;

	//send back fn pointer
	int **	p_array = (int**) argp ;
	int *	p_dest = *p_array;

	*p_dest = (int)GetInterface;

	return 0;
}

int	main( int argc, char * argv[] )
{
	return 0;
}

//*******************************  END OF FILE  ************************************
