/***********************************************************************************

  Module :	CFileSystem.h

  Description :	

  Last Modified $Date: $

  $Revision: $

  Copyright (C) 01 August 2005 T Swann

***********************************************************************************/

#ifndef CFILESYSTEM_H_
#define CFILESYSTEM_H_

//**********************************************************************************
//   Include Files
//**********************************************************************************
#include "CTypes.h"
#include "CString.h"

//**********************************************************************************
//   Macros
//**********************************************************************************

//**********************************************************************************
//   Types
//**********************************************************************************
enum eAttributeFlags
{
	AF_READ_ONLY	= ( 1 << 0 ),
	AF_HIDDEN		= ( 1 << 1 ),
	AF_UNKNOWN1		= ( 1 << 2 ),
	AF_UNKNOWN2		= ( 1 << 3 ),
	AF_DIRECTORY	= ( 1 << 4 ),
	AF_ARCHIVE		= ( 1 << 5 ),
	AF_BACK_BUTTON	= ( 1 << 30 ),
	AF_DRIVE		= ( 1 << 31 ),
};

struct sDirEntry
{
	CString		m_szFileName;
	SceIoStat	m_Stats;

	bool		IsFile() const			{ return ( IsDrive() == false && IsBackButton() == false && IsDirectory() == false ); }
	bool		IsDrive() const			{ return ( m_Stats.st_mode & AF_DRIVE ); }
	bool		IsBackButton() const	{ return ( m_Stats.st_mode & AF_BACK_BUTTON ); }
	bool		IsDirectory() const		{ return ( ( m_Stats.st_mode & FIO_S_IFDIR ) & FIO_S_IFMT ); }
};

typedef std::list< sDirEntry >	CFileList;
typedef SceUID					FIND_FILE_HANDLE;

//**********************************************************************************
//   Constants
//**********************************************************************************

//**********************************************************************************
//   Class definitions
//**********************************************************************************

//*************************************************************************************
//	
//*************************************************************************************
class CFile
{
	public:

		friend class CFileSystem;

	public:

		bool						Read( void * p_address, u32 length, u32 * p_nbytes_read = NULL ) const;
		bool						Write( const void * p_address, u32 length, u32 * p_nbytes_written = NULL ) const;

		bool						IsEOF() const;

		u32							GetLength() const;

		u32							Tell() const;
		bool						Seek( const u32 offset, const u32 origin ) const;

		char						FGetC() const;

		CString						GetExtension() const;

		bool						IsReadOnly() const;
		bool						IsWritable() const;

		operator FILE * () const;

	protected:

		CFile( const CString & filename, FILE * const p_handle );
		~CFile();

	private:

		CString						m_szFilename;
		FILE *						m_pHandle;
		u32							m_Length;
};

//*************************************************************************************
//	
//*************************************************************************************
class CFileSystem
{
	public:

		static bool					SetRoot( const CString & directory );

		static CFile *				Open( const CString & filename, const char * const p_open_flags );
		static void					Close( CFile * const p_file );

		static bool					FileExists( const CString & filename );
		static bool					DirectoryExists( const CString & directory );

		static bool					GetDirectoryFiles( const CString & directory, CFileList & dir_files );

		static bool					FindFirstFile( const CString & path, FIND_FILE_HANDLE & handle );
		static bool					FindNextFile( sDirEntry & dir_entry, FIND_FILE_HANDLE handle );
		static bool					FindCloseFile( FIND_FILE_HANDLE handle );

		static bool					CopyFile( const CString & src_file, const CString & dst_file, float progress_inc = 0.f );
		static bool					CopyDirectory( const CString & src_dir, const CString & dst_dir, float progress_inc = 0.f );

		static bool					DeleteFile( const CString & filename );
		static bool					DeleteDirectory( const CString & directory );

		static bool					MakeDirectory( const CString & directory );

		static bool					IsUMDInserted();
		static bool					ActivateUMD();

		static bool					Rename( const CString & old_name, const CString & new_name );

		static void					SplitPath( const CString & path, CString * p_drive, CString * p_dir, CString * p_fname, CString * p_ext );

		static bool					MakeReadOnly( const CString & filename );
		static bool					MakeWritable( const CString & filename );

		static CString				MakeFullPath( const CString & filename );

		static CString				GetSizeString( u32 size );
		static CString				GetFileExtension( const CString & filename );

		static bool					HideCorruptFiles();
		static void					SetHideCorruptFiles( bool hide );

	private:

		static SceIoDirent			s_DirEntry;
		static CString				s_szRootDirectory;
};

//**********************************************************************************
//   Externs
//**********************************************************************************

//**********************************************************************************
//   Prototypes
//**********************************************************************************

#endif /* CFILESYSTEM_H_ */
