/***********************************************************************************

  Module :	CFileSystem.cpp

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CFrameWork.h"
#include "CFileSystem.h"
#include "CHUD.h"
#include "CConfigFile.h"

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
static CVarInt		CVAR_HIDE_CORRUPT_FILES( "hide_corrupt_files", 0 );
static const s32	COPY_SIZE_IN_BYTES( 64 * 1024 );

SceIoDirent			CFileSystem::s_DirEntry;
CString				CFileSystem::s_szRootDirectory( "" );

//**********************************************************************************
//   Class Definition
//**********************************************************************************

//*************************************************************************************
//*************************************************************************************
//
//	File implenentation
//
//*************************************************************************************
//*************************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
CFile::CFile( const CString & filename, FILE * const p_handle )
:	m_szFilename( filename )
,	m_pHandle( p_handle )
,	m_Length( 0 )
{
	if ( Seek( 0, SEEK_END ) == true )
	{
		m_Length = Tell();

		if ( Seek( 0, SEEK_SET ) == false )
		{
			ASSERT( 0, "Failed to seek to beginning of file" );
		}
	}
	else
	{
		ASSERT( 0, "Failed to seek to the end of file" );
	}
}

//*************************************************************************************
//	
//*************************************************************************************
CFile::~CFile()
{
	fclose( m_pHandle );
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CFile::Read( void * p_address, u32 length, u32 * p_nbytes_read ) const
{
	const u32	bytes_read( fread( p_address, 1, length, m_pHandle ) );

	if ( p_nbytes_read != NULL )
	{
		*p_nbytes_read = bytes_read;
	}

	return ( bytes_read == length );
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CFile::Write( const void * p_address, u32 length, u32 * p_nbytes_written ) const
{
	const u32	bytes_written( fwrite( p_address, 1, length, m_pHandle ) );

	if ( p_nbytes_written != NULL )
	{
		*p_nbytes_written = bytes_written;
	}

	return ( bytes_written == length );
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CFile::IsEOF() const
{
	return ( feof( m_pHandle ) != 0 );
}

//*************************************************************************************
//	
//*************************************************************************************
u32	CFile::GetLength() const
{
	return m_Length;
}

//*************************************************************************************
//	
//*************************************************************************************
u32	CFile::Tell() const
{
	return ftell( m_pHandle );
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CFile::Seek( const u32 offset, const u32 origin ) const
{
	return ( fseek( m_pHandle, offset, origin ) == 0 );
}

//*************************************************************************************
//	
//*************************************************************************************
CString	CFile::GetExtension() const
{
	return CFileSystem::GetFileExtension( m_szFilename );
}

//*************************************************************************************
//	
//*************************************************************************************
char	CFile::FGetC() const
{
	char	character;

	if ( Read( &character, 1 ) == true )
	{
		return character;
	}

	return EOF;
}

//*************************************************************************************
//	
//*************************************************************************************
CFile::operator FILE * () const
{
	return m_pHandle;
}


//*************************************************************************************
//*************************************************************************************
//
//	File system implementation
//
//*************************************************************************************
//*************************************************************************************

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::SetRoot( const CString & directory )
{
	if ( sceIoChdir( directory ) >= 0 )
	{
		s_szRootDirectory = directory;

		return true;
	}

	return false;
}

//*************************************************************************************
//	
//*************************************************************************************
CFile *	CFileSystem::Open( const CString & filename, const char * const p_open_flags )
{
	FILE * const	p_handle( fopen( filename, p_open_flags ) );

	if ( p_handle == NULL )
	{
		return NULL;
	}

	return new CFile( filename, p_handle );
}

//*************************************************************************************
//	
//*************************************************************************************
void	CFileSystem::Close( CFile * const p_file )
{
	delete p_file;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::FileExists( const CString & filename )
{
	CFile * const	p_file( Open( filename, "rb" ) );

	if ( p_file == NULL )
	{
		return false;
	}

	Close( p_file );

	return true;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::DirectoryExists( const CString & directory )
{
	CFile * const	p_file( Open( directory, "rb" ) );

	if ( p_file == NULL )
	{
		return false;
	}

	Close( p_file );

	return true;
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CFileSystem::FindFirstFile( const CString & path, FIND_FILE_HANDLE & handle )
{
	handle = sceIoDopen( path );

	return ( handle >= 0 );
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CFileSystem::FindNextFile( sDirEntry & dir_entry, FIND_FILE_HANDLE handle )
{
	ASSERT( handle >= 0, "Cannot search with invalid directory handle" );

	const s32	error_code( sceIoDread( handle, &s_DirEntry ) );

	ASSERT( error_code >= 0, "Error reading directory entry" );

	if ( error_code > 0 )
	{
		dir_entry.m_Stats = s_DirEntry.d_stat;
		dir_entry.m_szFileName = s_DirEntry.d_name;

		return true;
	}

	return false;
}

//*************************************************************************************
//	
//*************************************************************************************
bool	CFileSystem::FindCloseFile( FIND_FILE_HANDLE handle )
{
	ASSERT( handle >= 0, "Trying to close an invalid directory handle" );

	return ( sceIoDclose( handle ) >= 0 );
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::GetDirectoryFiles( const CString & directory, CFileList & dir_files )
{
	bool				ret_code( false );
	FIND_FILE_HANDLE	handle;

	if ( FindFirstFile( directory, handle ) == true )
	{
		sDirEntry	dir_entry;

		ret_code = true;

		while ( FindNextFile( dir_entry, handle ) == true && ret_code == true )
		{
			if ( dir_entry.m_szFileName != "." && dir_entry.m_szFileName != ".." )
			{
				dir_entry.m_szFileName = CString( directory + CString( "/" ) + dir_entry.m_szFileName );

				dir_files.push_back( dir_entry );

				if ( dir_entry.IsDirectory() == true )
				{
					ret_code = GetDirectoryFiles( dir_entry.m_szFileName, dir_files );
				}
			}
		}

		FindCloseFile( handle );
	}
	else
	{
		ASSERT( 0, "FindFirstFile FAILED!\n" );
	}

	return ret_code;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::CopyFile( const CString & src_file, const CString & dst_file, float progress_inc )
{
	if ( FileExists( dst_file ) == true )
	{
		if ( DeleteFile( dst_file ) == false )
		{
			return false;
		}
	}

	CFile * const	p_src_file( Open( src_file, "rb" ) );
	CFile * const	p_dst_file( Open( dst_file, "wb" ) );

	if ( p_src_file != NULL && p_dst_file != NULL )
	{
		bool	success( true );
		s32		src_length( p_src_file->GetLength() );

		if ( src_length > 0 )
		{
			u8 *		p_buffer( new u8[ src_length ] );
			const float	src_frac( ( src_length / COPY_SIZE_IN_BYTES ) + 1 );
			const float	copy_frac_inc( src_frac == 0.f ? 0.f : ( progress_inc / src_frac ) );

			while ( src_length > COPY_SIZE_IN_BYTES && success == true )
			{
				if ( p_src_file->Read( p_buffer, COPY_SIZE_IN_BYTES ) == false )
				{
					success = false;
					ASSERT( 0, "CFileSystem::CopyFile() - FAILED!\n" );
				}

				if ( p_dst_file->Write( p_buffer, COPY_SIZE_IN_BYTES ) == false )
				{
					success = false;
					ASSERT( 0, "CFileSystem::CopyFile() - FAILED!\n" );
				}

				src_length -= COPY_SIZE_IN_BYTES;

				CHUD::SetProgressBar( ( CHUD::GetProgressBar() + copy_frac_inc ) );
			}

			if ( success == true )
			{
				if ( p_src_file->Read( p_buffer, src_length ) == false )
				{
					success = false;
					ASSERT( 0, "CFileSystem::CopyFile() - FAILED!\n" );
				}

				if ( p_dst_file->Write( p_buffer, src_length ) == false )
				{
					success = false;
					ASSERT( 0, "CFileSystem::CopyFile() - FAILED!\n" );
				}
			}

			SAFE_RDELETE( p_buffer );
		}

		Close( p_dst_file );
		Close( p_src_file );

		return success;
	}

	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::CopyDirectory( const CString & src_dir, const CString & dst_dir, float progress_inc )
{
	//
	//	Gather the files in this directory
	//
	CFileList	dir_files;

	if ( GetDirectoryFiles( src_dir, dir_files ) == false )
	{
		ASSERT( 0, "GetDirectoryFiles FAILED!\n" );

		return false;
	}

	//
	//	Strip off the src path from the filenames
	//
	const u32	src_path_len( src_dir.Length() );

	for ( CFileList::iterator it = dir_files.begin(); it != dir_files.end(); ++it )
	{
		sDirEntry &	dir_entry( *it );

		dir_entry.m_szFileName = &dir_entry.m_szFileName[ src_path_len ];
	}

	//
	//	Make sure the destination folder exists
	//
	if ( MakeDirectory( dst_dir ) == false )
	{
		return false;
	}

	//
	//	Make sure all the destination directorys exist
	//
	for ( CFileList::iterator it = dir_files.begin(); it != dir_files.end(); ++it )
	{
		const sDirEntry &	dir_entry( *it );

		if ( dir_entry.IsDirectory() == true )
		{
			const CString	directory( dst_dir + dir_entry.m_szFileName );

			if ( MakeDirectory( directory ) == false )
			{
				return false;
			}
		}
	}

	//
	//	Copy all the files
	//
	for ( CFileList::iterator it = dir_files.begin(); it != dir_files.end(); ++it )
	{
		const sDirEntry &	dir_entry( *it );

		if ( dir_entry.IsFile() == true )
		{
			const CString	src_file( src_dir + dir_entry.m_szFileName );
			const CString	dst_file( dst_dir + dir_entry.m_szFileName );

			if ( CopyFile( src_file, dst_file, progress_inc ) == false )
			{
				return false;
			}
		}
	}

	return true;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::DeleteFile( const CString & filename )
{
	if ( MakeWritable( filename ) == true )
	{
		return ( sceIoRemove( filename ) >= 0 );
	}

	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::DeleteDirectory( const CString & directory )
{
	//
	//	Get a list of all the files and folders to delete
	//
	CFileList	dir_files;

	if ( GetDirectoryFiles( directory, dir_files ) == false )
	{
		ASSERT( 0, "GetDirectoryFiles FAILED!\n" );

		return false;
	}

	//
	//	Delete all the files first
	//
	bool	no_files( false );

	while ( no_files == false && dir_files.empty() == false )
	{
		no_files = true;

		for ( CFileList::iterator it = dir_files.begin(); it != dir_files.end(); ++it )
		{
			const sDirEntry &	dir_entry( *it );

			if ( dir_entry.IsFile() == true )
			{
				if ( DeleteFile( dir_entry.m_szFileName ) == false )
				{
					return false;
				}

				no_files = false;

				dir_files.erase( it );

				break;
			}
		}
	}

	//
	//	Delete all the directorys last
	//
	bool	no_directory( false );

	while ( no_directory == false && dir_files.empty() == false )
	{
		no_directory = true;

		for ( CFileList::iterator it = dir_files.begin(); it != dir_files.end(); ++it )
		{
			const sDirEntry &	dir_entry( *it );

			if ( dir_entry.IsDirectory() == true )
			{
				no_directory = false;

				if ( sceIoRmdir( dir_entry.m_szFileName ) == 0 )
				{
					dir_files.erase( it );

					break;
				}
			}
		}
	}

	//
	//	Finally remove the parent directory
	//
	if ( sceIoRmdir( directory ) != 0 )
	{
		return false;
	}

	return true;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::MakeDirectory( const CString & directory )
{
	if ( DirectoryExists( directory ) == false )
	{
		return ( sceIoMkdir( directory, 0 ) == 0 );
	}

	return true;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::IsUMDInserted()
{
	const s32	ret_code( sceUmdCheckMedium( 0 ) );

	return ( ret_code > 0 );
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::ActivateUMD()
{
	if ( sceUmdWaitDriveStat( 2 ) < 0 )
	{
		return false;
	}

	if ( sceUmdActivate( 1, "disc0:" ) < 0 )
	{
		return false;
	}

	if ( sceUmdWaitDriveStat( UMD_WAITFORINIT ) < 0 )
	{
		return false;
	}

	return true;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::Rename( const CString & old_name, const CString & new_name )
{
	if ( MakeWritable( old_name ) == true )
	{
		return ( sceIoRename( old_name, new_name ) >= 0 );
	}

	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileSystem::SplitPath( const CString & path, CString * p_drive_out, CString * p_dir_out, CString * p_fname_out, CString * p_ext_out )
{
	CString	drive, dir, fname, ext;
	CString	work_path( path );
	char *	p_work_path( const_cast< char * >( work_path.GetPtr() ) );

	//
	//	Find the extension
	//
	char *	p_ext( const_cast< char * >( work_path.Find( "." ) ) );

	if ( p_ext != NULL )
	{
		ext = ( p_ext + 1 );

		*p_ext = '\0';

		//
		//	Now find the fname by stepping back from the end of the path til we find a slash
		//
		while ( p_ext != p_work_path && *p_ext != '\\' && *p_ext != '/' )
		{
			--p_ext;
		}

		if ( p_ext != p_work_path )
		{
			p_ext += 1;

			fname = p_ext;

			*p_ext = '\0';
		}
	}

	//
	//	Look for a ':' to denote the end of a drive letter
	//
	char *	p_dir( const_cast< char * >( work_path.Find( ":" ) ) );

	if ( p_dir != NULL )
	{
		p_dir += 1;

		dir = p_dir;

		*p_dir = '\0';

		drive = work_path;
	}
	else
	{
		dir = work_path;
	}

	//
	//	Store the results
	//
	if ( p_drive_out != NULL )
	{
		*p_drive_out = drive;
	}

	if ( p_dir_out != NULL )
	{
		*p_dir_out = dir;
	}

	if ( p_fname_out != NULL )
	{
		*p_fname_out = fname;
	}

	if ( p_ext_out != NULL )
	{
		*p_ext_out = ext;
	}
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::MakeReadOnly( const CString & filename )
{
	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::MakeWritable( const CString & filename )
{
	SceIoStat	stats;

	if ( sceIoGetstat( filename, &stats ) >= 0 )
	{
		const u32	bits( 1 );

		stats.st_mode &= FIO_SO_IFMT;
		stats.st_mode |= FIO_SO_IROTH | FIO_SO_IWOTH;

		if ( sceIoChstat( filename, &stats, bits ) >= 0 )
		{
			return true;
		}
		else
		{
			ASSERT( 0, "sceIoChstat FAILED!" );
		}
	}
	else
	{
		ASSERT( 0, "sceIoGetstat FAILED!" );
	}

	return false;
}

//**********************************************************************************
//	
//**********************************************************************************
CString	CFileSystem::MakeFullPath( const CString & filename )
{
	if ( filename.Find( s_szRootDirectory ) != NULL )
	{
		return filename;
	}
	else
	{
		return ( s_szRootDirectory + filename );
	}
}

//**********************************************************************************
//	
//**********************************************************************************
CString	CFileSystem::GetFileExtension( const CString & filename )
{
	CString	ext;

	SplitPath( filename, NULL, NULL, NULL, &ext );

	return ext;
}

//**********************************************************************************
//	
//**********************************************************************************
bool	CFileSystem::HideCorruptFiles()
{
	return CVAR_HIDE_CORRUPT_FILES.Get();
}

//**********************************************************************************
//	
//**********************************************************************************
void	CFileSystem::SetHideCorruptFiles( bool hide )
{
	CVAR_HIDE_CORRUPT_FILES.Set( hide );
}

//**********************************************************************************
//	
//**********************************************************************************
CString	CFileSystem::GetSizeString( u32 file_size )
{
	CString	text;

	if ( file_size >= ( 1024 * 1024 ) )
	{
		text.Printf( "%.3fkb", static_cast< float >( file_size ) / ( 1024 * 1024 ) );
	}
	else
	{
		text.Printf( "%.0fkb", static_cast< float >( file_size ) / ( 1024 ) );
	}

	return text;
}

//*******************************  END OF FILE  ************************************
