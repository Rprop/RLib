/********************************************************************
	Created:	2014/06/30  20:34
	Filename: 	RLib_AppBase_Part.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_MemoryPool.h"
using namespace System::IO;

//-------------------------------------------------------------------------

extern MemoryPool RLib_GlobalBasePool;

//-------------------------------------------------------------------------

RLIB_RESTRICT_RETURN LPVOID AppBase::Allocate(intptr_t bytes 
											  RLIB_INTERNAL_DEBUG_PARAM)
{
	return RLib_GlobalBasePool.AllocByte(bytes
										 RLIB_INTERNAL_DEBUG_PARAM_VALUE);
}

//-------------------------------------------------------------------------

RLIB_RESTRICT_RETURN LPVOID AppBase::Reallocate(LPVOID p, intptr_t bytes)
{
	return RLib_GlobalBasePool.ReAlloc(p, bytes);
}
//-------------------------------------------------------------------------

void AppBase::Collect(LPVOID p)
{
	RLib_GlobalBasePool.Collect(p);
}

//-------------------------------------------------------------------------

MemoryPool *AppBase::GetUsingPool()
{
	return &RLib_GlobalBasePool;
}
