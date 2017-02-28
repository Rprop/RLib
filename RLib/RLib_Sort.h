/********************************************************************
	Created:	2016/08/14  4:29
	Filename: 	RLib_Sort.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib.h"
#include "RLib_Standard.h"

namespace System
{
	namespace Generic
	{
		/// <summary>
		/// Provides several sorting algorithms
		/// </summary>
		namespace Sorting
		{
			extern "C" RLIB_API void quick_sort(void  *_Base,
												size_t _NumOfElements,
												size_t _SizeOfElements,
												int(__cdecl *_PtFuncCompare)(void const *, void const *));

			template<class T> inline void sort(T _First, T _Last) {	
				_STD sort(_First, _Last);
			}
		}
	}
}


