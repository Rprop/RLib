/********************************************************************
	Created:	2016/08/14  4:52
	Filename: 	RLib_Sort.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Sort.h"
#include <corecrt_search.h>
using namespace System::Generic;

//-------------------------------------------------------------------------

extern "C" RLIB_API void Sorting::quick_sort(void  *_Base,
										size_t _NumOfElements,
										size_t _SizeOfElements,
										int(__cdecl *_PtFuncCompare)(void const *, void const *))
{
	return ::qsort(_Base, _NumOfElements, _SizeOfElements, _PtFuncCompare);
}
