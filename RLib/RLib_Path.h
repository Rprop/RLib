/********************************************************************
	Created:	2014/07/01  14:27
	Filename: 	RLib_Path.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_PATH
#define _USE_PATH
#include "RLib_String.h"

#ifndef _MAX_DRIVE
# define RLIB_MAX_DRIVE  3   // max. length of drive component
# define RLIB_MAX_DIR    256 // max. length of path component
# define RLIB_MAX_FNAME  256 // max. length of file name component
# define RLIB_MAX_EXT    256 // max. length of extension component
#else
# define RLIB_MAX_DRIVE  _MAX_DRIVE
# define RLIB_MAX_DIR    _MAX_DIR
# define RLIB_MAX_FNAME  _MAX_FNAME
# define RLIB_MAX_EXT    _MAX_EXT 
#endif // !_MAX_PATH

//-------------------------------------------------------------------------

namespace System
{
	namespace IO
	{
		/// <summary>
		/// Represents filesystem path structure
		/// </summary>
		struct RLIB_API PathInfo
		{
		public:
			TCHAR Drive[RLIB_MAX_DRIVE]; // C: or Empty in UNC path
			TCHAR Dir[RLIB_MAX_DIR];     // \Windows\System32\  or \\server\dir\ in UNC path
			TCHAR Fname[RLIB_MAX_FNAME]; // RLib
			TCHAR Ext[RLIB_MAX_EXT];     // .dll
		};
		/// <summary>
		/// Represents filesystem path
		/// </summary>
		struct RLIB_API Path
		{
		protected:
			PathInfo m_PathInfo;
			String   m_FullPath;

		public:
			Path(const String &);
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// Gets the absolute NT path
			/// </summary>
			operator const String &() const {
				return this->m_FullPath;
			}
			/// <summary>
			/// Gets the path info
			/// </summary>
			operator const PathInfo &() const {
				return this->m_PathInfo;
			}
			/// <summary>
			/// Gets the absolute MS-DOS path
			/// </summary>
			String GetDosPath();
			/// <summary>
			/// Gets the absolute MS-DOS path
			/// </summary>
			RLIB_PROPERTY_GET(String DosPath, GetDosPath);
			/// <summary>
			/// Gets the directory, eg. C:\Windows\System32\ 
			/// </summary>
			String GetDirectory();
			/// <summary>
			/// Gets the directory, eg. C:\Windows\System32\ 
			/// </summary>
			RLIB_PROPERTY_GET(String Directory, GetDirectory);
			/// <summary>
			/// Gets the file name, eg. RLib.dll
			/// </summary>
			String GetFileName();
			/// <summary>
			/// Gets the file name, eg. RLib.dll
			/// </summary>
			RLIB_PROPERTY_GET(String FileName, GetFileName);
			/// <summary>
			/// Determines if this path is a root path
			/// </summary>
			bool IsRoot();

		public:
			/// <summary>
			/// Converts forward slash to backslash in path
			/// </summary>
			static String ToBackslash(const String &path);
			/// <summary>
			/// Converts forward slash to backslash in path
			/// </summary>
			static String ToForwardslash(const String &path);
			/// <summary>
			/// Converts path to absolute NT path
			/// </summary>
			static String ToNtPath(const String &path);
			/// <summary>
			/// Converts path to absolute MS-DOS path
			/// </summary>
			static String ToDosPath(const String &path, bool nt_processed = false);
			/// <summary>
			/// Append a backslash _T('\') to a path if one doesn't exist
			/// </summary>
			static String AddBackslash(const String &path);
			/// <summary>
			/// Remove a trailing backslash from a path
			/// </summary>
			static String RemoveBackslash(const String &path);
			/// <summary>
			/// Surround a path containing spaces in quotes.
			/// The path is not changed if it has no spaces
			/// </summary>
			static String QuoteSpaces(const String &path);
			/// <summary>
			/// Determines if a path is a root path.
			/// If path is relative path, this method returns false immediately
			/// </summary>
			static bool IsRoot(const String &path);
			/// <summary>
			/// Determines if a path is a relative path
			/// </summary>
			static bool IsRelative(const String &path);
			/// <summary>
			/// Creates a relative path from one path to another.
			/// If the paths do not share a common prefix, this method returns absolute path of path_to
			/// </summary>
			static String RelativePathTo(const String &path_from, const String &path_to);
		};
	};
};
#endif