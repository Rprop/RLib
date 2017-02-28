/********************************************************************
	Created:	2014/07/02  14:44
	Filename: 	RLib_UnDNameEx.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_AppBase.h"
/************************************************************************
 * 
 */
typedef void *(*rlib_alloc_func)  (size_t);
typedef void  (*rlib_collect_func)(void *);
/*********************************************************************
 *		__unDNameEx (MSVCRT.@)
 *
 * Demangle a C++ identifier.
 *
 * PARAMS
 *  buffer   [O] If not NULL, the place to put the demangled string
 *  mangled  [I] Mangled name of the function
 *  buflen   [I] Length of buffer
 *  memget   [I] Function to allocate memory with
 *  memfree  [I] Function to free memory with
 *  unknown  [?] Unknown, possibly a call back
 *  flags    [I] Flags determining demangled format
 *
 * RETURNS
 *  Success: A string pointing to the unmangled name, allocated with memget.
 *  Failure: NULL.
 */
extern "C" char * CDECL __unDNameEx(char *buffer, const char *mangled,
									int buflen, rlib_alloc_func memget, rlib_collect_func memfree,
									void *unknown = 0, unsigned short int flags = 0);

//-------------------------------------------------------------------------

#ifdef _DEBUG

static void *alloc(size_t bytes)
{
	return RLIB_GlobalAllocAny(void *, bytes);
}

//-------------------------------------------------------------------------

RLIB_API char *_unDNameEx(char* buffer, const char* mangled, int buflen)
{
#ifdef _DEBUG
	return __unDNameEx(buffer, mangled, buflen, alloc, System::AppBase::Collect);
#else
	return __unDNameEx(buffer, mangled, buflen, System::AppBase::Allocate,
					   System::AppBase::Collect);
#endif // _DEBUG
};

//-------------------------------------------------------------------------

#endif // _DEBUG