/********************************************************************
	Created:	2015/09/27  10:57
	Filename: 	RLib_IAllocator.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_IAllocator.h"
#include "RLib_MemoryPool.h"

using namespace System::IO;

//-------------------------------------------------------------------------

void *IAllocator::allocateMemory(intptr_t cb RLIB_INTERNAL_DEBUG_PARAM)
{
	return RLIB_GlobalAllocDebug(cb, RLIB_INTERNAL_DEBUG_PARAM_VALUE);
}

//-------------------------------------------------------------------------

void IAllocator::freeMemory(void *address) 
{
	RLIB_GlobalCollect(address);
}

//-------------------------------------------------------------------------

void *IAllocator::reallocateMemory(void *address, intptr_t cb)
{
	return RLIB_GlobalReAlloc(address, cb);
}

//-------------------------------------------------------------------------

IAllocator *IAllocator::getSharedInstance()
{
	return reinterpret_cast<IAllocator *>(nullptr);
}