/********************************************************************
	Created:	2014/12/01  17:24
	Filename: 	RLib_Directory.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_DIRECTORY
#define _USE_DIRECTORY
#include "RLib_File.h"

//////////////////////////////////////////////////////////////////////////
namespace System
{
	namespace IO
	{
		/// <summary>
		/// Exposes static methods for creating, moving, 
		/// and enumerating through directories and subdirectories
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE Directory
		{
		public:
			/// <summary>
			/// Creates new directory in the specified path
			/// </summary>
			static bool Create(const String &path, ACCESS_MASK mark);
			/// <summary>
			/// Creates new directory in the specified path with access rights
			/// </summary>
			static bool Create(const String &path,
							   PSECURITY_DESCRIPTOR lpSecurityDescriptor = nullptr);
			/// <summary>
			/// Trys to create the specified directory if not exist, with access rights
			/// </summary>
			static bool CreateIf(const String &path, ACCESS_MASK mark);
			/// <summary>
			/// Trys to create the specified directory if not exist
			/// </summary>
			static bool CreateIf(const String &path,
								 PSECURITY_DESCRIPTOR lpSecurityDescriptor = nullptr);
			/// <summary>
			/// Determines if a directory is already exist
			/// </summary>
			static bool Exist(const String &path);
			/// <summary>
			/// Determines if a path is directory
			/// </summary>
			static bool IsDirectory(const String &path);
		};
	};
}
#endif //_USE_DIRECTORY